import numpy as np
import math

# 数字の振り直し
def reallocation(field):
  unique = {}
  new_val = 0
  if isinstance(field, list):
    for i in range(len(field)):
      val = field[i]
      if val not in unique:
        unique[val] = new_val
        new_val += 1
      field[i] = unique[val]

  elif isinstance(field, np.ndarray):
    for y in range(field.shape[0]):
      for x in range(field.shape[1]):
        val = field[y, x]
        if val not in unique:
          unique[val] = new_val
          new_val += 1
        field[y, x] = unique[val]

# Fieldをバイト列にエンコード
def encodeField(f):
  if type(f) is np.ndarray:
    f = f.ravel().tolist()

  reallocation(f)
  fsize = int(math.isqrt(len(f)))


  # d = { k: 0 for k in range(len(f)//2) }
  d = [0] * (len(f) // 2)
  def getDIndex(upK:int):
    di = {}
    idx = 0
    for k in range(len(d)):
      if d[k] > 1:
        d[k] = -1
      elif d[k] >= 0:
        di[k] = idx
        idx += 1
    d[f[upK]] += 1
    return di, d[f[upK]] == 2
        
  ret = bytearray()
  d[0] += 1
  num_p = 1 # 出現しうる数字の最大値
  shift_count = 0
  b = 0
  for e in range(1, fsize*fsize - 1, 1):
    di, flag = getDIndex(e) # di[f[e]]: index
    if num_p != 0:
      sc = num_p.bit_length()
      b = (b << sc) | di[f[e]]
      shift_count += sc
      if shift_count >= 8:
        shift_count -= 8
        ret.append((b >> shift_count) & 0xFF)
        b &= (1 << shift_count) - 1

    num_p += -1 if flag else 1
    num_p = min(num_p, di[max(di.keys())])

  if shift_count != 0:
    ret.append(b << (8 - shift_count) & 0xFF)

  return bytes(ret)

def decodeField(bfi:bytes, fsize):
  fentsize = fsize * fsize
  bitbuf = int.from_bytes(bfi, 'big')
  bitlen = len(bfi) * 8
  bitpos = 0
  def get(n):
    nonlocal bitbuf, bitpos, bitlen
    if bitpos + n > bitlen:
      return 0
    shift = bitlen - (bitpos + n)
    val = (bitbuf >> shift) & ((1 << n) - 1)
    bitpos += n
    return val

  dsize = fentsize // 2
  d = [0] * dsize
  ret = np.full(fentsize, -1, dtype=int)
  ret[0] = 0
  d[0] = 1
  num_p = 1
  for i in range(1, fentsize, 1):
    val = get(num_p.bit_length())

    di = []
    for k in range(dsize):
      if d[k] <= 1:
        di.append(k)
    target = di[val]
    d[target] += 1
    ret[i] = target
    num_p += -1 if d[target] == 2 else 1
    num_p = min(num_p, len(di) - 1)
  return ret.reshape((fsize, fsize))

# 6*6までのfieldで対応
def encodeOperate(ope:list):
  # x: 0~4, y:0~4, n:2~5(0~3)
  # xxxyyynn
  x, y, n = ope
  if 0 <= x < 5 and 0 <= y < 5 and 2 <= n < 6:
    return bytes([ (x << 5) | (y << 2) | (n - 2) ])
  raise ValueError(f"encodeOperate: x={ope[0]}, y={ope[1]}, n={ope[2]}")

def decodeOperate(bop:bytes):
  b = bop[0]
  return [ b >> 5, (b >> 2) & 0b111, (b & 0b11) + 2 ]

def rotate(field:np.ndarray, x:int, y:int, n:int, rev:bool=False):
  # np.ndarryのfieldを90度回転
  # rev=Trueで左回転
  sub = field[y:y+n, x:x+n]
  if rev:
    field[y:y+n, x:x+n] = np.flipud(sub.T)
  else:
    field[y:y+n, x:x+n] = np.fliplr(sub.T)



  # 終了条件確認
def isEnd(field):
  pairs = {}
  for y, x in zip(*np.where(field >= 0)):  # 念のため負数除外
    k = field[y, x]
    if k not in pairs:
      pairs[k] = [(y, x)]
    else:
      pairs[k].append((y, x))

  for k, pos in pairs.items():
    if len(pos) != 2:
      return False
    (y1, x1), (y2, x2) = pos
    if not ((abs(y1 - y2) == 1 and x1 == x2) or (abs(x1 - x2) == 1 and y1 == y2)):
      return False
  return True

def isProblem(field):
  # 問題の形式通りか確認
  unique, counts = np.unique(field, return_counts=True)
  if not (len(unique) == field[0] * field[1] // 2 and all(counts == 2)):
    return False
  return True


# field4に縮小。できなければNone
def toSmallField(field):
  if field[0].shape[0] == 4:
    return None
  ret = []
  fsize = field.shape[0]
  fentlen = fisze * fsize // 2
  for y1, y2 in ((0, fsize - 2), (2, fsize)):
    for x1, x2 in ((0, fsize - 2), (2, fsize)):
      if isProblem(field[y1:y2, x1:x2]):
        f = field.copy()
        f[y1:y2, x1:x2] = -1
        for k in range(fentlen):
          pos = np.argwhere(field == k)
          if pos.size == 0:
            continue

          (y1, x1), (y2, x2) = pos
          if not ((abs(y1 - y2) == 1 and x1 == x2) or (abs(x1 - x2) == 1 and y1 == y2)):
            return None
        ret.append((field[y1:y2, x1:x2], x1, y1))
  return ret


# DBへ登録するkeyとvalueを返す
# 全体を回転させた盤面がDBに登録されているなら登録しない
# 全体を回転させた時、最もバイト数が少ないやつを登録
def putDB(txn, field, ope):
  fsize = field.shape[0]
  encf = encodeField(field)
  if txn.get(encf) is not None:
    return None, None

  best_enc = encf
  best_ope = ope
  min_len = len(encf)
  f = field.copy()
  for _ in range(3):
    rotate(f, 0, 0, fsize)
    encf = encodeField(f)
    if txn.get(encf) is not None:
      return None, None

    x, y, n = ope
    ope = [fsize - y - n, x, n]
    if len(encf) < min_len:
      best_enc = encf
      best_ope = ope
      min_len = len(encf)

  return best_enc, encodeOperate(best_ope)

# DBから取得
def getDB(txn, field):
  fsize = field.shape[0]
  encf = encodeField(field)
  enco = txn.get(encf)
  if enco is not None:
    return decodeOperate(enco)

  f = field.copy()
  for i in range(1, 4, 1):
    rotate(f, 0, 0, fsize)
    encf = encodeField(f)
    enco = txn.get(encf)
    if enco is not None:
      ope = decodeOperate(enco)
      for _ in range(i):
        x, y, n = ope
        ope = [y, fsize - x - n, n]
      return ope

# 問題を解く
# dbs: データベース。インデックスが大きいほど、小さいFieldのデータベース
# field6を解く -> dbs[0] = db6, dbs[1] = db4
def resolve(dbs:list, field):
  class Tree:
    def __init__(self, parent=None):
      self.data = []
      self.parent = parent
    def append(self, d):
      self.data.append(d)
    def get(self):
      if self.parent is None:
        return self.data
      else:
        return self.parent.get() + self.data
    def __len__(self):
      ret = len(self.data)
      if self.parent is not None:
        ret += len(self.parent)
      return ret

  fsize = field.shape[0]
  fields = [ ( field, 0, 0, Tree()) ] # (field, px, py, ret_index)
  ret = []
  for db in dbs:
    _fields = fields
    fields = []
    for f in _fields:
      with db.begin() as txn:
        while True:
          ope = getDB(txn, f[0])
          if ope is None:
            break
          f[3].append(( ope[0] + f[1], ope[1] + f[2], ope[2] ))
          rotate(f[0], ope[0] + f[1], ope[1] + f[2], ope[2])
      if isEnd(f[0]):
        ret.append(f[3])
        continue

      # 盤面を小さくする
      res = toSmallField(f[0])
      if res is None:
        ret.append(f[3])
        continue

      fields.extend((res[0], res[1], res[2], Tree(f[3])))
    if len(fields) == 0:
      break

  if len(ret) == 0:
    return None

  result = ret[0]
  for r in ret:
    if len(result) > len(r):
      result = r
  return result.get()





  



