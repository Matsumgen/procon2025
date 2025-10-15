import csv
import subprocess
import tempfile
import os
import random
import numpy as np
import sys
import shutil


size = int(sys.argv[1]) if len(sys.argv) > 1 else 4
n = int(sys.argv[2]) if len(sys.argv) > 2 else 100  # 繰り返す回数
results = []
save_csv_data = "";

def createProblem(size):
  if(size < 4 or 24 < size or size%2 == 1):
    raise ValueError(f"Invalid args: size={size}")
  entities = list(range(int(size*size/2)))
  entities.extend(range(int(size*size/2)))
  random.shuffle(entities)
  
  entities = [entities[i * size:(i + 1) * size] for i in range(size)]
  return entities


for i in range(n):
  entities = createProblem(size)
  # 一時CSVファイルを作成
  with tempfile.NamedTemporaryFile(mode='w', suffix='.csv', delete=False, newline='') as temp_csv:
    writer = csv.writer(temp_csv)
    for e in entities:
      writer.writerow(e)
      
    temp_csv_path = temp_csv.name  # ファイルパスを保存

  # コマンドとして実行（例：your_program.py に渡す）
  try:
    ret = subprocess.run(['./exe', temp_csv_path], capture_output=True, text=True, check=True)
    res = [-1,-1,-1]
    ret_str = ret.stdout.splitlines()
    for j in range(3):
      res[j] = int(ret_str[j+1].split(' ')[-1])
    result_str = f"[{i+1}/{n}]\tsize: {res[0]}\ttime: {res[1]}\tstep: {res[2]}"
    results.append(res)
  except Exception as e:
    result_str = f"[{i+1}/{n}] ERROR"
    print(e)
    shutil.copy(temp_csv_path, f"./error_{int(random.random() * 1000)}.csv")
  finally:
    # 実行後に一時ファイルを削除
    print(result_str)
    save_csv_data += result_str
    os.remove(temp_csv_path)

data = np.array(results)
s = [np.mean(data, axis=0),np.max(data, axis=0),np.min(data, axis=0),np.median(data, axis=0),np.std(data, axis=0)]
result_str = f"size: {results[0][0]}\ntime: mean:{s[0][1]}\tmax:{s[1][1]}\tmin:{s[2][1]}\tmedian:{s[3][1]}\tstd:{s[4][[1]]}\nstep: mean:{s[0][2]}\tmax:{s[1][2]}\tmin:{s[2][2]}\tmedian:{s[3][2]}\tstd:{s[4][[2]]}"
print(result_str)
save_csv_data += result_str

with open(f"result_{size}_{int(random.random() * 1000)}.txt", "w", encoding="utf-8") as file:
    file.write(save_csv_data)
