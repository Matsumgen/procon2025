from lib import *
from tqdm import tqdm
import lmdb

FIELD_SIZE = 24
X_SIZE = FIELD_SIZE * 2 - 2
db = lmdb.open(f'algo1_5_2_{FIELD_SIZE}_hor.db', map_size=16 * 1024 * 1024 * 1024)

def dkey(hor, depth):
  index = setIndex(hor, depth, 0, 0)
  return bytes([index >> 8, index & 0xFF, 0, 0, 0, 0, 0])

# depth以外が同じ時のレコードを含む
def worker(hor, depth, X1, X2, field, buf):
  # print("worker", hor, depth, X1, X2)
  for y in range(X_SIZE - 1):
    for x in range(FIELD_SIZE - 2 if y < 2 else 0, X_SIZE - 1):
      for n in range(2, FIELD_SIZE):
        if y + n > FIELD_SIZE:
          break
  
        if x < X1:
          x1 = x
          x2 += X1 - x
        else:
          x1 = X1

        a = n + x
        x2 = a - X1 if a > X1 + X2 else X2

        if x2 > FIELD_SIZE - 2:
          break

        f = field.copy()
        rotate(f, x, y, n, True)
        if checkField(f, FIELD_SIZE, hor):
          c = field == f
          if np.all(c):
            continue

          flag = True
          for yy, xx in zip(*np.where(~c)):
            if f[yy, xx] == 4 or field[yy, xx] == 4:
              flag = False
              break

          if flag:
            continue

          # print(hor, depth, x1, x2)
          # printfield(f)
          k = encodeKey(f, hor, depth, x1, x2)
          decodeKey(k, FIELD_SIZE)
          if k not in buf:
            buf[k] = set()
          # print(x, y, n)
          # printfield(f)
          buf[k].add((x - x1, y, n))

if __name__ == "__main__":
  length = 1

# (x, y, n)のsetを保存
  buf = dict()

  try:
    for hor in range(1, 21):
      start_field = createfield(FIELD_SIZE, hor)

      buf.clear()
      worker(hor, 1, 20, 4, start_field, buf)
      length = len(buf)
      with db.begin(write=True) as txnw:
        for k, v in buf.items():
          txnw.put(k, encodeOperateL(v))
          # print((k[0] << 8) + k[1])
        if hor >= 2:
          txnw.put(encodeKey(createfield(FIELD_SIZE, hor, other=True), hor, 1, FIELD_SIZE - 6, 6), encodeOperate(0, 0, 4))


      for depth in range(2, 4):
        start_key = dkey(hor, depth - 1)
        end_key = dkey(hor, depth)
        print(f"depth: {depth}, length: {length}")
        print((start_key[0] << 8) + start_key[1], (end_key[0] << 8) + end_key[1])
        with db.begin() as txnr:
          with txnr.cursor() as cursor:
            if cursor.set_range(start_key):
              buf.clear()
              for fkey, _ in tqdm(cursor, total=length, desc='Field_loop'):
                if fkey >= end_key:
                  break
                # print(fkey)
                _hor, _depth, X1, X2, field = decodeKey(fkey, FIELD_SIZE)
                # print((fkey[0] << 8) + fkey[1], _hor, _depth, X1, X2)
                worker(_hor, depth, X1, X2, field, buf)

              with db.begin(write=True) as txnw:
                for k, v in buf.items():
                  # print((k[0] << 8) + k[1], end=" ")
                  txnw.put(k, encodeOperateL(v))
              length = len(buf)
            else:
              print("Not scan")


  except KeyboardInterrupt:
    print("depth: ", depth)
