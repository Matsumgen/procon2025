from lib import *
import lmdb

db = lmdb.open('algo1_5_2_16.db', map_size=16 * 1024 * 1024 * 1024)
FIELD_SIZE = 16

field = randomField(FIELD_SIZE)
start_field = field.copy()
fsize = field.shape[0]
x1 = FIELD_SIZE - 2
x2 = FIELD_SIZE * 2 - 2
start_key = bytearray([ x1 >> 2, (x1 & 0x3) << 6 , 0, 0, 0, 0, 0 ])
t_x, t_y, t_p, mode = 0, 0, 0, 0
count = 0

opes = list()

print("t_x, t_y, t_p, mode =", t_x, t_y, t_p, mode)
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
          if p[1] >= fsize:
            flag = True
            break
          p[0] -= x1
          p[1] += t_p

          if mode == 1:
            _p = rotateP(p, FIELD_SIZE)
            # print(p, end=" ")
            p[0] = _p[0]
            p[1] = _p[1] + t_p + 2
            # print(p)

        if flag:
          flag = False
          continue
        
        if checkBox(field, p1, p2, p3, p4):
          # print(parseKey(key))
          ope = list(decodeOperate(value))

          ope[0] -= x1
          ope[1] += t_p
          if mode == 1:
            ope = list(rotateR(ope, FIELD_SIZE))
            ope[1] += t_p + 2

          if ope[0] < 0 or ope[1] + ope[2] > field.shape[1]:
            continue

          print("depth:", d, ope)
          rotate(field, ope[0], ope[1], ope[2])
          opes.append((d, ope))
          flag = True
          count += 1
          break

  if not flag:
    print("not found error")
    break

  if checkBox(field, (t_x, t_y), (t_x + 1, t_y), (t_x, t_y + 1), (t_x + 1, t_y + 1)):
    if mode == 0:
      if t_x >= fsize - 2: # 上2行が終わった時
        x1, x2, mode = FIELD_SIZE - 2, FIELD_SIZE * 2 - 4 - t_p, 1
        t_y += 2
      else:
        t_x += 2
        x1 -= 2
        x2 -= 2
    elif mode == 1:
      if t_y >= FIELD_SIZE - 2: # 右2列が終わった時
        t_p += 2
        t_x, t_y = 0, t_p
        x1, x2, mode = FIELD_SIZE - 2, FIELD_SIZE * 2 - 2 - t_p, 0
        fsize -= 2
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
for ope in opes:
  o = ope[1]
  print(o[0], o[1], o[2])
  ddict[ope[0]] += 1

print()
print(ddict)


