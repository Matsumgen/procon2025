# CMakeの最低バージョンを指定
cmake_minimum_required(VERSION 3.10)

# プロジェクト名と使用言語(C++)を定義
project(ProconSolver CXX)

# C++17規格を使用するように設定
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOSのバージョン警告を抑制するための設定（前回の警告内容に合わせています）
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.6")

# --------------------------------------------------------------------

# コンパイルするソースファイル(.cpp)をすべて自動で探す
# "src/*.cpp" は、srcフォルダ内のすべての.cppファイルという意味
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 実行可能ファイルを作成する
# 実行ファイル名を "procon_solver" とし、SOURCES変数に入っているすべての.cppファイルから作成
add_executable(procon_solver ${SOURCES})

# --------------------------------------------------------------------

# ヘッダーファイル(.hpp)が置かれている場所をコンパイラに教える
# これにより、#include "../inc/all.hpp" が正しく解決される
target_include_directories(procon_solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# --------------------------------------------------------------------

# vcpkgでインストールした外部ライブラリを見つけてくる
# cpr (HTTPリクエスト用) と nlohmann_json (JSON解析用)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# 実行可能ファイルに外部ライブラリをリンク（接続）する
# これにより、cprやjsonの関数がプログラム内で使えるようになる
target_link_libraries(procon_solver PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)