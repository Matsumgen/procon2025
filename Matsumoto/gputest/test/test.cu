#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <algo.cuh>

namespace fs = std::filesystem;

// 問題フォルダのパス
const std::string TARGET_FOLDER = "/mnt/c/Users/sisim/Documents/procon/test";

int main() {
    // 1. フォルダ存在確認
    if (!fs::exists(TARGET_FOLDER)) {
        std::cerr << "Folder not found: " << TARGET_FOLDER << std::endl;
        return 1;
    }

    std::vector<std::string> files;

    // 2. フォルダ内の .txt ファイルを収集
    for (const auto& entry : fs::directory_iterator(TARGET_FOLDER)) {
        if (entry.path().extension() == ".txt") {
            // 【重要】ここで entry.path().string() を使うとフルパスになります。
            // filename() を使うとファイル名だけになり、読み込めなくなります。
            files.push_back(entry.path().string());
        }
    }

    // ソート
    std::sort(files.begin(), files.end());

    std::cout << "Target Folder: " << TARGET_FOLDER << std::endl;
    std::cout << "Found " << files.size() << " files. Start processing..." << std::endl;
    
    std::vector<SolveResult> results;
    
    // 3. 各ファイルを順に処理
    for (const auto& f : files) {
        // 表示用: フルパスからファイル名だけを取り出す
        std::string fname_only = f.substr(f.find_last_of("/\\") + 1);
        std::cout << "Solving " << fname_only << " ..." << std::endl;
        
        // ソルバーには「フルパス」を渡す
        SolveResult res = solve_from_file(f);
        results.push_back(res);
    }

    // 4. 結果のまとめ表示
    std::cout << "\n================ SUMMARY ================" << std::endl;
    std::cout << std::left << std::setw(30) << "Filename" 
              << std::right << std::setw(6) << "Size"
              << std::right << std::setw(10) << "Time(s)" 
              << std::setw(10) << "Moves" 
              << std::setw(10) << "Status" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    for (const auto& res : results) {
        std::string fname = res.filename.substr(res.filename.find_last_of("/\\") + 1);
        
        std::cout << std::left << std::setw(30) << fname 
                  << std::right << std::setw(6) << res.fsize
                  << std::right << std::setw(10) << std::fixed << std::setprecision(3) << res.time_seconds 
                  << std::setw(10) << res.moves 
                  << std::setw(10) << (res.solved ? "OK" : "FAIL") << std::endl;
    }
    std::cout << "=========================================" << std::endl;

    return 0;
}