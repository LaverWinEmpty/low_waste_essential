#ifndef LWE_ALLOCATOR_HEADER
#define LWE_ALLOCATOR_HEADER

#include "aligner.hh"
#include "config.hh"

namespace lwe {
namespace mem {

class allocator {
public:
    static void* malloc(size_t size, size_t align) noexcept;
    static void  free(void*) noexcept;

public:
    allocator(size_t size, size_t align = config::DEF_ALIGN, size_t cache = config::DEF_CACHE);
    ~allocator() noexcept;

public:
    allocator(const allocator&)            = delete;
    allocator(allocator&&)                 = delete;
    allocator& operator=(const allocator&) = delete;
    allocator& operator=(allocator&&)      = delete;

public:
    void* allocate()        noexcept;
    void  deallocate(void*) noexcept;

private:
    const size_t ALIGNMENT;
    const size_t SIZE;
    const size_t CACHE;

private:
    size_t count = 0;
    void** stack = nullptr;

public:
    template<size_t Size, size_t Align = config::DEF_ALIGN, size_t Cache> static allocator& statics();
};

} // namespace mem
} // namespace lwe
#include "allocator.inl"
#endif