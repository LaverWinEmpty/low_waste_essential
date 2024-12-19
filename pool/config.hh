#ifndef LWE_CONFIG_HEADER
#define LWE_CONFIG_HEADER

//=============================================================================
// C library
//=============================================================================
#include <cstdint>
#include <cstdlib>

//=============================================================================
// C++ library
//=============================================================================
#include <atomic>
#include <utility>
#include <unordered_set>

//=============================================================================
// thirdparty
//=============================================================================
#include <concurrent_queue.h>

namespace lwe {
namespace config {
enum {
    DEF_ALIGN = sizeof(void*),
    DEF_CACHE = (sizeof(void*) * sizeof(void*)) << 1,
};
} // namespace config
} // namespace lwe
#endif