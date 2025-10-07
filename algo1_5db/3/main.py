from lib import *
from save import *
from itertools import product
from tqdm import tqdm
import lmdb
# ((0, 0), (2, 0)) -> None
# ((0, 0), (4, 0)) -> None

SEARCH_DEPTH = 3
LENGTH_MAX = 5
db6 = lmdb.open('algo1_5_3.db', map_size=8 * 1024 * 1024 * 1024)

def all_search(field, deep=2):
  def getMinRet(ret):
    minS = 9999999
    true_ret = ret[0]
    for r in ret:
      _, _, _, _, fc = getCache(r, field)
      if fc.shape[0] * fc.shape[1] < minS:
        true_ret = r
        minS = fc.shape[0] * fc.shape[1]
    return true_ret

  buf = [(field, [])]
  nbuf = []
  ret = []
  eflag = False
  for count in range(deep):
    for _f, l in tqdm(buf, leave=False):
      for x, y, n in product(range(9), range(9), range(2, 10)):
        if max(x + n, y + n) > 10: continue
        f = _f.copy()
        rotate(f, x, y, n)
        ll = l + [(x, y, n)]

        if isEnd(f):
          ret.append(ll)
          eflag = True
          if count >= 2:
            return getMinRet(ret)
        elif not eflag and count < deep - 1:
          nbuf.append((f, ll))

    if ret:
      return getMinRet(ret)
          
    buf = nbuf
    nbuf = []
  return None


def resolv(p1, p2):
  print("Advance input")
  s = input()
  buf = list()
  buf2 = list()
  while s != "end":
    if s == "next":
      buf.append(buf2)
      buf2 = list()
    else:
      parts = s.strip().split()
      if len(parts) != 3 or not all(part.isdigit() for part in parts):
        print("不正な文字列")
      else:
        buf2.append(( int(parts[0]), int(parts[1]), int(parts[2]) ))
    s = input()
    

  print("buf: ", buf)
  buf2 = list()
  try:
    for f in cgenerator(p1, p2):
      flag = True
      _ff = np.array(f).reshape((BOARD_SIZE, BOARD_SIZE))  
      for r in buf:
        if len(r) > LENGTH_MAX:
          break
        ff = _ff.copy()
        for a in r:
          rotate(ff, a[0], a[1], a[2])

        if isEnd(ff):
          CACHE.append(getCache(r, _ff))
          flag = False
          break

      if not flag:
        continue

      r = all_search(_ff, SEARCH_DEPTH)
      if r is not None:
        print("search:", r)
        buf.append(r)
        buf.sort(key=lambda x: len(x))
        CACHE.append(getCache(r, _ff))
        flag = False

      if flag:
        buf2.append(_ff)

    print("buf:", buf)
    print("buf2 lenght:", len(buf2))
    for f in buf2:
      flag = True
      for r in buf:
        if len(r) > LENGTH_MAX:
          break
        ff = f.copy()
        for a in r:
          rotate(ff, a[0], a[1], a[2])

        if isEnd(ff):
          CACHE.append(getCache(r, f))

      printfield(f)
      r = list()
      s = input()
      while s != 'end':
        parts = s.strip().split()
        if len(parts) != 3 or not all(part.isdigit() for part in parts):
          print("不正な文字列")
        else:
          r.append(( int(parts[0]), int(parts[1]), int(parts[2]) ))
        s = input()

      ff = f.copy()
      for a in r:
        rotate(ff, a[0], a[1], a[2])

      if isEnd(ff):
        print("clear: ", r)
        print()
        if r not in buf:
          buf.append(r)
          buf.sort(key=lambda x: len(x))
          if len(r) <= LENGTH_MAX:
            CACHE.append(getCache(r, _ff))
        break
      else:
        print(ff)
        print("完成していません")
  except KeyboardInterrupt:
    print()
    print(p1, p2)
    print(buf)
    exit(1)
  return buf



print("select mode: manual(0) auto(1)")
s = input()

if s == '0':
  while True:
    print("p1: ", end='')
    s = input()
    parts = s.strip().split()
    if len(parts) == 2 and all(part.isdigit() for part in parts):
      p1 = (int(parts[0]), int(parts[1]))
      break

  while True:
    print("p2: ", end='')
    s = input()
    parts = s.strip().split()
    if len(parts) == 2 and all(part.isdigit() for part in parts):
      p2 = (int(parts[0]), int(parts[1]))
      break

  print(f"p1 = {p1}, p2 = {p2}")
  with db6.begin() as txn:
    flag = txn.get(encodeKey(p1, p2))

  if flag is not None:
    print("This is finishid. one more charenge?")
    s  = input()
    if s != "yes":
      print("end.")
      exit()
    else:
      for ope in decodeValue(flag):
        for o in ope:
          print(o[0], o[1], o[2])
        print()
  buf = resolv(p1, p2)
  print(f"p1 = {p1}, p2 = {p2}")
  print(buf)
  with db6.begin(write=True) as txn:
    save_lmdb(txn, p1, p2, buf)

elif s == '1':
  for y1, x1, y2, x2 in product(range(10), range(10), range(10), range(10)):
    p1 = (x1, y1)
    p2 = (x2, y2)
    if y2 < y1 or (y1 <= y2 and x2 < x1):
      c = p1
      p1 = p2
      p2 = c
    with db6.begin() as txn:
      flag = txn.get(encodeKey(p1, p2))

    if flag is not None:
      # print("conttinue:", p1, p2)
      continue

    print(f"p1 = {p1}, p2 = {p2}")
    buf = resolv(p1, p2)


    print(f"p1 = {p1}, p2 = {p2}")
    print(buf)
    with db6.begin(write=True) as txn:
      save_lmdb(txn, p1, p2, buf)
