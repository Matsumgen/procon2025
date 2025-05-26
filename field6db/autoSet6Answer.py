# deep=4 84% 7.6G
from itertools import product
from field6lib import *
import numpy as np
import lmdb
import time

def hsize(num: float, suffix: str = 'B') -> str:
  for unit in ['', 'K', 'M', 'G', 'T', 'P']:
    if abs(num) < 1024.0:
      return f"{num:.1f}{unit}{suffix}"
    num /= 1024.0
  return f"{num:.1f}E{suffix}"


if __name__ == "__main__":
  startTime = time.perf_counter()
  def nowT():
    return int(time.perf_counter() - startTime)
  db = lmdb.open('field6.db', map_size=int(1e10)) # 約9.31G
  print("[0]\tCreating DB...")

  with open("pattern/6x6_patturn.txt", "r") as f:
    lines = [line.strip() for line in f if line.strip()]

  # 終了盤面取得
  lines = lines[1:]
  target = []
  bytes_count = 0
  num = 0
  with db.begin(write=True) as txn:
    for i in range(0, len(lines), 6):  # 6行ごとに切り取る
      block_lines = lines[i:i+6]
      block = [list(map(int, line.split())) for line in block_lines]
      if len(block) == 6 and all(len(row) == 6 for row in block):  # 安全チェック
        key, _ = putDB(txn, np.array(block), [0, 0, 2])
        if key is not None:
          bytes_count += len(key) + 1
          target.append(key)
          txn.put(key, b'\xff')
  del lines

  print(f"[{nowT()}]\tLoaded fieldList")
  print(f"inserted: {len(target)}")
  print("deep:", num)

  num += 1
  fLL = len(target)
  with db.begin(write=True) as txn:
    while fLL != 0:
      print(f"[{nowT()}]\ttarget: {len(target)}\tdeep: {num}")
      _target = target
      target = []
      for i, bfield in enumerate(_target):
        field = decodeField(bfield)
        field = np.array(field).reshape((6, 6))
        for y, x, n in product(range(5), range(5), range(2, 6)): # y, x, n
          if max(y+n, x+n) >= 6:
            continue
          f = field.copy()
          rotate(f, x, y, n, rev=True)
          ope = [x, y, n]
          key, value = putDB(txn, f, ope)
          if key is not None:
            bytes_count += len(key) + 1
            target.append(key)
            txn.put(key, value)
        if i % 10000 == 0:
          print(f"{i/fLL*100:.4f}%\t{i}/{fLL}\tinserted: {len(target)}\tstorage: {hsize(bytes_count)}")
      num += 1
      fLL = len(target)




