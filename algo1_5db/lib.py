import numpy as np
import random

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



def randomField(fsize):
  fieldl = [ i for i in range(fsize * fsize // 2) for _ in range(2) ]
  random.shuffle(fieldl)
  field = np.array(fieldl).reshape((fsize,fsize))
  return field


def emptyfield(fsize):
  arr = np.full((fsize, fsize * 2 - 2), 4, dtype=int)
  arr[0:2, 0:fsize - 2] = 7
  return arr

def createfield(fsize, other:bool=False):
  arr = emptyfield(fsize)
  if other:
    arr[2, fsize - 4] = 0
    arr[3, fsize - 4] = 1
    arr[2, fsize - 3] = 2
    arr[3, fsize - 3] = 3
  else:
    arr[0, fsize - 2] = 0
    arr[1, fsize - 2] = 1
    arr[0, fsize - 1] = 2
    arr[1, fsize - 1] = 3
  return arr

# np.ndarryのfieldを90度回転
# rev=Trueで左回転
def rotate(field:np.ndarray, x:int, y:int, n:int, rev:bool=False):
  sub = field[y:y+n, x:x+n]
  field[y:y+n, x:x+n] = np.flipud(sub.T) if rev else np.fliplr(sub.T)

# 7byte
def encodeKey(field:np.ndarray, x1:int, x2:int, effort:int, ba:bool=False):
  assert 0 <= effort < 16
  assert 0 <= x1 <  46
  assert 0 <  x2 <= 46
    # 初期化
  ret = bytearray()

  # 上位3バイト: effort(4bit), x1(6bit), x2(6bit)
  ret.append((effort << 4) | (x1 >> 2))
  ret.append(((x1 & 0x3) << 6) | x2)

  buf = 0
  bbuf = 0
  a = 2
  for n in range(4):
    cod = np.where(field == n)
    if len(cod[0]) != 1:
      printfield(field)
      print(n, cod)
      raise ValueError("Field must contain each of 0–3 exactly once")

    for c in (cod[1][0] - x1, cod[0][0]):
      buf = (buf << 5) | c
      bbuf += 5
      while bbuf >= 8:
        bbuf -= 8
        b = (buf >> bbuf) & 0xFF
        ret.append(b)

  return ret if ba else bytes(ret)

def decodeKey(key:bytes, fsize:int):
  f = emptyfield(fsize)
  b0, b1, b2, b3, b4, b5, b6 = key
  n = b0 >> 4
  x1 = ((b0 & 0x0F) << 2) | (b1 >> 6)
  x2 = b1 & 0x3F
  # print(n, x1, x2)

  x = (b2 >> 3)
  y = ((b2 & 0x07) << 2) | (b3 >> 6)
  # print(x, y)
  f[y, x + x1] = 0

  x = (b3 >> 1) & 0x1F
  y = ((b3 & 0x01) << 4) | (b4 >> 4)
  # print(x, y)
  f[y, x + x1] = 1

  x = ((b4 & 0x0F) << 1) | (b5 >> 7)
  y = (b5 & 0x7F) >> 2
  # print(x, y)
  f[y, x + x1] = 2

  x = ((b5 & 0x03) << 3) | (b6 >> 5)
  y = b6 & 0x1F
  # print(x, y)
  f[y, x + x1] = 3

  return n, x1, x2, f

def parseKey(key:bytes):
  b0, b1, b2, b3, b4, b5, b6 = key
  n = b0 >> 4
  x1 = ((b0 & 0x0F) << 2) | (b1 >> 6)
  x2 = b1 & 0x3F

  p1 = [(b2 >> 3) + x1, ((b2 & 0x07) << 2) | (b3 >> 6)]
  p2 = [((b3 >> 1) & 0x1F) + x1, ((b3 & 0x01) << 4) | (b4 >> 4)]
  p3 = [(((b4 & 0x0F) << 1) | (b5 >> 7)) + x1, (b5 & 0x7F) >> 2]
  p4 = [(((b5 & 0x03) << 3) | (b6 >> 5)) + x1, b6 & 0x1F]
  return n, x1, x2, p1, p2, p3, p4
  

def checkField(field:np.ndarray, fsize:int):
  return np.all(field[0:2, 0:fsize - 2] == 7)

# 24 * 24までのfield対応
# xxxxxyyy yynnnnn0
# 外で xに-x1する
def encodeOperate(x:int, y:int, n:int):
  if 0 <= x < 32 and 0 <= y < 32 and 2 <= n < 32:
    return bytes([ (x << 3) | (y >> 2), ((y << 6) & 0xFF) | (n << 1) ])
  raise ValueError(f"encodeOperate: x={x}, y={y}, n={n}")

def decodeOperate(bop:bytes, duplicate:bool=False):
  if duplicate:
    ret = list()
    for i in range(0, len(bop), 2):
      ret.append([ bop[i] >> 3, ((bop[i] & 0x07) << 2) | (bop[i+1] >> 6), (bop[i+1] >> 1 & 0x1F)])
    return ret
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
      if d == 4:
        d = '.'
      elif d == 7:
        d = 'X'
      print(d, "", end='')
    print()
