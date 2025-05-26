import numpy as np

# 数字の振り直し
def reallocation(field):
  d = {}
  if type(field) is list:
    for i, ent in enumerate(field):
      if d.get(ent) is None:
        d[ent] = len(d)
      field[i] = d[ent]
  elif type(field) is np.ndarray:
    for y in range(field.shape[0]):
      for x in range(field.shape[1]):
        ent = field[y, x]
        if d.get(ent) is None:
          d[ent] = len(d)
        field[y, x] = d[ent]

# Fieldを固定長バイト列にエンコード
# 118bit(固定長) k:20, s:28
# 12233444 55566677 78888999 9aaaabbb bccccddd 
# deeeefff fggggghh hhhiiiii jjjjjkkk kllllmmm
# mnnnnooo oppppqqq qrrrrsss tttuuuvv vwwxxy
def encodeFixed(f):
  d = { k: 0 for k in range(18) }
  ret = b''
  # f[upK]のインデックスを取得 & d更新
  def getDIndex(upK:int):
    i = 0
    for dk in sorted(d.keys()):
      if d[dk] > 1:
        d.pop(dk)
      else:
        if dk == f[upK]:
          d[dk] += 1
          return i
        else:
          i += 1

  def retAdd(r:int):
    ret += r.to_bytes()

  for e in f[:20]:
    d[e] += 1
  retAdd(f[1] << 7 | f[2] << 5 | f[3] << 3 | f[4])
  retAdd(f[5] << 5 | f[6] << 2 | f[7] >> 1)
  for i in range(4):
    retAdd((f[7+i*2] & 1) << 7 | f[8+i*2] << 3 | f[9+i*2] >> 1)

  retAdd((f[15] & 1) << 7 | f[16] << 2 | f[17] >> 3)
  retAdd(f[17] & 7 << 5 | f[18])

  di = getDIndex(20)
  retAdd(f[19] << 3 | di >> 1)
  b = (di & 1) << 7

  b |= getDIndex(21) << 3

  for i in range(3):
    di = getDIndex(22 + i)
    retAdd(b | di >> 1)
    b = (di & 1) << 6
    b |= getDIndex(23 + i) << 3

  di = getDIndex(28)
  retAdd(b | di)

  b = getDIndex(29) << 5
  b |= getDIndex(30) << 2
  bi = getDIndex(31)
  retAdd(b | bi >> 1)
  b = (bi & 1) << 7
  b |= getDIndex(32) << 5
  b |= getDIndex(33) << 3
  retAdd(b | getDIndex(34) << 2)

  return ret




# Fieldをバイト列にエンコード
# fixed: 固定長でエンコード
def encodeField(f, fixed=False):
  if type(f) is np.ndarray:
    f = f.flatten().tolist()
  reallocation(f)

  if fixed:
    return encodeFieldFixed(f)

  d = { k: 0 for k in range(18) }
  def getDIndex(upK:int):
    di = {}
    for dk in sorted(d.keys()):
      if d[dk] > 1:
        d.pop(dk)
      else:
        di[dk] = len(di)
    d[f[upK]] += 1
    return di, d[f[upK]] == 2

  ret = b''
  d[0] += 1
  num_p = 1 # 出現しうる数字の最大値
  shift_count = 0
  b = 0
  for e in range(1, 35, 1):
    di, flag = getDIndex(e) # di[f[e]]: index
    # print(d)
    # print(di)
    # print("flag:", flag, ", num_p:", num_p, ", shift_count:", shift_count)
    if num_p != 0:
      sc = num_p.bit_length()
      # print("sc:", sc, ", di[f[e]]:", bin(di[f[e]])[2:], ", b:", bin(b)[2:])
      b <<= sc
      b |= di[f[e]]
      shift_count += sc
      if shift_count > 8:
        shift_count -= 8
        # print("shift: shift_count:", shift_count)
        # print("ret:", ' '.join(format(bb, '08b') for bb in ret))
        # print("b      :", bin(b)[2:])
        # print("add ret:", bin(b >> shift_count)[2:])
        # print("and    : 00000000", bin(2 ** (shift_count) - 1)[2:], sep='')
        ret += (b >> shift_count).to_bytes()
        b &= 2**(shift_count) - 1
        # print("after b:", bin(b)[2:])

    num_p += -1 if flag else 1
    num_p = min(num_p, di[max(di.keys())])
    # print()

  if shift_count != 0:
    ret += (b << (8 - shift_count)).to_bytes()

  return ret

def decodeFieldFixed(bfi:bytes):
  ret = [ -1 for _ in range(36) ]
  ret[0] = 0
  ret[1]  = bfi[0] >> 7
  ret[2]  = (bfi[0] >> 5) & 0b11
  ret[3]  = (bfi[0] >> 3) & 0b11
  ret[4]  = bfi[0] & 0b111
  ret[5]  = bfi[1] >> 5
  ret[6]  = (bfi[1] >> 2) & 0b111
  ret[7]  = (bfi[1] & 0b11) << 1
  for i in range(4):
    ret[7+i*2] +=  bfi[2+i] >> 7
    ret[8+i*2]  = (bfi[2+i] >> 3) & 0b1111
    ret[9+i*2]  = (bfi[2+i] & 0b111) << 1

  ret[15] += bfi[6] >> 7
  ret[16]  = (bfi[6] >> 2) & 0b11111
  ret[17]  = (bfi[6] & 0b11) << 3
  ret[17] += bfi[7] >> 5
  ret[18]  = bfi[7] & 0b11111
  ret[19]  = bfi[8] >> 3
  ret[20]  = (bfi[8] & 0b111) << 1

  for i in range(4):
    ret[20+i*2] +=  bfi[9+i] >> 7
    ret[21+i*2]  = (bfi[9+i] >> 3) & 0b1111
    ret[22+i*2]  = (bfi[9+i] & 0b111) << 1
  ret[28] >> 1
  ret[29]  = bfi[13] >> 5
  ret[30]  = (bfi[13] >> 2) & 0b111
  ret[31]  = (bfi[13] & 2) << 1
  ret[31] += bfi[14] >> 7
  ret[32]  = (bfi[14] >> 5) & 0b11
  ret[33]  = (bfi[14] >> 3) & 0b11
  ret[34]  = (bfi[14] >> 2) & 0b1

  # ret[20] 以降について



