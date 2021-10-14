#include <cstdint>
#include "raw_array.h"
#include <utility>

std::pair<raw_array<int32_t>, int32_t> scan_exclusive_blocked(raw_array<int32_t> const& x, uint32_t blocks_count);

std::pair<raw_array<int32_t>, int32_t> scan_exclusive_sequential(raw_array<int32_t> const& x);