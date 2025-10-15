// IDDFS
// 時間がかかりすぎてだめ
#include <algo2_1.hpp>
#include <array>
#include <iostream>
#include <vector>

bool dfs(Field &field, int limit, int depth,
         std::vector<std::array<int, 3>> &path) {
  if (field.isEnd())
    return true;
  if (depth >= limit)
    return false;

  int size = field.getSize();
  for (int y = 0; y <= size - 2; ++y) {
    for (int x = 0; x <= size - 2; ++x) {
      for (int n = 2; x + n <= size && y + n <= size; ++n) {
        if (field.canRotate(x, y, n)) {
          Field next = field;
          next.rotate(x, y, n);
          path.push_back({x, y, n});

          if (dfs(next, limit, depth + 1, path)) {
            field = next; // 解を field に反映
            return true;
          }

          path.pop_back();
        }
      }
    }
  }
  return false;
}

void alg2_1(Field &f, int dummy1, int dummy2, int dummy3) {
  (void)dummy1;
  (void)dummy2;
  (void)dummy3;
  for (int depth = 1;; ++depth) {
    std::vector<std::array<int, 3>> ops;
    Field temp = f;
    if (dfs(temp, depth, 0, ops)) {
      for (auto &op : ops)
        f.rotate(op[0], op[1], op[2]);
      std::cout << "Solved at depth: " << depth << std::endl;
      return;
    }
    printf("完了 %d\n", depth);
  }
  std::cerr << "No solution found." << std::endl;
}
