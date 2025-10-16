import numpy as np
import time

def test1():
# データ生成（大きめのランダム配列）
  N = 24
  field = np.random.randint(0, 256, (N, N), dtype=np.uint8)
  x, y = 1, 0  # 左上座標
  n = 22       # 回転サイズ

# ---- 1. np.rot90を使う ----
  def rotate_np_rot90(field, x, y, n):
      result = field.copy()
      sub = field[y:y+n, x:x+n]
      result[y:y+n, x:x+n] = np.rot90(sub, k=-1)
      return result

# ---- 2. transpose + flip ----
  def rotate_np_transpose_flip(field, x, y, n):
      result = field.copy()
      sub = field[y:y+n, x:x+n]
      rotated = np.transpose(sub[::-1, :])  # 時計回り90度
      result[y:y+n, x:x+n] = rotated
      return result

# ---- 3. 手動ループで回転 ----
  def rotate_manual(field, x, y, n):
      result = field.copy()
      sub = field[y:y+n, x:x+n]
      rotated = np.zeros_like(sub)
      for i in range(n):
          for j in range(n):
              rotated[j, n - 1 - i] = sub[i, j]
      result[y:y+n, x:x+n] = rotated
      return result

# ---- 4. .T + fliplr で回転 ----
  def rotate_fliplr_transpose(field, x, y, n):
      result = field.copy()
      sub = field[y:y+n, x:x+n]
      result[y:y+n, x:x+n] = np.fliplr(sub.T)
      return result

# ---- 計測関数（1000回） ----
  def benchmark(func, name, iterations=1000):
      start = time.perf_counter()
      for _ in range(iterations):
          func(field, x, y, n)
      end = time.perf_counter()
      avg_ms = 1000 * (end - start) / iterations
      print(f"{name:<30}: {avg_ms:.6f} ms (avg over {iterations} runs)")
      return avg_ms

# ---- 実行 ----
  print("🔁 回転処理の平均実行速度（1000回繰り返し）:")
  benchmark(rotate_np_rot90,         "np.rot90")
  benchmark(rotate_fliplr_transpose, "fliplr + transpose")
  benchmark(rotate_np_transpose_flip,"transpose + flip")
  benchmark(rotate_manual,           "manual loop")

def test2():
  import argparse
  import tensorflow as tf
  import numpy as np
  from Network import AlphaZeroResNet
  from lib import Field
  parser = argparse.ArgumentParser()
  parser.add_argument("model")
  args = parser.parse_args()
  model = tf.keras.models.load_model(args.model, custom_objects={'AlphaZeroResNet': AlphaZeroResNet})
  field = Field()
  nn_policy, nn_value = model.predict(tf.expand_dims(field.encode(), axis=0))
  nn_policy = nn_policy[0]
  action = np.random.choice(np.where(np.array(nn_policy) == max(nn_policy))[0])
  print(field)
  print(Field.ACTIONS[action])




test2()
