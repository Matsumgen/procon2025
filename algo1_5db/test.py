from lib import *
import lmdb

# db = lmdb.open('algo1_5_2_16.db', map_size=16 * 1024 * 1024 * 1024)
db = lmdb.open('save2/algo1_5_2_8.db', map_size=16 * 1024 * 1024 * 1024)

def test1():
  f = createfield()
  rotate(f, FIELD_SIZE - 1, 1, 5, rev=True)
  rotate(f, FIELD_SIZE - 2, 1, 7, rev=True)
  rotate(f, FIELD_SIZE - 1, 0, 5, rev=True)
  x1 = 9
  x2 = 23
  fsize = x2 - x1
  field = f[:fsize, x1:x2]
  printfield(field)
  print("shape:", field.shape)
  start_key = bytearray([ x1 >> 2, (x1 & 0xF) << 2 | x2, 0, 0, 0, 0, 0 ])
  depth = 1
  while not isEnd(field):
    flag = False
    with db.begin() as txn:
      with txn.cursor() as cursor:
        for dp in range(5):
          start_key[0] = (start_key[0] & 0xF) | (dp << 4)
          print("start_key:", start_key)
          if cursor.set_range(start_key):
            for key, value in cursor:
              d, _x1, _x2, fi = decodeKey(key)
              if _x2 > x2 or d != dp:
                break
              p = []
              for i in range(4):
                cod = np.where(fi == i)
                if cod[0][0] >= fsize:
                  flag = True
                  break
                p.append((cod[1][0] - x1, cod[0][0]))

              if flag:
                flag = False
                continue

              if field[p[0][1], p[0][0]] == 4 or field[p[1][1], p[1][0]] == 4 or field[p[2][1], p[2][0]] == 4 or field[p[3][1], p[3][0]] == 4:
                continue

              if field[p[0][1], p[0][0]] == fi[p[0][1], p[0][0] + x1] and field[p[1][1], p[1][0]] == fi[p[1][1], p[1][0] + x1] and field[p[2][1], p[2][0]] == fi[p[2][1], p[2][0] + x1] and field[p[3][1], p[3][0]] == fi[p[3][1], p[3][0] + x1]:
              # if field[p[0][1], p[0][0]] == field[p[1][1], p[1][0]] and field[p[2][1], p[2][0]] == field[p[3][1], p[3][0]] or field[p[0][1], p[0][0]] == field[p[2][1], p[2][0]] and field[p[1][1], p[1][0]] == field[p[3][1], p[3][0]]:
                ope = decodeOperate(value)
                if ope[1] >= fsize:
                  continue
                print(ope)
                rotate(field, ope[0] - x1, ope[1], ope[2])
                printfield(field)
                flag = True
                break

          if flag:
            break;

def printf(f):
  for y in range(f.shape[0]):
    for x in range(f.shape[1]):
      print(f"{f[y, x]:>3}", end=' ')
    print()

def checkBox(f, p1, p2, p3, p4):
  return f[p1[1], p1[0]] == f[p2[1], p2[0]] and f[p3[1], p3[0]] == f[p4[1], p4[0]] \
      or f[p1[1], p1[0]] == f[p3[1], p3[0]] and f[p2[1], p2[0]] == f[p4[1], p4[0]]

def test2():
  field = randomField(FIELD_SIZE)
  fsize = field.shape[0]
  x1 = FIELD_SIZE - 2
  x2 = FIELD_SIZE * 2 - 2
  start_key = bytearray([ x1 >> 2, (x1 & 0x3) << 6 , 0, 0, 0, 0, 0 ])
  t_x, t_y, t_p, mode = 0, 0, 0, 0
  count = 0
  print("t_x, t_y, t_p, mode =", t_x, t_y, t_p, mode)
  while not isEnd(field):
    with db.begin() as txn:
      with txn.cursor() as cursor:
        flag = False
        start_key = bytearray([ x1 >> 2, (x1 & 0x3) << 6 , 0, 0, 0, 0, 0 ])
        for dp in range(6):
          print("depth: ", dp)
          start_key[0] = (start_key[0] & 0xF) | (dp << 4)
          # print("start_key:", start_key)
          if cursor.set_range(start_key):
            for key, value in cursor:
              d, _x1, _x2, p1, p2, p3, p4 = parseKey(key)
              if _x2 > x2 or d != dp:
                break
              for p in (p1, p2, p3, p4):
                if p[1] >= fsize:
                  flag = True
                  break
                p[0] -= x1

                if mode == 1:
                  _p = rotateP(p, fsize)
                  p[0] = _p[0]
                  p[1] = _p[1] - t_p

              if flag:
                flag = False
                continue

              if checkBox(field, p1, p2, p3, p4):
                ope = list(decodeOperate(value))
                if ope[1] >= fsize:
                  continue

                ope[0] -= x1
                if mode == 1:
                  ope = rotateR(ope, fsize)
                  ope[1] - t_p
                print(ope)
                rotate(field, ope[0], ope[1], ope[2])
                flag = True
                count += 1
                break
            if flag:
              break;

        if checkBox(field, (t_x, t_y), (t_x + 1, t_y), (t_x, t_y + 1), (t_x + 1, t_y + 1)):
          if mode == 0:
            if t_x >= FIELD_SIZE - 2: # 上2行が終わった時
              x1, x2, mode = FIELD_SIZE - 2, FIELD_SIZE * 2 - 4 - t_p, 1
              t_y += 2
              t_x += 1
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
          print("x1, x2, t_x, t_y, t_p, mode =", x1, x2, t_x, t_y, t_p, mode)
          count = 0
          printf(field)

def test3():
  with db.begin() as txn:
    with txn.cursor() as cursor:
      for key, value in cursor:
        if value == b'':  continue
        # print(len(key), len(value), key, value)
        print(parseKey(key), decodeOperate(value, True))
        # print()


test3()
