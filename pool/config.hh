#ifndef LWE_CONFIG_HEADER
#define LWE_CONFIG_HEADER

#include <cstdint>

namespace lwe {
namespace config {
enum {
    DEF_ALIGN = sizeof(void*),
    DEF_CACHE = (sizeof(void*) * sizeof(void*)) << 1,
};
} // namespace config
} // namespace lwe
#endif