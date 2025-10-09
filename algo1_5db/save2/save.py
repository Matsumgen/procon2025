from lib import *
from tqdm import tqdm
import lmdb
import struct
import os
FIELD_SIZE = 18
X_SIZE = FIELD_SIZE * 2 - 2
SAVE_COUNT = 20000
# SAVE_COUNT = 100

def dkey(depth):
  return bytes([(depth << 4), 0, 0, 0, 0, 0, 0])

db = lmdb.open(f'algo1_5_2_{FIELD_SIZE}.db', map_size=16 * 1024 * 1024 * 1024)

with db.begin(write=True) as txn:
  txn.put(encodeKey(createfield(FIELD_SIZE), FIELD_SIZE - 2, FIELD_SIZE, 0), b'')

depth = 0
length = 1
save_count = 0
buffer_path = "buffer.bt"
try:
  while depth < 4:
    start_key = dkey(depth)
    depth += 1
    end_key = dkey(depth)
    # print()
    print(f"depth: {depth}, length: {length}")
    # print([f'{byte:08b}' for byte in start_key], [f'{byte:08b}' for byte in end_key])
    with db.begin() as txnr:
      with txnr.cursor() as cursor:
        if cursor.set_range(start_key):
          save_count = 0
          buf = dict()
          for fkey, _ in tqdm(cursor, total=length, desc='Field_loop'):
            # print("fkey:", fkey, _)
            # print()
            # print(''.join(f'{byte:08b}' for byte in fkey))
            if fkey >= end_key:
              break

            _, X1, X2, field = decodeKey(fkey, FIELD_SIZE)
            for y in range(X_SIZE - 1):
              for x in range(FIELD_SIZE - 2 if y < 2 else 0, X_SIZE - 1):
                for n in range(2, FIELD_SIZE):
                  if y + n > FIELD_SIZE:
                    break

                  x1 = x if x < X1 else X1
                  a = n + x
                  x2 = a if a > X2 else X2

                  if x2 - x1 > FIELD_SIZE:
                    break

                  f = field.copy()
                  rotate(f, x, y, n, True)
                  if checkField(f, FIELD_SIZE) and np.any(field != f):
                    key = encodeKey(f, x1, x2, 0, ba=True)
                    # print("key: ", key)
                    flag = False
                    for k in range(1, depth+1):
                      key[0]= (key[0] & 0xf) | (k << 4)
                      if txnr.get(key) is not None:
                        flag = True
                        break

                    if flag:
                      continue

                    k = bytes(key)
                    v = encodeOperate(x - x1, y, n)
                    if k in buf:
                      buf[k] = buf[k] + v
                    else:
                      buf[k] = v

            if save_count > SAVE_COUNT:
              with open(buffer_path, 'ab') as f:
                for key, value in buf.items():
                  f.write(key)
                  f.write(struct.pack('>I', len(value)))
                  f.write(value)

              buf.clear()
              save_count = 0
            save_count += 1


          length = len(buf)
          with db.begin(write=True) as txnw:
            for k, v in buf.items():
              txnw.put(k, v)
            buf.clear()
            if depth == 1:
              txnw.put(encodeKey(createfield(FIELD_SIZE, other=True), FIELD_SIZE - 4, FIELD_SIZE, 1), encodeOperate(0, 0, 4))

            if os.path.exists(buffer_path):
              with open(buffer_path, 'rb') as f:
                while True:
                  key = f.read(7)
                  if not key:
                    break

                  value_len = struct.unpack('>I', f.read(4))[0]
                  value = f.read(value_len)
                  now_value = txnw.get(key)
                  if now_value is None:
                    txnw.put(key, value)
                    length += 1
                  else:
                    txnw.put(key, now_value + value)

          if os.path.exists(buffer_path):
            os.remove(buffer_path)
        else:
          print("候補が存在しない")
except KeyboardInterrupt:
  print("depth: ", depth)




