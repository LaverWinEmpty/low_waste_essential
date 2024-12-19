#ifndef LWE_ALLOCATOR_HEADER
#define LWE_ALLOCATOR_HEADER

// TODO: count to atomic

#include <cstdlib>
#include "aligner.hh"
#include "config.hh"

/********************************************************************************
 * allocator
 *
 * not thread safe
 *
 * static mehod is nocaching
 ********************************************************************************/

namespace lwe {
namespace mem {

class allocator {
public:
    /// @brief aligned malloc, size and align is adjust
    static void* malloc(size_t size, size_t align = config::DEF_ALIGN) noexcept;
    static void  free(void*) noexcept;

public:
    allocator(size_t size, size_t align = config::DEF_ALIGN, size_t cache = config::DEF_CACHE) noexcept;
    ~allocator() noexcept;

public:
    allocator(const allocator&)            = delete;
    allocator(allocator&&)                 = delete;
    allocator& operator=(const allocator&) = delete;
    allocator& operator=(allocator&&)      = delete;

public:
    void* allocate() noexcept;
    void  deallocate(void*) noexcept;

private:
    const size_t ALIGN;
    const size_t SIZE;
    const size_t CACHE;

private:
    size_t count = 0;
    void** stack = nullptr;

public:
    template<size_t Size, size_t Align = config::DEF_ALIGN, size_t Cache = config::DEF_CACHE>
    static allocator& statics() noexcept;
};

} // namespace mem
} // namespace lwe
#include "allocator.inl"
#endif