def decodeField(bfi:bytes, fixed=False):
  if fixed:
    return decodeFieldFixed(bfi)

  bis = ''.join(format(byte, '08b') for byte in bfi)
  def get(n):
    nonlocal bis
    ret = bis[:n]
    bis = bis[n:]
    if not ret:
      return 0
    return int(ret, 2)

  d = { k: 0 for k in range(18) }
  def getDNum(val:int):
    di = []
    for dk in sorted(d.keys()):
      if d[dk] > 1:
        d.pop(dk)
      else:
        di.append(dk)
    d[di[val]] += 1
    return di, d[di[val]] == 2



  ret = [ -1 for _ in range(36) ]
  ret[0] = 0
  d[0] = 1
  num_p = 1
  for i in range(1, 36, 1):
    # print("i:", i, ", num_p", num_p)
    val = get(num_p.bit_length())
    # print("val:", val)
    di, flag = getDNum(val)
    # print("d:", d)
    # print("di:", di)
    # print("flag:", flag)
    ret[i] = di[val]
    num_p += -1 if flag else 1
    num_p = min(num_p, len(di) - 1)
  ret = np.array(ret).reshape((6,6))
  return ret



def encodeOperate(ope:list):
  # x: 0~4, y:0~4, n:2~5(0~3)
  # xxxyyynn
  if 0 <= ope[0] < 5 and 0 <= ope[1] < 5 and 2 <= ope[2] < 6:
    n = ope[2] - 2
    return ((ope[0] << 5) | (ope[1] << 2) | n).to_bytes()
  raise ValueError(f"encodeOperate: x={ope[0]}, y={ope[1]}, n={ope[2]}")

def decodeOperate(bop:bytes):
  return [ bop[0] >> 5, (bop[0] >> 2) & 0b111, bop[0] & 0b11 ]

def rotate(field:np.ndarray, x:int, y:int, n:int, rev:bool=False):
  # np.ndarryのfieldを90度回転
  # rev=Trueで左回転
  sub = field[y:y+n, x:x+n]
  k = 1 if rev else -1
  rotated = np.rot90(sub, k=k)
  field[y:y+n, x:x+n] = rotated



def isEnd(field):
  # 終了条件確認
  for k in range(18):
    positions = np.argwhere(field == k)  # [[y1, x1], [y2, x2]]
    if positions.shape != (2, 2):
      return False

    (y1, x1), (y2, x2) = positions
    if not ((abs(y1 - y2) == 1 and x1 == x2) or (abs(x1 - x2) == 1 and y1 == y2)):
      return False
  return True

def isProblem(field):
  # 問題の形式通りか確認
  if field.shape != (6, 6):
    return False

  unique, counts = np.unique(field, return_counts=True)
  if not (len(unique) == 18 and all(counts == 2)):
    return False
  return True


# field4に縮小。できなければNone
def toField4(field):
  ret = []
  for y1, y2 in ((0, 4), (2, 6)):
    for x1, x2 in ((0, 4), (2, 6)):
      if isProblem(field[y1:y2, x1:x2]):
        f = field.copy()
        f[y1:y2, x1:x2] = -1
        for k in range(18):
          pos = np.argwhere(field == k):
          if pos.size == 0:
            continue

          (y1, x1), (y2, x2) = pos
          if not ((abs(y1 - y2) == 1 and x1 == x2) or (abs(x1 - x2) == 1 and y1 == y2)):
            return None
        ret.append(field[y1:y2, x1:x2])
  return ret





# DBへ登録するkeyとvalueを返す
# 全体を回転させた盤面がDBに登録されているなら登録しない
# 全体を回転させた時、最もバイト数が少ないやつを登録
def putDB(txn, field, ope):
  f = field.copy()
  encf = encodeField(f)
  if txn.get(encf) is not None:
    return None, None

  putf = (encf, ope)
  opec = ope.copy()
  min_len = len(encf)
  for _ in range(3):
    rotate(f, 0, 0, 6)
    encf = encodeField(f)
    if txn.get(encf) is not None:
      return None, None

    opec = [ 6 - opec[1] - opec[2], opec[0], opec[2]  ]
    if len(encf) < min_len:
      putf = (encf, opec)

  enco = encodeOperate(putf[1])
  encf = putf[0]
  return encf, enco

# DBから取得
def getDB(txn, field):
  f = field.copy()
  encf = encodeField(f)
  enco = txn.get(encf)
  if enco is not None:
    return decodeOperate(enco)

  for i in range(1, 4, 1):
    rotate(f, 0, 0, 6)
    encf = encodeField(f)
    enco = txn.get(encf)
    if enco is not None:
      ope = decodeOperate(enco)
      for _ in range(i):
        b = ope[0]
        ope[0] = ope[1]
        ope[1] = 6 - b - ope[2]
      return ope





  



