# deep=4 84% 7.6G
# {2: 2M, 3: 80M} 
# deep: 3, 9.9897%	 installed: {0: 1712, 1: 73132, 2: 2501253, 3: 2501253}
from itertools import product
from field6lib import *
import numpy as np
import lmdb
import time

if __name__ == "__main__":
  startTime = time.perf_counter()
  def nowT():
    return int(time.perf_counter() - startTime)

  db6 = lmdb.open('field6.db', map_size=int(1e11))
  print("[0]\tCreating DB...")

  with open("pattern/6x6_patturn.txt", "r") as f:
    lines = [line.strip() for line in f if line.strip()]

  # 終了盤面取得
  lines = lines[1:]
  target = []
  num = 0
  txn =  db6.begin(write=True)
  for i in range(0, len(lines), 6):
    block = [list(map(int, line.split())) for line in lines[i:i+6]]
    if len(block) == 6 and all(len(row) == 6 for row in block):
      block = np.array(block)
      key, _ = putDB(txn, block, [0, 0, 2])
      if key is not None:
        txn.put(key, b'\xff')
        target.append(key)

  txn.abort()
  del lines

  print(f"[{nowT()}]\tLoaded fieldList")
  print(f"inserted: {len(target)}")
  print("deep:", num)

  fLL = len(target)
  data_len = {0: fLL}
  num += 1
  while fLL != 0:
    print(f"[{nowT()}]\tdeep: {num}\ttarget: {len(target)}")
    _target = target
    target = []
    with db6.begin(write=True) as txn:
      try:
        for i, bfield in enumerate(_target):
          field = decodeField(bfield, 6)
          field = np.array(field).reshape((6, 6))
          for y, x, n in product(range(5), range(5), range(2, 6)): # y, x, n
            if max(y+n, x+n) > 6:
              continue
            f = field.copy()
            rotate(f, x, y, n, rev=True)
            ope = [x, y, n]
            key, value = putDB(txn, f, ope)
            if key is not None:
              target.append(key)
              txn.put(key, value)
          if i % 10000 == 0 and i != 0:
            print(f"{i/fLL*100:2.4f}%\t{i}/{fLL}\tinserted: {len(target)}")
        fLL = len(target)
        data_len[num] = fLL
        num += 1
      except KeyboardInterrupt:
        data_len[num] = fLL
        print()
        print(f"deep: {num}, {i/fLL*100:2.4f}%\t installed: {data_len}")
        fLL = 0
      except Exception as e:
        data_len[num] = fLL
        print(e)
        print(f"deep: {num}, {i/fLL*100:2.4f}%\t installed: {data_len}")
        fLL = 0




