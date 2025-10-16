import hashlib
import random


def _fast_isEnd_numpy(field):
  max_val = 288
  pos_y = np.zeros((max_val, 2), dtype=np.int32)
  pos_x = np.zeros((max_val, 2), dtype=np.int32)
  counts = np.zeros(max_val, dtype=np.int32)

  h, w = field.shape
  for y in range(h):
    for x in range(w):
      k = field[y, x]
      c = counts[k]
      if c >= 2:
        return False  # 2つ以上はNG
      pos_y[k, c] = y
      pos_x[k, c] = x
      counts[k] = c + 1

  for k in range(max_val):
    if counts[k] == 0:
      continue  # 値が存在しないならスキップ
    if counts[k] != 2:
      return False  # 2個じゃなければNG
    y1, y2 = pos_y[k, 0], pos_y[k, 1]
    x1, x2 = pos_x[k, 0], pos_x[k, 1]
    if not ((abs(y1 - y2) == 1 and x1 == x2) or (abs(x1 - x2) == 1 and y1 == y2)):
      return False
  return True


def _fast_isEnd_cupy(field):
  # 0以上の位置座標と値を取得
  yx = np.argwhere(field >= 0)
  values = field[yx[:, 0], yx[:, 1]]

  # 2個ずつセットで存在しないなら即False
  if values.size % 2 != 0:
    return False

  # 値でソート
  sort_idx = np.argsort(values)
  values = values[sort_idx]
  yx = yx[sort_idx]

  # 2つ1組に分割
  v1 = values[0::2]
  v2 = values[1::2]

  # 値がペアで一致しているかのチェック
  if not np.all(v1 == v2):
    return False

  # 座標をペアで取得
  y1, x1 = yx[0::2, 0], yx[0::2, 1]
  y2, x2 = yx[1::2, 0], yx[1::2, 1]

  # 隣接判定（縦か横が1差で他は同じ）
  cond = ((np.abs(y1 - y2) == 1) & (x1 == x2)) | ((np.abs(x1 - x2) == 1) & (y1 == y2))
  if not np.all(cond):
    return False

  return True

try:
  import cupy as np
  np.array([1, 2, 3])
  fast_isEnd = _fast_isEnd_cupy
except Exception as e:
  import numpy as np
  from numba import njit
  fast_isEnd = njit(_fast_isEnd_numpy)
  # print(f"CuPyが使えないためNumPyを使います: {e}")


_actions = list()
for x in range(23):
  for y in range(23):
    for n in range(2, 25):
      if x + n <= 24 and y + n <= 24:
        _actions.append((x, y, n))


class Field:
  ACTION_SPACE = len(_actions)
  ACTIONS = _actions
  def __init__(self, field=None, count=0):
    if field is None:
      self.field = Field.randomfield()
    else:
      self.field = field

    self.count = count
    self.reallocation()

  @staticmethod
  def randomfield():
    field = np.repeat(np.arange(288), 2)
    field = np.random.permutation(field)
    return field.reshape((24, 24))

  def reallocation(self):
    flat = self.field.ravel()
    unique, inverse = np.unique(flat, return_inverse=True)
    self.field = inverse.reshape(self.field.shape)


  def encode(self):
    ret = np.zeros((24, 24, 288), dtype=np.float32)
    y_idx, x_idx = np.indices((24, 24))
    ret[y_idx, x_idx, self.field] = 1
    return ret

  def step(self, action):
    act = Field.ACTIONS[action]
    return self.rotate(*act)

  def rotate(self, x, y, n):
    result = self.field.copy()
    sub = self.field[y:y+n, x:x+n]
    result[y:y+n, x:x+n] = np.fliplr(sub.T)
    return Field(result, self.count + 1)
    

  def isEnd(self):
    return fast_isEnd(self.field)

  def key(self):
    return hashlib.sha256(self.field.tobytes()).hexdigest()

  def evaluation(self):
    if self.count == 0:
      return 1
    else:
      return 1/self.count

  def __str__(self):
    s = f"count: {self.count}\n"
    for y in self.field:
      for i in y:
        s += f"{i:>4}"
      s += "\n"
    return s

  def __repr__(self):
    return str(self)
