cmake_minimum_required(VERSION 3.10)
project(ProconAPIClient CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET "15.6")

# vcpkg のツールチェーンファイルを見つける
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(procon_client connection.cpp)

# ライブラリをリンクする
target_link_libraries(procon_client PRIVATE cpr::cpr nlohmann_json::nlohmann_json)