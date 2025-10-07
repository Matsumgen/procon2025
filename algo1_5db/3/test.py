from lib import *
import lmdb

p1 = (0, 0)
p2 = (3, 0)
p1 = rotateP(p1)
p2 = rotateP(p2)
print(p1, p2)
db6 = lmdb.open('algo1_5_3.db', map_size=8 * 1024 * 1024 * 1024)
with db6.begin() as txn:
  opes = decodeValue(txn.get(encodeKey(p1, p2)))

print(opes)
d = dict()
for i in range(6):
  d[i] = 0

for f in cgenerator(p1, p2):
  flag = False
  f = np.array(f).reshape((BOARD_SIZE, BOARD_SIZE))  
  for ope in opes:
    _f = f.copy()
    for o in ope:
      rotate(_f, o[0], o[1], o[2])

    if isEnd(_f):
      flag = True
      d[len(ope)] += 1

  if not flag:
    print("don't clear")
    printfield(f)

print(d)
