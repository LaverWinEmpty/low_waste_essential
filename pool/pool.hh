#ifndef LWE_POOL_HEADER
#define LWE_POOL_HEADER

#include "aligner.hh"
#include "allocator.hh"
#include "deque.hh"

#include <queue>
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
    static constexpr size_t DEFAULT_COUNT = (sizeof(intptr_t) * sizeof(intptr_t)) << 2;
    static constexpr size_t DEFAULT_ALIGN = sizeof(intptr_t);

private:
    /**
     * @brief helper to next address in empty space when idle
     */
    struct chunk;

    /**
     * @brief memory pool block node
     */
    struct block;

public:
    /**
     * @brief thread-safe static memory pool
     * @note  1. use thread_local, it is thread safe but pool count is same as thread count
     * @note  2. different template parameter ​​result in different types
     */
    template<size_t Size, size_t Count = DEFAULT_COUNT, size_t Align = DEFAULT_ALIGN> class global {
        static thread_local pool singleton;

    public:
        /**
         * @brief malloc
         */
        static void* allocate();

    public:
        /**
         * @brief free
         */
        static void deallocate(void* in);

    public:
        /**
         * @brief call pool::cleanup
         */
        static void cleanup();
    };

public:
    /**
     * @brief thread-safe static memory pool based on object type
     * @note  1. use thread_local, it is thread safe but pool count is same as thread count
     * @note  2. different template parameter ​​result in different types
     */
    template<typename T, size_t Count = DEFAULT_COUNT, size_t Align = DEFAULT_ALIGN> class constructor {
        using type = global<sizeof(T), Count, Align>;

    public:
        /**
         * @brief new
         */
        template<typename... Args> T* construct(Args&&...);

    public:
        /**
         * @brief delete
         */
        void destruct(T*);

    public:
        /**
         * @brief call pool::cleanup
         */
        void cleanup();
    };

public:
    /**
     * @brief construct a new pool object
     *
     * @param [in] chunk - chunk size, it is padded to the pointer size.
     * @param [in] count - chunk count.
     * @param [in] align - chunk Align, it is adjusted to the power of 2.
     */
    pool(size_t chunk, size_t count = DEFAULT_COUNT, size_t align = DEFAULT_ALIGN);

public:
    /**
     * @brief destroy the pool object.
     */
    ~pool();

public:
    /**
     * @brief get memory, call malloc when top is null and garbage collector is empty.
     */
    void* allocate();

public:
    /**
     * @brief return memory, if not child of pool, push to garbage collector of parent pool.
     */
    void deallocate(void*);

public:
    /**
     * @brief cleaning idle blocks and garbage collector.
     */
    void cleanup();

private:
    /**
     * @brief call memory allocate function.
     */
    block* generate();

private:
    /**
     * @brief chunk to block.
     */
    void free(chunk*);

public:
    /**
     * @brief return memory to parent pool
     */
    static void release(void*);

private:
    /**
     * @brief value to power of 2.
     */
    static size_t alignment(size_t);

private:
    /**
     * @brief value to muiple of pointer size.
     */
    static size_t padding(size_t in);

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
     * @brief memory alignment
     */
    const size_t ALIGNMENT;

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
    Concurrency::concurrent_queue<chunk*> gc;
};

} // namespace mem
} // namespace lwe

#include "pool.inl"
#endif