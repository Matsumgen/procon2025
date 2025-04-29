from itertools import product
from field4lib import *
import numpy as np
import lmdb
import time
# total: 2027025



if __name__ == "__main__":
  startTime = time.perf_counter()
  def nowT():
    return int(time.perf_counter() - startTime)
  db = lmdb.open('field4.buffer.db', map_size=int(1e8))
  print("[0]\tCreating DB...")
  creat4field(db)
  print(f"[{nowT()}]\tCreated")

  num = 0
  inserted = 0
  while True:
    fieldList = []
    print(f"[{nowT()}]\tLoading fieldList...")
    with db.begin() as txn:
      with txn.cursor() as cursor:
        for key, value in cursor:
          if value[1] == num:
            fieldList.append(key)
    fLL = len(fieldList)
    inserted += fLL
    print(f"[{nowT()}]\tLoaded fieldList")
    print(f"inserted: {inserted}/2027025")
    print("deep:", num)
    print("fieldList length:", fLL)
    if fLL == 0:
      break
    num += 1
    insert = 0
    for i, bfield in enumerate(fieldList):
      field = decodeField(bfield)
      field = np.array(field).reshape((4, 4))
      for y, x, n in product(range(3), range(3), range(2, 4)):
        if n == 3 and (x == 2 or y == 2):
          continue
        f = field.copy()
        rotate(f, x, y, n, rev=True)
        reallocation(f)
        dat = encodeField(f.flatten().tolist())
        with db.begin(write=True) as txn:
          value = txn.get(dat)
          if value == b'No' or value[1] > num:
            datparm = encodeAnswer([x, y, n], num)
            txn.put(dat, datparm)
            insert += 1
      if i % 10000 == 0:
        print(f"{i/fLL*100:.4f}\t{i}/{fLL}\t inserted: {insert}")
    print()

  print(f"[{nowT()}]\tOptimizing the database...")
  db4 = lmdb.open('field4.db', map_size=int(1e8))
  with db.begin() as txn:
    with db4.begin(write=True) as db4txn:
      with txn.cursor() as cursor:
        for key, value in cursor:
          db4txn.put(key, value[0].to_bytes())
  print(f"[{nowT()}]\tOptimized the database")

