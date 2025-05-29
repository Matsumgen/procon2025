from itertools import product
from field6lib import *
import numpy as np
import lmdb
import time
from tqdm import tqdm
# {0:  0, 1:  84, 2:  751, 3:  6057, 4:  41418, 5: 184080, 6:  254742, 7: 21133}

if __name__ == "__main__":
  startTime = time.perf_counter()
  def nowT():
    return int(time.perf_counter() - startTime)

  db4 = lmdb.open('field4.db', map_size=int(1e8))
  print("[0]\tCreating DB...")

  with open("pattern/4x4_patturn.txt", "r") as f:
    lines = [line.strip() for line in f if line.strip()]

  # 終了盤面取得
  lines = lines[1:]
  target = []
  num = 0
  txn =  db4.begin(write=True)
  for i in range(0, len(lines), 4):
    block = [list(map(int, line.split())) for line in lines[i:i+4]]
    if len(block) == 4 and all(len(row) == 4 for row in block):
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
    target = process_layer(db4, target, data_len, num, 4)
    num += 1
