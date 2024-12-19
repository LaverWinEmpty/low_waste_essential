#ifndef LWE_ALIGNER_HEADER
#define LWE_ALIGNER_HEADER

#include "config.hh"

namespace lwe {
namespace util {

/// @brief static class
struct aligner {
    aligner() = delete;

    /// @brief  check power of 2
    /// @note   pointer too
    /// @return false is not power of 2
    static constexpr bool check(size_t in) {
        if(in == 0) {
            return false;
        }
        return !(in & (in - 1));
    };

    /// @brief value to next power of 2
    /// @note  eg. x86: [9 -> 12] | x64: [9 -> 16]
    static constexpr size_t boundary(size_t in) {
        if(in <= sizeof(void*)) {
            return sizeof(void*);
        }
        in -= 1;
        for(size_t i = 1; i < sizeof(size_t); i <<= 1) {
            in |= in >> i;
        }
        return in + 1;
    }

    /// @brief value to a muliple of the power of 2
    /// @note  eg. (9, 6) -> return 16: (9 + 8 - 1) / 8 * 8
    static constexpr size_t padding(size_t in, size_t target = config::DEF_ALIGN) {
        if(!check(target)) {
            target = boundary(target);
        }
        return (in + target - 1) & ~(target - 1);
    }
};

} // namespace util
} // namespace lwe
#endif