import numpy as np
import random

N0 = 0
N1 = 1
NULL_CELL = 2
CONST_CELL = 3

FIELD_SIZE = 24
MIN_FSIZE = 10
XY_RANGE_MAX = FIELD_SIZE - MIN_FSIZE - 1
N_RANGE_MAX = FIELD_SIZE - MIN_FSIZE + 1

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

# np.ndarryのfieldを90度回転
# rev=Trueで左回転
def rotate(field:np.ndarray, x:int, y:int, n:int, rev:bool=False):
  sub = field[y:y+n, x:x+n]
  field[y:y+n, x:x+n] = np.flipud(sub.T) if rev else np.fliplr(sub.T)

def randomField(fsize):
  fieldl = [ i for i in range(fsize * fsize // 2) for _ in range(2) ]
  random.shuffle(fieldl)
  field = np.array(fieldl).reshape((fsize,fsize))
  return field

# 24 * 24までのfield対応
# xxxxxyyy yynnnnn0
# 外で xに-x1する
def encodeOperate(x:int, y:int, n:int):
  if 0 <= x < 32 and 0 <= y < 32 and 2 <= n < 32:
    return bytes([ (x << 3) | (y >> 2), ((y << 6) & 0xFF) | (n << 1) ])
  raise ValueError(f"encodeOperate: x={x}, y={y}, n={n}")

def encodeOperateL(opes:list):
  ret = bytearray()
  for x, y, n in opes:
    ret.append((x << 3) | (y >> 2))
    ret.append(((y << 6) & 0xFF) | (n << 1))
  return bytes(ret)

def decodeOperate(bop:bytes, duplicate:bool=False):
  if duplicate:
    ret = set()
    for i in range(0, len(bop), 2):
      ret.add(( bop[i] >> 3, ((bop[i] & 0x07) << 2) | (bop[i+1] >> 6), (bop[i+1] >> 1 & 0x1F)))
    return list(ret)
  return [ bop[0] >> 3, ((bop[0] & 0x07) << 2) | (bop[1] >> 6), (bop[1] >> 1 & 0x1F)]

def rotateP(p, fsize):
  return (fsize - p[1] - 1, p[0])

def rotateR(r, fsize):
  return (fsize - (r[1] + r[2] - 1) - 1, r[0], r[2])

def printf(f):
  for y in range(f.shape[0]):
    for x in range(f.shape[1]):
      print(f"{f[y, x]:>3}", end=' ')
    print()

def checkBox(f, p1, p2, p3, p4):
  return f[p1[1], p1[0]] == f[p2[1], p2[0]] and f[p3[1], p3[0]] == f[p4[1], p4[0]] \
      or f[p1[1], p1[0]] == f[p3[1], p3[0]] and f[p2[1], p2[0]] == f[p4[1], p4[0]]



def printfield(f:np.ndarray):
  for y in range(f.shape[0]):
    for x in range(f.shape[1]):
      d = f[y, x]
      if d == NULL_CELL:
        d = '.'
      elif d == CONST_CELL:
        d = 'X'
      print(d, "", end='')
    print()

def emptyfield(fsize):
  xy = fsize * 2 - 2
  arr = np.full((xy, xy), NULL_CELL, dtype=int)
  arr[fsize - 2: fsize, 0: fsize - 2] = CONST_CELL

# FIELD_SIZE = 24, MIN_FSIZE = 10, depth < 4 -> 13bit (12.89)
# FIELD_SIZE = 24, MIN_FSIZE = 12, depth < 4 -> 13bit (12.20)
def encodeIndex(depth:int, X:int, Y:int, N:int):
  assert 1 <= depth
  assert 0 <= X < XY_RANGE_MAX
  assert 0 <= Y < XY_RANGE_MAX
  assert MIN_FSIZE <= N < FIELD_SIZE + 1
  return  (depth - 1) * (( XY_RANGE_MAX ** 2) * N_RANGE_MAX)\
        + X * XY_RANGE_MAX * N_RANGE_MAX \
        + Y * N_RANGE_MAX\
        + (N - MIN_FSIZE)

def decodeIndex(index: int):
  unit_n = N_RANGE_MAX
  unit_y = unit_n
  unit_x = XY_RANGE_MAX * unit_n
  unit_depth = XY_RANGE_MAX * XY_RANGE_MAX * unit_n
  depth = index // unit_depth + 1
  rem = index % unit_depth
  X = rem // unit_x
  rem = rem % unit_x
  Y = rem // unit_y
  rem = rem % unit_y
  N = rem + MIN_FSIZE
  return depth, X, Y, N

def encodeKey(field:np.ndarray, depth:int, X:int, Y:int, N:int):
  ret = bytearray()
  index = encodeIndex(depth, X, Y, N)
  pairs0 = list(zip(*np.where(field == N0)))
  pairs1 = list(zip(*np.where(field == N1)))
  if len(pairs0) != 2 or len(pairs1) != 2:
    print("pairs0:", pairs0)
    print("pairs1:", pairs1)
    raise ValueError("pairs is not founds")


def decodeKey(key:bytes, fsize:int):
  pass
def parseKey(key:bytes):
  pass
def checkField(field:np.ndarray, fsize:int):
  pass


