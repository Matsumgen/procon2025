from lib import *
from itertools import product
from tqdm import tqdm


# x1以上, x2未満
# rotateのx, nの情報からその盤面が x1~x2の範囲で完結することを情報としてもつ
# x1~x2の範囲は短いが、yは大きくなる可能性
#   -> 使用時にy + nがfsizeを超えないか確認
# DB:(n, x1, x2, 1, 2, 3, 4) -> (x, y, n) : 4 + 6 + 6 + 5 * 2 * 4, 16 -> 7byte, 2byte

# fsize = 24
#depth:  4(2%)
# key lenght: 140478428
# byte size:  937.79 MB
# total key lenght: 13238920
# total byte size:  88.38 MB

# fsize = 18
# depth:  4
# key lenght: 5426695
# byte size:  36.23 MB
# total key lenght: 2706102
# total byte size:  18.07 MB

def format_bytes(size: int) -> str:
  # SI接頭辞に基づく単位リスト
  units = ['B', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB']
  for unit in units:
    if size < 1024:
      return f"{size:.2f} {unit}"
    size /= 1024
  return f"{size:.2f} ZB"  # 超巨大データ向け


key_byte = len(encodeKey(createfield(), FIELD_SIZE - 2, FIELD_SIZE, 0))
byte_size = key_byte
key_length = 1

keys = set()
keys_buf = set()
depth = 1

keys.add(encodeKey(createfield(), FIELD_SIZE - 2, FIELD_SIZE, 0))
try:
  while True:
    for fkey in tqdm(keys, desc="Field  loop"):
      # print(''.join(f'{byte:08b}' for byte in fkey))
      _, X1, X2, field = decodeKey(fkey)
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
            if checkField(f):
              keys_buf.add(encodeKey(f, x1, x2, depth))

    key_length += len(keys_buf)
    byte_size += key_byte * len(keys_buf)

    print("depth: ", depth)
    print("key lenght:", len(keys_buf))
    print("byte size: ", format_bytes(key_byte * len(keys_buf)))
    print("total key lenght:", key_length)
    print("total byte size: ", format_bytes(byte_size))
    print()

    depth += 1
    keys = keys_buf
    keys_buf = set()

    if depth == 2:
      keys.add(encodeKey(createfield(other=True), FIELD_SIZE - 4, FIELD_SIZE, 1))

except KeyboardInterrupt:
  print("depth: ", depth)
  print("key lenght:", len(keys_buf))
  print("byte size: ", format_bytes(key_byte * len(keys_buf)))
  print("total key lenght:", key_length)
  print("total byte size: ", format_bytes(byte_size))
  print()
  

