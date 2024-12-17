#ifndef LWE_POOL_HEADER
#define LWE_POOL_HEADER

#include "aligner.hh"
#include "allocator.hh"
#include "deque.hh"

#include <unordered_set>
#include <concurrent_queue.h>
#include <cstdlib>

// TODO: padding by alignment value

namespace lwe {
namespace mem {

class pool {
public:
    pool(const pool&)            = delete;
    pool& operator=(const pool&) = delete;
    pool(pool&&)                 = delete;
    pool& operator=(pool&&)      = delete;

private:
    /**
     * @brief memory pool block node
     */
    struct block;

public:
    /**
     * @brief thread-safe static memory pool
     * @note  1. use thread_local, it is thread safe but pool count is same as thread count
     * @note  2. different template parameter result in different types
     */
    template<size_t Size, size_t Count = config::DEF_CACHE, size_t Align = config::DEF_ALIGN> class statics {
        static thread_local pool singleton;

    public:
        static void* allocate();
        static void  deallocate(void* in);
        static void  cleanup();
    };

public:
    /**
     * @brief construct a new pool object
     *
     * @param [in] chunk - chunk size, it is padded to the pointer size.
     * @param [in] count - chunk count.
     * @param [in] align - chunk Align, it is adjusted to the power of 2.
     */
    pool(size_t chunk, size_t count = config::DEF_CACHE, size_t align = config::DEF_ALIGN);

public:
    /**
     * @brief destroy the pool object.
     */
    ~pool();

public:
    /**
     * @brief get memory, call malloc when top is null and garbage collector is empty.
     */
    template<typename T = void, typename... Args> T* construct(Args&&...) noexcept;

public:
    /**
     * @brief return memory, if not child of pool, push to garbage collector of parent pool.
     */
    template<typename T = void> void destruct(T*) noexcept;

public:
    /**
     * @brief cleaning idle blocks and garbage collector.
     */
    void cleanup();

private:
    /**
     * @brief call memory allocate function.
     */
    block* setup();

private:
    /**
     * @brief chunk to block.
     */
    void release(void*);

public:
    /**
     * @brief auto return memory to parent pool
     */
    static void revert(void*) noexcept;

private:
    /**
     * @brief memory alignment
     * @note  FIRST INTIALIZE ON CONSTRUCTOR
     */
    const size_t ALIGNMENT;

private:
    /**
     * @brief chunk size
     */
    const size_t SIZE;

private:
    /**
     * @brief chunk count
     */
    const size_t COUNT;

private:
    /**
     * @brief allocated size
     */
    const size_t ALLOCTATE;

private:
    /**
     * @brief stack: current using block
     */
    block* top = nullptr;

private:
    /**
     * @brief idle blocks
     */
    data::deque<block*> idle;

private:
    /**
     * @brief generated blocks
     */
    std::unordered_set<block*> all;

private:
    /**
     * @brief temp lock-free queue for windows
     */
    Concurrency::concurrent_queue<void*> gc;
};

} // namespace mem
} // namespace lwe

#include "pool.inl"
#endif