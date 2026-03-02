#ifndef ALGO1_CUH_
#define ALGO1_CUH_

#include <stdint.h>
#include <vector>


const uint32_t tidPerField_list[] = { 0, 0, 0, 0,13, 0, 54, 0, 139, 0, 284, 0, 505, 0, 818, 0, 1239, 0, 1784, 0, 2469, 0, 3310, 0, 4323 };

namespace bs1{
  std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize);
};
namespace bs11{
  std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize);
};
namespace bs2{
  std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize);
};
namespace bs3{
  std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize);
};
namespace bs4{
  std::vector<uint32_t> algo(uint16_t *start_field, uint32_t fsize);
};


#endif
