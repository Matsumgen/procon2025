
import numpy as np
import struct

FSIZE = 24
SELLS = FSIZE * FSIZE

matrix = np.zeros((SELLS, SELLS), dtype=np.bool_)
with open("adjacency_matrix1.bin", 'rb') as f:
  data = f.read()
data = struct.unpack('>' + 'H' * (len(data) // 2), data)

i = 0
while i < len(data):
  fi = data[i]
  length = data[i+1]
  i += 2
  for j in range(i, i+length):
    matrix[fi, data[j]] = True
  i += length

matrix[(SELLS>>1):, :] = matrix[:(SELLS>>1), :][::-1, ::-1]


matrix2 = [ [ list() for i in range(SELLS) ] for j in range(SELLS) ]
with open("adjacency_matrix2.bin", 'rb') as f:
  data = f.read()
data = struct.unpack('>' + 'H' * (len(data) // 2), data)

i = 0
while i < len(data):
  fi = data[i]
  ti = data[i+1]
  length = data[i+2]
  i += 3
  matrix2[fi][ti] = [data[j] for j in range(i, i+length)]
  i += length

matrix2[(SELLS>>1):] = [[[SELLS - d - 1 for d in dl[::-1]] for dl in row[::-1]] for row in matrix2[:(SELLS>>1)][::-1]]






m = np.zeros((SELLS, SELLS), dtype=np.bool_)
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
          m[f, t] = True

m2 = [ [ list() for i in range(SELLS) ] for j in range(SELLS) ]
m2_num = np.zeros((SELLS, SELLS), dtype=np.int32)
for f in range(SELLS):
  for t in range(SELLS):
    if f != t:
      common = matrix[f] & matrix[:, t]
      m2[f][t] = np.where(common)[0].tolist()
      m2_num[f][t] += len(m2[f][t])



print(np.all(m == matrix))
print(matrix2 == m2)
