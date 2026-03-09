import itertools
import subprocess
import os
import time
from datetime import datetime

# =======================================================
# 試したいパラメータのリストをここに定義します
# =======================================================
# =======================================================
# 試したいパラメータのリストをここに定義します
# =======================================================
# =======================================================
# 試したいパラメータのリストをここに定義します
# =======================================================
# =======================================================
# 試したいパラメータのリストをここに定義します
# =======================================================
param_grid = {
    # --- 安全かつ高精度なビーム幅に固定 ---
    "P_BEAM_WIDTH": [262144], 
    
    # --- 最強の評価関数「3」に全振り ---
    "P_EVALUATION": [3],
    
    # --- 優秀だった SLICE 1 と 2 ---
    "P_SLICE": [1, 2],
    
    # --- THRESHOLD を広範囲で細かく検証 ---
    # 18〜20が良かったので、その前後を広く取る
    "P_THRESHOLD": [16, 18, 20, 22, 24],
    
    # --- SN と EN の限界領域を探る ---
    # SN: これまで2〜5を試したが、6や7も入れてみる
    "P_SN": [2, 3, 4, 5, 6, 7],
    # EN: 10〜12が良かったが、より深く探索させる13〜15も追加
    "P_EN": [10, 11, 12, 13, 14, 15]
}

def main():
    # 組み合わせを生成
    keys = list(param_grid.keys())
    values = list(param_grid.values())
    combinations = list(itertools.product(*values))

    # 結果保存用のディレクトリを作成
    os.makedirs("results", exist_ok=True)
    
    total = len(combinations)
    print(f"Total combinations to test: {total}")

    for i, combo in enumerate(combinations):
        params = dict(zip(keys, combo))
        
        # マクロ定義用のフラグを作成 (例: -DP_SN=5 -DP_EN=12)
        param_flags = " ".join([f"-D{k}={v}" for k, v in params.items()])
        
        print(f"\n[{i+1}/{total}] Starting test with parameters:")
        for k, v in params.items():
            print(f"  {k} = {v}")
            
        # 1. コンパイル (make re を使って完全にクリーンビルド)
        print("  => Building...")
        build_cmd = f"make re PARAM_FLAGS='{param_flags}'"
        build_res = subprocess.run(build_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        if build_res.returncode != 0:
            print("  [ERROR] Build failed!")
            print(build_res.stderr.decode("cp932", errors="replace"))
            continue
            
        # 2. 実行と結果の記録
        print("  => Running...")
        
        # ファイル名を一意にするための文字列生成
        param_str = "_".join([f"{k.replace('P_','')}{v}" for k, v in params.items()])
        timestamp = datetime.now().strftime("%Y%md_%H%M%S")
        output_filename = f"results/result_{param_str}_{timestamp}.txt"
        
        start_time = time.time()
        run_cmd = "./exe"
        
        try:
            # プログラムを実行し、出力をキャプチャ
            process = subprocess.Popen(run_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            out, _ = process.communicate()
            end_time = time.time()
            elapsed_time = end_time - start_time
            
            # 結果をファイルに書き込む
            with open(output_filename, "w", encoding="utf-8") as f:
                f.write(f"--- Configuration ---\n")
                for k, v in params.items():
                    f.write(f"{k}: {v}\n")
                f.write(f"---------------------\n\n")
                
                f.write(out.decode("utf-8", errors="replace"))
                
                f.write(f"\n\n--- Performance ---\n")
                f.write(f"Total Execution Time: {elapsed_time:.3f} sec\n")
                
            print(f"  => Finished. Saved to {output_filename} ({elapsed_time:.2f} sec)")
            
        except Exception as e:
            print(f"  [ERROR] Execution failed: {e}")

if __name__ == "__main__":
    main()