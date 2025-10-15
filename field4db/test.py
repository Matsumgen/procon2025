
from field4lib import *
import numpy as np
import lmdb
import random

db = lmdb.open('field4.db', map_size=int(1e8))

#現在登録済みの盤面一覧
def func1():
  count = 0
  step = { i: 0 for i in range(8) }
  with db.begin() as txn:
    with txn.cursor() as cursor:
      for key, value in cursor:
        if value != b'No':
          field = decodeField(key)
          field = np.array(field).reshape((4, 4))
          param, num = decodeAnswer(value)
          if num is not None:
            step[num] += 1
          count += 1
        else:
          print(key, value)
          field = decodeField(key)
          field = np.array(field).reshape((4, 4))
          print(field)
  print("total:", count)
  print("step", step)

#完成している盤面数
def func2():
  count = 0
  with db.begin() as txn:
    with txn.cursor() as cursor:
      for key, value in cursor:
        try:
          field = decodeField(key)
          field = np.array(field).reshape((4, 4))
          if isEnd(field):
            count += 1
            if value == b'No':
              print(field)
              print()
        except Exception as e:
          print(e)
          print(key, value)
          # break
  print("total:", count)

#盤面の数
def func3():
  count = 0
  with db.begin() as txn:
    with txn.cursor() as cursor:
      for key, value in cursor:
        count += 1
  print("total:", count)

#ランダムな盤面を解いてみる
def func4():
  steps = []
  for _ in range(10000):
    fieldl = [ i for i in range(8) for _ in range(2) ]
    random.shuffle(fieldl)
    field = np.array(fieldl).reshape((4,4))

    answer = resolv4field(db, field.copy())
    # if isEnd(field):
    #   print(field)
    #   print(answer)
    steps.append(len(answer))
    for a in answer:
      rotate(field, a[0], a[1], a[2])
      # print(f"x: {a[0]}, y: {a[1]}, n: {a[2]}\n")
      # print(field)
      # print(a[0], a[1], a[2])
    if not isEnd(field):
      print(fieldl)
      raise Exception("Didn't resolv problem")

  steps = np.array(steps)
  print(f"mean:{np.mean(steps)}, min:{np.min(steps)}, max:{np.max(steps)}, median:{np.median(steps)}, std:{np.std(steps)}")


# バグチェック
# key = b'\xb2nKr\xc0'
# print("key:", key)
# with db.begin() as txn:
#   value = txn.get(key)
#   print(key, value)
#   field = decodeField(key)
#   print(field)





func4()


