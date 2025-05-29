# deep=4 84% 7.6G
# {2: 2M, 3: 80M} 

# isEnd無し
# deep: 4, 2.3458%	 installed: {0: 1712, 1: 73132, 2: 2501253, 3: 83674211, 4: 83674211}

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

  num += 1
  data_len = {0: len(target)}
  while target:
    print(f"[{nowT()}] Depth {num}, targets: {len(target)}")
    target = process_layer(db6, target, data_len, num, 6)
    num += 1
  print(f"[{nowT()}] Depth {num}, targets: {len(target)}")

