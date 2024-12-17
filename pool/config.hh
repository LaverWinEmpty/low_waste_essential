#ifndef LWE_CONFIG_HEADER
#define LWE_CONFIG_HEADER

#include <cstdint>

namespace lwe {
namespace config {

static constexpr size_t DEF_ALIGN = sizeof(void*);
static constexpr size_t DEF_CACHE = (sizeof(void*) * sizeof(void*)) << 1;

} // namespace config
} // namespace lwe
#endif