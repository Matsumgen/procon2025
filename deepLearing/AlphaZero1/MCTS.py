import math
import random
import json

import tensorflow as tf
from lib import Field

try:
  import cupy as np
  np.array([1, 2, 3])
except Exception as e:
  import numpy as np
  # print(f"CuPyが使えないためNumPyを使います: {e}")

class MCTS:
  MAX_DEPTH = 512
  def __init__(self, network, alpha, c_puct=1.0, epsilon=0.25):
    self.network = network
    self.alpha = alpha
    self.c_puct = c_puct
    self.eps = epsilon

    #: prior probability
    self.P = dict()

    #: visit count
    self.N = {}

    #: Wは累計価値
    #: Q(s, a) = W(s, a) / N(s, a)
    self.W = {}

    #: cache next states to save computation
    self.next_states = {}

  def search(self, root_state, num_simulations):
    root_key = root_state.key()

    if root_key not in self.P:
      _ = self._expand(root_state)
    
    #: root状態にだけは事前確立にディリクレノイズをのせて探索を促進する
    dirichlet_noise = np.random.dirichlet(alpha=[self.alpha]*Field.ACTION_SPACE)
    for a, noise in zip(range(Field.ACTION_SPACE), dirichlet_noise):
      self.P[root_key][a] = (1 - self.eps) * self.P[root_key][a] + self.eps * noise

    # simuration 実行
    for _ in range(num_simulations):
      U = [self.c_puct * self.P[root_key][a] * math.sqrt(sum(self.N[root_key])) / (1 + self.N[root_key][a]) for a in range(Field.ACTION_SPACE)]
      Q = [w / n if n != 0 else 0 for w, n in zip(self.W[root_key], self.N[root_key])]

      #: PUCTスコアの算出
      scores = [u + q for u, q in zip(U, Q)]
      scores = np.array([score if action in range(Field.ACTION_SPACE) else -np.inf for action, score in enumerate(scores)])

      #: スコアのもっとも高いactionを選択
      action = random.choice(np.where(scores == scores.max())[0])
      next_state = self.next_states[root_key][action]

      #: 選択した行動を評価
      v = self._evaluate(next_state)

      self.W[root_key][action] += v
      self.N[root_key][action] += 1

    #: mcts_policyは全試行回数に占める各アクションの試行回数の割合
    mcts_policy = [n / sum(self.N[root_key]) for n in self.N[root_key]]
    return mcts_policy


  def _expand(self, state:Field):
    with tf.device("/cpu:0"):
      nn_policy, nn_value = self.network.predict(state.encode())

    nn_policy, nn_value = nn_policy.numpy().tolist()[0], nn_value.numpy()[0][0]

    state_key = state.key()
    self.P[state_key] = nn_policy
    self.N[state_key] = [0] * Field.ACTION_SPACE
    self.W[state_key] = [0] * Field.ACTION_SPACE


    self.next_states[state_key] = [ state.step(action) for action in range(Field.ACTION_SPACE) ]
    return nn_value

  def _evaluate(self, state, depth=0):
    # print(f"[Depth {depth}] Evaluating state: {state.str_data()}")
    end = state.isEnd()
    state_key = state.key()

    if depth > MCTS.MAX_DEPTH:
      # MAX_DEPTHだけ手が進んだら終了
      return 0
    elif end:
      return state.evaluation()
    elif state_key in self.P:
      #: PUCTによってさらに子盤面を選択する
      U = [self.c_puct * self.P[state_key][a] * math.sqrt(sum(self.N[state_key])) / (1 + self.N[state_key][a]) for a in range(Field.ACTION_SPACE)]
      Q = [q / n if n != 0 else q for q, n in zip(self.W[state_key], self.N[state_key])]

      v_actions = [ i for i, b in enumerate(range(Field.ACTION_SPACE)) if b ]

      scores = [u + q for u, q in zip(U, Q)]
      scores = np.array([score if action in v_actions else -np.inf for action, score in enumerate(scores)])

      action = random.choice(np.where(scores == scores.max())[0])
      next_state = self.next_states[state_key][action]
      v = -self._evaluate(next_state, depth=depth+1)
      self.W[state_key][action] += v
      self.N[state_key][action] += 1
      return v
    else:
      nn_value = self._expand(state)
      return nn_value


