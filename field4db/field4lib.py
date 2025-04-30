import numpy as np

def reallocation(field):
  # 番号の振りなおしを行う
  d = {}
  if type(field) is list:
    for i, ent in enumerate(field):
      if d.get(ent) is None:
        d[ent] = len(d)
      field[i] = d[ent]
  elif type(field) is np.ndarray:
    for y in range(4):
      for x in range(4):
        ent = field[y, x]
        if d.get(ent) is None:
          d[ent] = len(d)
        field[y, x] = d[ent]


def generate_valid_numbers():
  # field[n] = 0 ~ min(n, 7) という条件で全ての組み合わせを出力するイテレータ
  from collections import Counter

  result = []
  counts = [0] * 8

  def backtrack(pos):
    if pos == 16:
      yield result[:]
      return

    max_digit = min(pos, 7)
    for n in range(max_digit + 1):
      if counts[n] < 2:
        result.append(n)
        counts[n] += 1
        if sum(2 - c for c in counts if c < 2) <= 16 - pos - 1:
          yield from backtrack(pos + 1)

        result.pop()
        counts[n] -= 1

  yield from backtrack(0)

def encodeField(field:list):
  # 盤面を34bitにエンコード
  # 12233444 55566677 7888999a aabbbccd de
  # c~fは残った数の何番目かを入れている
  d = { k: 0 for k in range(8) }
  for e in field[:12]:
    d[e] += 1

  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { k: i for i, k in enumerate(sorted(list(d.keys()))) }

  ret = b''
  bi = field[1] << 7
  bi += field[2] << 5
  bi += field[3] << 3
  bi += field[4]
  ret += bi.to_bytes()
  bi = field[5] << 5
  bi += field[6] << 2
  bi += field[7] >> 1
  ret += bi.to_bytes()
  bi = (field[7] & 0b001) << 7
  bi += field[8] << 4
  bi += field[9] << 1
  bi += field[10] >> 2
  ret += bi.to_bytes()
  bi = (field[10] & 0b011) << 6
  bi += field[11] << 3

  bi += d_index[field[12]] << 1
  d[field[12]] += 1
  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { k: i for i, k in enumerate(sorted(list(d.keys()))) }

  bi += d_index[field[13]] >> 1
  ret += bi.to_bytes()
  bi = (d_index[field[13]] & 0b01) << 7
  d[field[13]] += 1
  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { k: i for i, k in enumerate(sorted(list(d.keys()))) }

  bi += d_index[field[14]] << 6
  ret += bi.to_bytes()
  return ret

def decodeField(bfi:bytes):
  # 34bitデータをlength=16のlistへデコード
  ret = [ 99 for i in range(16) ]
  ret[0]  = 0
  ret[1]  = (bfi[0] & 0b10000000) >> 7
  ret[2]  = (bfi[0] & 0b01100000) >> 5
  ret[3]  = (bfi[0] & 0b00011000) >> 3
  ret[4]  = (bfi[0] & 0b00000111)
  ret[5]  = (bfi[1] & 0b11100000) >> 5
  ret[6]  = (bfi[1] & 0b00011100) >> 2
  ret[7]  = (bfi[1] & 0b00000011) << 1
  ret[7] += (bfi[2] & 0b10000000) >> 7
  ret[8]  = (bfi[2] & 0b01110000) >> 4
  ret[9]  = (bfi[2] & 0b00001110) >> 1
  ret[10] = (bfi[2] & 0b00000001) << 2
  ret[10]+= (bfi[3] & 0b11000000) >> 6
  ret[11] = (bfi[3] & 0b00111000) >> 3


  d = { k: 0 for k in range(8) }
  for e in ret[:12]: d[e] += 1
  d_index = {}
  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { i: k for i, k in enumerate(sorted(list(d.keys()))) }

  i = (bfi[3] & 0b00000110) >> 1
  ret[12] = d_index[i]
  d[d_index[i]] += 1;
  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { i: k for i, k in enumerate(sorted(list(d.keys()))) }

  i = (bfi[3] & 0b00000001) << 1
  i+= (bfi[4] & 0b10000000) >> 7
  ret[13] = d_index[i]
  d[d_index[i]] += 1;
  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { i: k for i, k in enumerate(sorted(list(d.keys()))) }

  i = (bfi[4] & 0b01000000) >> 6
  ret[14] = d_index[i]
  d[d_index[i]] += 1;
  d = { k: v for k, v in d.items() if v <= 1 }
  d_index = { i: k for i, k in enumerate(sorted(list(d.keys()))) }

  ret[15] = d_index[0]
  return ret


def encodeAnswer(answer:list, number:int=None):
  # 回答をエンコード
  # [x, y, n] -> b'00xxyynn'
  ret  = answer[0] << 4
  ret += answer[1] << 2
  ret += answer[2]
  ret = ret.to_bytes()
  if number is not None:
    ret += number.to_bytes()
  return ret

def decodeAnswer(answer:bytes):
  # 回答をデコード
  if answer is None or len(answer) == 0:
    return None, 0
  ret = [-1] * 3
  ret[0] = (answer[0] & 0b00110000) >> 4
  ret[1] = (answer[0] & 0b00001100) >> 2
  ret[2] = (answer[0] & 0b00000011)
  return ret, None if len(answer) < 2 else answer[1]

def rotate(field:np.ndarray, x:int, y:int, n:int, rev:bool=False):
  # np.ndarryのfieldを90度回転
  # rev=Trueで左回転
  sub = field[y:y+n, x:x+n]
  k = 1 if rev else -1
  rotated = np.rot90(sub, k=k)
  field[y:y+n, x:x+n] = rotated

def isProblem(field):
  # 問題の形式通りか確認
  if field.shape != (4, 4):
    return False

  unique, counts = np.unique(field, return_counts=True)
  if not (len(unique) == 8 and all(counts == 2)):
    return False
  return True

def isEnd(field):
  # 終了条件確認
  for k in range(8):
    positions = np.argwhere(field == k)  # [[y1, x1], [y2, x2]]
    if positions.shape != (2, 2):
      return False

    (y1, x1), (y2, x2) = positions
    if not ((abs(y1 - y2) == 1 and x1 == x2) or (abs(x1 - x2) == 1 and y1 == y2)):
      return False
  return True

def creat4field(db):
  # データベースにfieldの取りうる全ての盤面を登録する
  # 終了条件を満たしているときはvalueに0をそれ以外はNoを入れる
  inserted = 0
  for i, lfield in enumerate(generate_valid_numbers()):
    field = np.array(lfield).reshape((4, 4))
    reallocation(lfield)
    dat = encodeField(lfield)
    with db.begin(write=True) as txn:
      existing = txn.get(dat)
      if existing is None:
        txn.put(dat, b'\x00\x00' if isEnd(field) else b'No')
        inserted += 1
    if i % 1000000 == 0:
      print(f"{inserted/2027025*100:.4f}%\t{inserted}/2027025")

def resolv4field(db, field):
  # 問題を解く。
  ret = []
  if type(field) is list:
    field = np.array(field).reshape((4,4))
  reallocation(field)
  while True:
    dat = encodeField(field.flatten().tolist())
    with db.begin() as txn:
      datparm = txn.get(dat)
    if datparm is None or datparm[0] == b'N':
      print(dat, datparm)
      print(field)
      raise Exception("Unknown field")

    if datparm[0] == 0:
      break

    parm, _ = decodeAnswer(datparm)
    rotate(field, parm[0], parm[1], parm[2])
    reallocation(field)
    ret.append(parm)
  return ret


