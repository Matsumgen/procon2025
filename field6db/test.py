import random
import numpy as np
from field6lib import *
import lmdb
from tqdm import tqdm

db4 = lmdb.open('field4.db', map_size=int(1e8))
db6 = lmdb.open('_field6.db', map_size=int(1e8))

def printBytes(b:bytes):
  print(' '.join(format(bb, '08b') for bb in b))
  

# ランダムな盤面を扱う
# 平均13.61byte
def test1():
  enc_len = []
  for _ in range(100000):
    fieldl = [ i for i in range(18) for _ in range(2) ]
    random.shuffle(fieldl)
    field = np.array(fieldl).reshape((6,6))
    reallocation(field)
    # print(field)
    encf = encodeField(field)
    enc_len.append(len(encf))
    # printBytes(encf)
    decf = decodeField(encf)
    # print(decf)
    if len(encf) >= 16:
      print(fieldl)
    if not np.all(field == decf):
      print(fieldl)
    # print()
  print("min:", min(enc_len), ", max:", max(enc_len), ", ave:", sum(enc_len)/len(enc_len))

#ランダムな盤面を解いてみる
def test2(fsize=4):
  steps = []
  for _ in range(1):
    fieldl = [ i for i in range(fsize*fsize//2) for _ in range(2) ]
    random.shuffle(fieldl)
    field = np.array(fieldl).reshape((fsize,fsize))
    answer = resolve([db4], field.copy())
    steps.append(len(answer))
    for f in field:
      for e in f:
        print(e, sep='', end=' ')
      print()
    for a in answer:
      print(a[0], a[1], a[2])
      rotate(field, a[0], a[1], a[2])
    if not isEnd(field):
      print(fieldl)
      raise Exception("Didn't resolve problem")
  steps = np.array(steps)
  print(f"mean:{np.mean(steps)}, min:{np.min(steps)}, max:{np.max(steps)}, median:{np.median(steps)}, std:{np.std(steps)}")

#現在登録済みの盤面一覧
def test3(db, fsize=4):
  count = 0
  step = { i: 0 for i in range(fsize*fsize//2) }
  b = {}
  with db.begin() as txn:
    with txn.cursor() as cursor:
      for key, value in tqdm(cursor):
        opes = resolve([db], decodeField(key, fsize))
        if len(key) in b.keys():
          b[len(key)] += 1
        else:
          b[len(key)] = 1

        if opes is None:
          print(key, value)
        step[len(opes)] += 1
        count += 1

  print("total:", count)
  print("step: ", step)
  print("bytes:", b)
# total: 508265
# step:  {0: 0, 1: 84, 2: 751, 3: 6057, 4: 41418, 5: 184080, 6: 254742, 7: 21133}
# bytes: {1: 2, 2: 2392, 3: 91889, 4: 381431, 5: 32551}








test3(db6)
