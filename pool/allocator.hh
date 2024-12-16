#ifndef LWE_ALLOCATOR_HEADER
#define LWE_ALLOCATOR_HEADER

#include "config.hh"
#include "macro.h"

namespace lwe {
namespace mem {
/**
 * @brief   the purpose is to optimize system call
 * @note    chunk only: not supported array
 * @warning different Cache result in different type
 */
template<size_t Size, size_t Align = config::DEF_ALIGN, size_t Cache = config::DEF_COUNT> class allocator {
public:
    allocator(const allocator&)            = delete;
    allocator(allocator&&)                 = delete;
    allocator& operator=(const allocator&) = delete;
    allocator& operator=(allocator&&)      = delete;

public:
    /**
     * @brief for array
     * @note  same as aligned_alloc / aligned_free
     */
    struct uncaching {
        uncaching() = delete;
        static void* allocate(size_t) noexcept;
        static void  deallocate(void*) noexcept;
    };

public:
    /**
     * @brief use singleton pattern
     */
    allocator() noexcept;

public:
    /**
     * @brief destructor for static object
     */
    ~allocator() noexcept;

private:
    /**
     * actual allocate
     */
    void* get() const noexcept;

private:
    /**
     * actual deallocate
     */
    void free(void*) const noexcept;

public:
    /**
     * @brief  call by default instance
     * @param  [in] count
     * @return failed: nullptr
     */
    static void* allocate() noexcept;

public:
    /**
     * @brief call by default instance
     * @note  number of pointer keep: config::DEF_COUNT
     */
    static void deallocate(void*) noexcept;

private:
    /**
     *@brief index like stack top
     */
    mutable size_t count;

private:
    /**
     * @brief does not affect ownership.
     */
    mutable void* store[Cache];

private:
    /**
     * @brief default instance
     */
    static thread_local allocator singleton;
}; // namespace mem

} // namespace mem
} // namespace lwe
#include "allocator.inl"
#endif