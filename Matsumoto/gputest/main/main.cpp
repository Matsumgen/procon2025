#include <iostream>
#include <string>
#include <algo.cuh> // solve_from_file の宣言があるとして

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <problem_file_path>" << std::endl;
        return 1;
    }
    std::string filepath = argv[1];
    solve_from_file(filepath);
    return 0;
}