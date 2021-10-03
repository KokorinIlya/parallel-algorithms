#include <vector>
#include <cstdint>

void calc_sequential(std::vector<int32_t> const& x, std::vector<int32_t>& res);

void calc_parallel(std::vector<int32_t> const& x, uint32_t threads, std::vector<int32_t>& res);