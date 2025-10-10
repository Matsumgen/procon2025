# deep=4 84% 7.6G
# {2: 2M, 3: 80M} 

#  installed: {0: 1712, 1: 71420}

from itertools import product
from field6lib import *
import numpy as np
import lmdb
import time
import tempfile
import struct
import os

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
  num = 0
  txn =  db6.begin(write=True)
  count = 0
  with tempfile.NamedTemporaryFile(delete=False) as f:
    for i in range(0, len(lines), 6):
      block = [list(map(int, line.split())) for line in lines[i:i+6]]
      if len(block) == 6 and all(len(row) == 6 for row in block):
        block = np.array(block)
        key, _ = putDB(txn, block, [0, 0, 2])
        if key is not None:
          txn.put(key, b'\xff')
          f.write(struct.pack("B", len(key)))
          f.write(key)
          count += 1
    target = f.name

  txn.abort()
  del lines

  print(f"[{nowT()}]\tLoaded fieldList")
  print(f"inserted[byte]: {os.path.getsize(target)}")
  print("deep:", num)

  data_len = {0: count}
  num += 1
  while target is not None and count != 0:
    print(f"[{nowT()}] Depth {num}, targets: {count}")
    ret_target, count = process_layer(db6, target, count, 6)
    os.remove(target)
    target = ret_target
    data_len[num] = count
    num += 1
  print(f"[{nowT()}] Depth {num}, targets: {count}")
  print("result:", data_len)

