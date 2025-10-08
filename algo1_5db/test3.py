from lib import *
import lmdb
import random

# 最低でも4手は欲しい
FIELD_SIZE = 16
FSIZE = 16
# db_file_name = f'save2/algo1_5_2_{FIELD_SIZE}.db'
db_file_name = f'algo1_5_2_{FIELD_SIZE}.db'
db = lmdb.open(db_file_name, map_size=16 * 1024 * 1024 * 1024)

fsize = FSIZE

field = randomField(fsize)
start_field = field.copy()
x1 = FIELD_SIZE - 2
x2 = FIELD_SIZE + fsize - 2
start_key = bytearray([ x1 >> 2, (x1 & 0x3) << 6 , 0, 0, 0, 0, 0 ])
t_x, t_y, t_p, mode = 0, 0, 0, 0
count = 0

result_opes = list()

printf(field)
print("fsize, x1, x2, t_x, t_y, t_p, mode =", fsize, x1, x2, t_x, t_y, t_p, mode)
while not isEnd(field):
  with db.begin() as txn:
    with txn.cursor() as cursor:
      flag = False
      for key, value in cursor:
        if value == b'':
          continue
        d, _x1, _x2, p1, p2, p3, p4 = parseKey(key)

        if _x1 < x1  or x2 < _x2:
          continue

        for p in (p1, p2, p3, p4):
          p[0] -= x1
          p[1] += t_p

          if p[1] >= FSIZE:
            flag = True
            break

          if mode == 1:
            _p = rotateP(p, FSIZE)
            # print(p, end=" ")
            p[0] = _p[0]
            p[1] = _p[1] + t_p + 2
            # print(p)


        if flag:
          flag = False
          continue

        # print(d, _x1, _x2, p1, p2, p3, p4)
        if checkBox(field, p1, p2, p3, p4):
          # print(d, _x1, _x2, p1, p2, p3, p4)
          # print(parseKey(key))
          _opes = decodeOperate(value, True)
          # print("\t", _opes)
          
          opes = list()
          for ope in _opes:
            # ope[0] = ope[0] + _x1 - x1
            ope[0] = ope[0]  - x1 # FIELD_SIZE == 16の時に使用
            ope[1] += t_p

            if mode == 1:
              _ope = list(rotateR(ope, FSIZE))
              ope[0] = _ope[0]
              ope[1] = _ope[1] + t_p + 2

            if ope[0] < 0 or ope[1] + ope[2] > FSIZE or ope[0] + ope[2] > fsize:
              continue
            opes.append(ope)
          if not opes:
            continue
          print("log:", d, _x1, _x2, field[p1[1], p1[0]], field[p2[1], p2[0]], field[p3[1], p3[0]], field[p4[1], p4[0]], "->", _opes)

          ope = random.choice(opes) # とりあえずランダムに選ぶ

          # print("depth:", d, ope)
          rotate(field, ope[0], ope[1], ope[2])
          result_opes.append((d, ope))
          flag = True
          count += 1
          break

  if not flag:
    print("not found error")
    break

  if checkBox(field, (t_x, t_y), (t_x + 1, t_y), (t_x, t_y + 1), (t_x + 1, t_y + 1)):
    if mode == 0:
      if t_x >= fsize - 2: # 上2行が終わった時
        x1, x2, mode = FIELD_SIZE - 2, FIELD_SIZE + fsize - 4, 1
        t_y += 2
      else:
        t_x += 2
        x1 -= 2
        x2 -= 2
    elif mode == 1:
      if t_y >= FSIZE - 2: # 右2列が終わった時
        fsize -= 2
        t_p += 2
        t_x, t_y = 0, t_p
        x1, x2, mode = FIELD_SIZE - 2, FIELD_SIZE + fsize - 2, 0
      else:
        t_y += 2
        x1 -= 2
        x2 -= 2
    print(f"clear box {count}")
    print("fsize, x1, x2, t_x, t_y, t_p, mode =", fsize, x1, x2, t_x, t_y, t_p, mode)
    count = 0
    printf(field)

  if count > 6:
    print("error")
    break

print("result")
printf(start_field)
ddict = dict()
for i in range(6):
  ddict[i] = 0
for ope in result_opes:
  o = ope[1]
  print(o[0], o[1], o[2])
  ddict[ope[0]] += 1

print()
print(ddict)


