from functools import wraps
import numpy as np

BOARD_SIZE = 10
BOARD_MAX_IDX = 99
NUM_PAIRS = 50

NULL_NUM = 99

WRAP_MAX_DEPTH = 30

CACHE = list()

def trace(func):
  @wraps(func)
  def wrapper(*args, **kwargs):
    indent = " " * wrapper.depth
    if wrapper.depth < WRAP_MAX_DEPTH:
      print(f"{indent}> {func.__name__}({args[1:]})")
    wrapper.depth += 1
    result = func(*args, **kwargs)
    wrapper.depth -= 1
    if wrapper.depth < WRAP_MAX_DEPTH:
      r = None if result is None else len(result)
      print(f"{indent}< {func.__name__}({args[1:]}) = {r}")
    return result
  wrapper.depth = 0
  return wrapper

def ptoi(x, y):
  return y * BOARD_SIZE + x
def ptoi(p):
  return p[1] * BOARD_SIZE + p[0]
  
def itop(i):
  return (i % BOARD_SIZE, i // BOARD_SIZE)

def adjacent(p1, p2):
  return abs(p1[0] - p2[0]) + abs(p1[1] - p2[1]) == 1

def create(p1, p2):
  if adjacent(p1, p2):
    print('adjacent:', p1, p2)
    return None
  f = [ NULL_NUM for _ in range(BOARD_SIZE * BOARD_SIZE) ]
  f[ptoi(p1)] = 0
  f[ptoi(p2)] = 0
  return _create(f, 0, 1)

@trace
def _create(f, i, n):
  while i <= BOARD_MAX_IDX and f[i] != NULL_NUM:
    i += 1

  if i > BOARD_MAX_IDX:
    return [f]

  ret = list()
  x, y = itop(i)
  if x < 9 and f[i+1] == NULL_NUM:
    ff = f.copy()
    ff[i] = n
    ff[i+1] = n
    r = _create(ff, i+2, n+1)
    if r:
      ret.extend(r)

  if y < 9 and f[i + BOARD_SIZE] == NULL_NUM:
    ff = f.copy()
    ff[i] = n
    ff[i+BOARD_SIZE] = n
    r = _create(ff, i+1, n+1)
    if r:
      ret.extend(r)

  return ret if ret else None
    
def cgenerator(p1, p2):
  if adjacent(p1, p2):
    print('adjacent:', p1, p2)
    return
  f = [NULL_NUM for _ in range(BOARD_SIZE * BOARD_SIZE)]
  f[ptoi(p1)] = 0
  f[ptoi(p2)] = 0
  yield from _cgenerator(f, 0, 1)

def _cgenerator(f, i, n):
  global CACHE
  _f = np.array(f).reshape((BOARD_SIZE, BOARD_SIZE))  
  for x1, x2, y1, y2, fc in CACHE:
    if np.all(_f[y1:y2, x1:x2] == fc):
      # print("in chace")
      return
  while i <= BOARD_MAX_IDX and f[i] != NULL_NUM:
    i += 1

  if i > BOARD_MAX_IDX:
    yield f
    return

  x, y = itop(i)

  # 横に置ける
  if x < 9 and f[i+1] == NULL_NUM:
    ff = f.copy()
    ff[i] = n
    ff[i+1] = n
    yield from _cgenerator(ff, i+2, n+1)

  # 縦に置ける
  if y < 9 and f[i + BOARD_SIZE] == NULL_NUM:
    ff = f.copy()
    ff[i] = n
    ff[i+BOARD_SIZE] = n
    yield from _cgenerator(ff, i+1, n+1)


def printfield(f):
  if f is None:
    print(None)
    return
  if type(f) is list:
    for i, n in enumerate(f):
      print(f'{n:>{2}}', end=' ')
      if i % BOARD_SIZE == BOARD_SIZE - 1:
        print()
  else:
    for y in range(f.shape[0]):
      for x in range(f.shape[1]):
        print(f"{f[y, x]:>3}", end=' ')
      print()

def rotate(f, x, y, n, rev=False):
  sub = f[y:y+n, x:x+n]
  f[y:y+n, x:x+n] = np.flipud(sub.T) if rev else np.fliplr(sub.T)

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

def encodeKey(p1, p2):
  i1 = ptoi(p1)
  i2 = ptoi(p2)
  v = (i1 << 10) | i2
  return bytes([ (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF ])

def decodeKey(d):
  v = (d[0] << 16) | (d[1] << 8) | d[2]
  a = (v >> 10) & 0x3FF
  b = v & 0x3FF
  return itop(a), itop(b)

# x: 0 ~ 9 -> 4bit
# y: 0 ~ 9 -> 4bit
# n: 2 ~ 9 -> 4bit
def encodeValue(buf):
  ret = bytearray()

  for r in buf:
    for (x, y, n) in r:
      if not (0 <= x < 10 and 0 <= y < 10 and 2 <= n < 10):
        raise ValueError(f"Invalid input: ({x}, {y}, {n})")
      ret.append((x << 4) | y)
      ret.append(n) # 終了時は少しかえる
    ret[-1] += 0xF0

  return bytes(ret)

def decodeValue(data: bytes):
  assert len(data) % 2 == 0, "データ長が偶数である必要があります"
  ret = []
  buf = []
  for i in range(0, len(data), 2):
    byte1 = data[i]
    byte2 = data[i + 1]
    x = (byte1 >> 4) & 0xF
    y = byte1 & 0xF
    n = byte2 & 0xF
    buf.append((x, y, n))
    if byte2 > 0x0F:
      ret.append(buf)
      buf = []

  return ret

def rotateP(p):
  return (9 - p[1], p[0])

def rotateR(r):
  return (9 - (r[1] + r[2] - 1), r[0], r[2])

def getCache(r, f):
  x1, x2, y1, y2 = None, None, None, None
  for ope in r:
    if x1 is None:
      x1 = ope[0]
      y1 = ope[1]
      x2 = x1 + ope[2]
      y2 = y1 + ope[2]
    else:
      if ope[0] < x1:
        x1 = ope[0]
      if ope[1] < y1:
        y1 = ope[1]
      if x2 < ope[0] + ope[2]:
        x2 = ope[0] + ope[2]
      if y2 < ope[1] + ope[2]:
        y2 = ope[1] + ope[2]
  return x1, x2, y1, y2, f[y1:y2, x1:x2].copy()



# ((0, 0), (2, 0)) -> None
if __name__ == '__main__':

  for i, f in enumerate(cgenerator((0, 0), (3, 0))):
    print(i, CACHE)
    printfield(f)
    s = input()
    if i == 2:
      _ff = np.array(f).reshape((BOARD_SIZE, BOARD_SIZE))  
      CACHE.append(getCache([ (2, 1, 2), (0, 0, 3) ], _ff))

  print("-----end-----")
