from lib import *
import lmdb


def save_lmdb(txn, p1, p2, opes):
  k = encodeKey(p1, p2)
  v = encodeValue(opes)
  txn.put(k, v)

  p1_1 = rotateP(p1)
  p2_1 = rotateP(p2)
  opes_1 = []
  for ope in opes:
    b = []
    for o in ope:
      b.append(rotateR(o))
    opes_1.append(b)

  # print(p1_1, p2_1, '\n', opes_1)
  k = encodeKey(p1_1, p2_1)
  v = encodeValue(opes_1)
  txn.put(k, v)

  p1_2 = rotateP(p1_1)
  p2_2 = rotateP(p2_1)
  opes_2 = []
  for ope in opes_1:
    b = []
    for o in ope:
      b.append(rotateR(o))
    opes_2.append(b)

  # print(p1_2, p2_2, '\n', opes_2)
  k = encodeKey(p1_2, p2_2)
  v = encodeValue(opes_2)
  txn.put(k, v)

  p1_3 = rotateP(p1_2)
  p2_3 = rotateP(p2_2)
  opes_3 = []
  for ope in opes_2:
    b = []
    for o in ope:
      b.append(rotateR(o))
    opes_3.append(b)

  # print(p1_3, p2_3, '\n', opes_3)
  k = encodeKey(p1_3, p2_3)
  v = encodeValue(opes_3)
  txn.put(k, v)
  print("saved: ", (p1, p2), (p1_1, p2_1), (p1_2, p2_2), (p1_3, p2_3))



 
if __name__ == '__main__':
  p1 = (0, 0)
  p2 = (3, 0)
  buf = [[(0, 0, 3)], [(2, 1, 2), (0, 0, 3)], [(0, 2, 2), (0, 0, 3)], [(1, 2, 2), (0, 0, 3)], [(0, 0, 2), (1, 0, 2), (2, 0, 2)], [(2, 2, 2), (1, 2, 2), (0, 0, 3)], [(2, 2, 6), (1, 2, 2), (0, 0, 3)], [(1, 1, 8), ( 1, 1, 8), (0, 0, 3)], [(1, 2, 2), (2, 3, 2), (0, 0, 3)], [(5, 2, 2), (2, 1, 4), (0, 0, 3)], [(1, 2, 2), (0, 0, 3), (3, 3, 2), (2, 3, 2)], [(8, 7, 2), (1, 1, 8), (1, 1, 8), (0, 0, 3)], [(1, 2, 2), (2, 3, 2), (3, 4, 2), (0, 0, 3)], [(7, 2, 2), (7, 4, 2), (2, 1, 6), (0, 0, 3)], [(0, 1, 2), (0, 0, 2), (1, 0, 2), (2, 0, 2)]]

  db6 = lmdb.open('algo1_5_3.db', map_size=8 * 1024 * 1024 * 1024)
  with db6.begin(write=True) as txn:
    save_lmdb(txn, p1, p2, buf)
  print("saved")

  # from itertools import product
  # with db6.begin(write=True) as txn:
  #   for x1, x2, y1, y2 in product(range(10), range(10), range(10), range(10)):
  #     p1, p2 = (x1, y1), (x2, y2)
  #     if ((x1 + x2 + y1 + y2) % 2) == 0 or p1 == p2:
  #       print(x1, y1, x2, y2)
  #       txn.put(encodeKey((x1, y1), (x2, y2)), b'')


