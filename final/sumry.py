import os
import glob
import re
import csv

def summarize_results(folder_path, output_csv="summary.csv"):
    # 指定されたフォルダ内のすべての .txt ファイルを取得
    file_pattern = os.path.join(folder_path, "*.txt")
    files = glob.glob(file_pattern)
    
    if not files:
        print(f"[警告] 指定されたフォルダ '{folder_path}' に .txt ファイルが見つかりません。")
        return

    summary_data = []
    
    # 抽出用の正規表現
    re_result_size = re.compile(r"result size:\s*(\d+)")
    re_exec_time = re.compile(r"Total Execution Time:\s*([\d.]+)\s*sec")
    
    # 全パラメータのキー（列名）を保持するセット
    all_param_keys = set()

    print(f"{len(files)} 件のファイルを処理中...")

    for filepath in files:
        # 1ファイル分のデータを格納する辞書
        data = {
            "Filename": os.path.basename(filepath),
            "Result Size": None,
            "Execution Time (sec)": None,
            "Used pmode=2": False
        }
        
        in_config = False
        
        # ファイルを読み込んで解析
        # ※Windows環境を考慮し、文字コードエラーを無視する設定にしています
        with open(filepath, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.strip()
                
                # --- Configuration --- ブロックの判定
                if line == "--- Configuration ---":
                    in_config = True
                    continue
                elif line == "---------------------":
                    in_config = False
                    continue
                
                # パラメータの抽出
                if in_config and ":" in line:
                    key, val = line.split(":", 1)
                    key = key.strip()
                    val = val.strip()
                    data[key] = val
                    all_param_keys.add(key)
                
                # "result size: XX" の抽出
                m_size = re_result_size.search(line)
                if m_size:
                    data["Result Size"] = int(m_size.group(1))
                
                # "Total Execution Time: XX sec" の抽出
                m_time = re_exec_time.search(line)
                if m_time:
                    data["Execution Time (sec)"] = float(m_time.group(1))
                    
                # 激重モード(pmode=2)に入ったかのチェック
                if "pmode=2" in line:
                    data["Used pmode=2"] = True
                    
        summary_data.append(data)
        
    # パラメータ列をアルファベット順にソート
    param_columns = sorted(list(all_param_keys))
    
    # CSVの列順を定義（ファイル名 -> パラメータ群 -> 結果 -> 実行時間 -> pmode=2判定）
    fieldnames = ["Filename"] + param_columns + ["Result Size", "Execution Time (sec)", "Used pmode=2"]
    
    # 結果をCSVファイルに書き出し
    with open(output_csv, "w", newline="", encoding="utf-8-sig") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        
        # 実行時間 (Execution Time) が短い順に並び替えてから書き込む
        summary_data.sort(key=lambda x: x["Execution Time (sec)"] if x["Execution Time (sec)"] is not None else float('inf'))
        
        for row in summary_data:
            writer.writerow(row)
            
    print(f"\n集計が完了しました！")
    print(f"結果を '{output_csv}' に保存しました。")

if __name__ == "__main__":
    # ==========================================
    # ここに結果ファイルが保存されているフォルダ名を指定します
    # ==========================================
    target_folder = "results"  
    
    # 出力されるCSVファイル名
    output_filename = "summary_results.csv"
    
    summarize_results(target_folder, output_filename)