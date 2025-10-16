BUFFER_SIZE = 1000000
SELFPLAY_COUNT = 1000
SAVE_COUNT = 1000
SELFPLAY_MAX = 1000
# BUFFER_SIZE = 10000
# SELFPLAY_COUNT = 1
# SAVE_COUNT = 1
# SELFPLAY_MAX = 10
OBJECT_STORE_MEMORY = 512 * 1024 * 1024
RANDOM_CHOICE = 50


from Network import AlphaZeroResNet
from MCTS import MCTS
from lib import Field

from tqdm import tqdm
import tensorflow as tf
import collections
import ray
import os
import json

try:
  import cupy as np
  np.array([1, 2, 3])
except Exception as e:
  import numpy as np
  print(f"CuPyが使えないためNumPyを使います: {e}")

gpus = tf.config.experimental.list_physical_devices('GPU')
for gpu in gpus:
    tf.config.experimental.set_memory_growth(gpu, True)

class ReplayBuffer:
  def __init__(self, buffer_size, state_shape, action_space):
    self.buffer_size = buffer_size
    self.index = 0
    self.full = False

    self.states = np.zeros((buffer_size, *state_shape), dtype=np.float32)
    self.policies = np.zeros((buffer_size, action_space), dtype=np.float32)
    self.rewards = np.zeros((buffer_size, 1), dtype=np.float32)

  def __len__(self):
    return self.buffer_size if self.full else self.index

  def add_record(self, states_np, policies_np, rewards_np):
    N = len(states_np)
    for i in range(N):
      self.states[self.index] = states_np[i]
      self.policies[self.index] = policies_np[i]
      self.rewards[self.index] = rewards_np[i]

      self.index = (self.index + 1) % self.buffer_size
      if self.index == 0:
        self.full = True

  def get_minibatch(self, batch_size):
    max_idx = self.buffer_size if self.full else self.index
    indices = np.random.choice(max_idx, size=batch_size, replace=False)

    return ( self.states[indices], self.policies[indices], self.rewards[indices] )

@ray.remote(num_cpus=1, num_gpus=0.062)
def selfplay(weights, num_mcts_simulations, dirichlet_alpha=0.35):
  
  states_np = []
  policies_np = []
  turns = []

  state = Field()
  #: networkのbuildし、最新の重みに同期
  network = AlphaZeroResNet(action_space=Field.ACTION_SPACE)
  network.predict(state.encode())
  network.set_weights(weights)

  mcts = MCTS(network=network, alpha=dirichlet_alpha)

  current_player = 1
  done = False
  i = 0

  while not state.isEnd():
    #: mcts_policyはnum_simulations=50回のMCTSにおける各アクションの試行回数の割合
    mcts_policy = mcts.search( root_state=state, num_simulations=num_mcts_simulations)

    if i <= RANDOM_CHOICE:
      #: 最初の50手（originalは30回）はMCTSの試行回数に比例した確率でアクションを選択
      action = np.random.choice( range(Field.ACTION_SPACE), p=mcts_policy)
    else:
      #: MCTSの試行回数がもっとも大きいアクションを選択
      #: np.argmaxを使うと同値maxの場合に選択が偏る
      action = np.random.choice(np.where(np.array(mcts_policy) == max(mcts_policy))[0])

    states_np.append(state.encode())
    policies_np.append(mcts_policy)

    state = state.step(action)
    i += 1
    if i > SELFPLAY_MAX:
      break;

  if state.isEnd():
    rewards = state.evaluation()
    rewards_np = [rewards] * i
  else:
    rewards_np = [-1] * i

  return np.array(states_np), np.array(policies_np), np.array(rewards_np).reshape(-1, 1)


def main_loop(num_cpus, num_gpus, n_parallel_selfplay=20, num_mcts_simulations=50, loop_lim=10000, batch_size=64, learn_model_path=None):
  ray.init(num_cpus=num_cpus, num_gpus=num_gpus, object_store_memory=OBJECT_STORE_MEMORY, include_dashboard=True, _system_config={
        "object_spilling_config": json.dumps({
            "type": "filesystem",
            "params": {
                # "directory_path": "/home/tea/geister-python-ai/deepLeaning/tmp"
                "directory_path": "./tmp"
            }
        })
    })

  if learn_model_path is None:
    #: networkのbuild
    network = AlphaZeroResNet(action_space=Field.ACTION_SPACE)
    dummy_state = Field().encode()
    network.predict(dummy_state)

    #: テスト用のmodelをセーブ
    os.makedirs("test_model", exist_ok=True)
    network.save("test_model/test.keras")
  else:
    print("load model: ", learn_model_path)
    network = tf.keras.models.load_model(learn_model_path, custom_objects={'AlphaZeroResNet': AlphaZeroResNet})

  #: ray.putはデータサイズが大きいオブジェクトを多数のworkerに渡すときに使うとパフォーマンスが向上
  current_weights = ray.put(network.get_weights())
  
  #: オリジナルはAdamでなくSGD
  optimizer = tf.keras.optimizers.Adam(learning_rate=0.0005)
  
  replay = ReplayBuffer(buffer_size=BUFFER_SIZE, state_shape=Field().encode().shape, action_space=Field.ACTION_SPACE)

  #: 並列Selfplayの開始
  work_in_progresses = [ selfplay.remote(current_weights, num_mcts_simulations) for _ in range(n_parallel_selfplay)]

  n = 0
  while n <= loop_lim:

    #: selfplayを300回実行してデータ収集
    for _ in tqdm(range(SELFPLAY_COUNT)):
      #: selfplayが終わったプロセスを一つ取得
      finished, work_in_progresses = ray.wait(work_in_progresses, num_returns=1)
      result_ref = finished[0]
      states, policies, rewards = ray.get(result_ref)
      replay.add_record(states, policies, rewards)
      work_in_progresses.extend([ selfplay.remote(current_weights, num_mcts_simulations) ])
      del result_ref
      n += 1

    # 5epoch分のネットワーク更新
    num_iters = 5 * (len(replay) // batch_size)
    print("replay length:", len(replay))
    for i in range(num_iters):
      #: minibatchを取得
      states, mcts_policy, rewards = replay.get_minibatch(batch_size=batch_size)

      with tf.GradientTape() as tape:

        p_pred, v_pred = network(states, training=True)

        #: valuelossはMSE
        value_loss = tf.square(rewards - v_pred)

        #: policylossはcategorical cross entropy
        policy_loss = -mcts_policy * tf.math.log(p_pred + 0.0001)
        policy_loss = tf.reduce_sum(
            policy_loss, axis=1, keepdims=True)

        loss = tf.reduce_mean(value_loss + policy_loss)

      grads = tape.gradient(loss, network.trainable_variables)
      optimizer.apply_gradients(zip(grads, network.trainable_variables))

    current_weights = ray.put(network.get_weights())

    if n % SAVE_COUNT == 0:
      save_dir = f"models/saved_model_step_{n}.keras"
      os.makedirs("models", exist_ok=True)
      network.save(save_dir)
      print(f"[Saved] Model saved to {save_dir}")

if __name__ == "__main__":
  import argparse
  parser = argparse.ArgumentParser()
  parser.add_argument('-m', '--model')
  args = parser.parse_args()
  main_loop(num_cpus=1, num_gpus=1, learn_model_path=args.model)
  # main_loop(num_cpus=16, num_gpus=1, n_parallel_selfplay=16, loop_lim=100000, learn_model_path=args.model)


