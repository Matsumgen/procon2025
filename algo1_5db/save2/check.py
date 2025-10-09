from lib import *
from tqdm import tqdm
import lmdb
import os
FIELD_SIZE = 18
db = lmdb.open(f'algo1_5_2_{FIELD_SIZE}.db', map_size=16 * 1024 * 1024 * 1024)
newdb = lmdb.open(f'algo1_5_2_{FIELD_SIZE}_checked.db', map_size=16 * 1024 * 1024 * 1024)
X_SIZE = FIELD_SIZE * 2 - 2

# 重複を消して、yが小さい順に並べて保存
d = dict()
with db.begin() as txnr:
  with txnr.cursor() as cursor:
    with newdb.begin(write=True) as txnw:
      try:
        for fkey, val in tqdm(cursor, total=txnr.stat()['entries']):
          # depth, X1, X2, p1, p2, p3, p4 = parseKey(fkey)
          opes = decodeOperate(val, True)
          opes.sort(key=lambda a: a[1] + a[2])
          txnw.put(fkey, encodeOperateL(opes))

          # print(depth, X1, X2, p1, p2, p3, p4, len(opes))
      except KeyboardInterrupt:
        print("end")


for k in sorted(d.keys()):
  print(f"{k}: {d[k]}, ", end='')
