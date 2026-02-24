import numpy as np
from itertools import product
import struct

FSIZE = 24
SELLS = FSIZE * FSIZE

matrix = np.zeros((SELLS, SELLS), dtype=np.bool_)

# 1乗, 2乗の隣接行列作成 -> 4手までの探索が可能
# if文は中身が少ないため影響小(だといいな)
# gpuで経路探索 -> 別カーネルで回転・評価
# fieldはtypeを持つ
# typeはfieldの状態を表す(別の状態へのパスを一意に決める・グラフのノード)

def isInField(x, y):
  return 0 <= x < FSIZE and 0 <= y < FSIZE

# to=(x1,y1), from=(x2,y2)
def canRotate(x1, y1, x2, y2):
  dx = x2 - x1
  dy = y2 - y1
  X = abs(dx)
  Y = abs(dy)
  n = X + Y + 1
  if dx > 0 and dy >= 0:
    x = x1 - Y
    y = y1
  elif dx <= 0 and dy > 0:
    x = x2 - Y
    y = y1 - X
  elif dx > 0:
    x = x1
    y = y2
  else:
    x = x2
    y = y2 - X

  return isInField(x, y) and isInField(x + n - 1, y + n - 1)



for x1 in range(FSIZE):
  for y1 in range(FSIZE):
    for x2 in range(FSIZE):
      for y2 in range(FSIZE):
        if canRotate(x1, y1, x2, y2) and (x1, y1) != (x2, y2):
          f = y1 * FSIZE + x1
          t = y2 * FSIZE + x2
          matrix[f, t] = True


# matrix
# 移動経路の最大は287個
# 経路の数は対称性がある
# 経路は中心で点対象
# 要素の数110400


print(matrix.sum())
# print(all([ matrix[i, j] == matrix[SELLS - i - 1, SELLS - j - 1] for i, j in product(range(SELLS), range(SELLS)) ]))
# m =  matrix.sum(axis=1)
# print(all([m[i] == m[SELLS - i - 1] for i in range(SELLS)]))
# print(m)

# for f in range(SELLS):
#   for t in range(SELLS):
#     print(f"{int(matrix[f, t])} ", end='')
#   print()


# matrix ^ 2
# matrixと同じ
# 経由インデックスはj = (SELLS - i - 1)
# keyの数は271964/331776
matrix2 = [ [ list() for i in range(SELLS) ] for j in range(SELLS) ]
m2_num = np.zeros((SELLS, SELLS), dtype=np.int32)
for f in range(SELLS):
  for t in range(SELLS):
    if f != t:
      common = matrix[f] & matrix[:, t]
      matrix2[f][t] = np.where(common)[0].tolist()
      m2_num[f][t] += len(matrix2[f][t])

# print(m2_num.max())
# print((m2_num != 0).sum())

# str_matrix = [[str(matrix2[f][t]) for t in range(SELLS)] for f in range(SELLS)]
# max_width = max(len(cell) for row in str_matrix for cell in row)
# for row in str_matrix:
#   print(" ".join(cell.ljust(max_width) for cell in row))



# メモリ使用量計算
# matrix(adjacency):     1296.0[KB]
# matrix(associative1):  107.8125[KB]
# matrix(associative2):  60.64453125[KB]
# matrix2(associative1): 22.18719482421875[MB]
# matrix2(associative2): 12.480297088623047[MB]

# 隣接行列そのまま
# print(f"matrix(adjacency):     {SELLS * (SELLS >> 1) * 8 / 1024}[KB]")
# uint16_tでインデックスを格納した二次元配列[t*SELLS+f][i]
# print(f"matrix(associative1):  {matrix[:(SELLS >> 1), :].sum() * 2 / 1024}[KB]")
# 9bitでインデックスを格納
# print(f"matrix(associative2):  {matrix[:(SELLS >> 1), :].sum() * 9 / 8 / 1024}[KB]")

# print(f"matrix2(associative1): {m2_num[:(SELLS) >> 1, :].sum() * 2 / 1024 / 1024}[MB]")
# print(f"matrix2(associative2): {m2_num[:(SELLS >> 1), :].sum() * 9 / 8 / 1024 / 1024}[MB]")


# ファイル保存(associative採用)
# matrix
# 形式: from(2), len(2), [to(2), ...]
# with open("adjacency_matrix1.bin", 'wb') as f:
#   for fi in range(SELLS>>1):
#     indices = np.where(matrix[fi])[0].astype(np.uint16)
#     l = len(indices)
#     if l > 0:
#       f.write(struct.pack(">HH", fi, l))
#       f.write(b"".join([ struct.pack(">H", d) for d in indices ]))

# matrix2
# 形式:　from(2), to(2), len(2), [i(2), ...]
# with open("adjacency_matrix2.bin", 'wb') as f:
#   for fi in range(SELLS>>1):
#     for ti in range(SELLS):
#       target = matrix2[fi][ti]
#       if target:
#         f.write(struct.pack(">HHH", fi, ti, len(target)))
#         f.write(b"".join([ struct.pack(">H", d) for d in target ]))

