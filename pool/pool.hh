#ifndef LWE_POOL_HEADER
#define LWE_POOL_HEADER

// #include "queue"

#include "deque.hh"

/*******************************************************************************
 * pool structure
 *
 * example configuration
 *  - chunk: 96
 *  - align: 32
 *  - count: 2
 *
 * 192                 256                384         512 | << address
 * ^                   ^                  ^           ^   |
 * ├───────┬────┬──────┼──────┬────┬──────┼──────┬────┤   |
 * │ block │    │ meta │ data │    │ meta │ data │    │   |
 * └───────┼────┼──────┴──────┼────┼──────┴──────┼────┤   |
 *         └ 24 ┤             ├ 24 ┤             ├ 32 ┘   | << padding
 *              └ ─  chunk  ─ ┘    └ ─  chunk  ─ ┘
 *
 * total: 320 byte (64 + 128 * 2)
 *
 * - block : block header (struct) like node
 *   ├─[ 8 byte]: next chunk pointer
 *   ├─[ 8 byte]: next block pointer
 *   ├─[ 8 byte]: prev block pointer
 *   ├─[ 8 byte]: parent pool pointer
 *   └─[32 byte]: padding
 *
 * - chunk: not a struct, abstract object for dynamic chunk size.
 *   ├─[ 8 byte]: parent block address (meta)
 *   ├─[96 byte]: actual usable space  (data)
 *   └─[24 byte]: padding
 *
 * NOTE: align is intended for SIMD use and increases capacity.
 * block header padding reason: to ensures alignment for chunk start addresses.
 *
 ******************************************************************************/

namespace lwe {
namespace mem {

/*******************************************************************************
 * NOTE: STATICS IS RELEASE WHEN TERMINATE END
 *******************************************************************************
 * how to use
 *
 * 1. use statics
 * - thread local object (lock-free)
 *
 * @code {.cpp}
 *  type* ptr = pool::statics<sizeof(type)>().construct(...); // malloc
 *  pool::statics<sizeof(type)>().deconstruct<type>(ptr);     // free
 * @endcode
 *
 * 2. create object (NOT THREAD-SAFE)
 *  - NOTE: safe when declared as thread_local.
 *
 * @code {.cpp}
 *  pool p1(sizeof(type), 512, 8);       // declare pool object 1
 *  pool p2(sizeof(type), 256, 32);      // declare pool object 2
 *  type* ptr = p1.construct<type>(...); // malloc "p1"
 *  p2.destruct<type>(ptr);              // free "p2" is ok (not recommended)
 * @endcode
 *
 * - automatically finds parent.
 * - but in this case, it goes into gc. (not recommanded reason)
 * - can also use it like this.
 *
 * @code {.cpp}
 *  pool::release(ptr); // free
 * @endcode
 *
 * - this is for use when the parents are unknown.
 *
 * gc (garbage collector)
 * - lock-free queue.
 * - push if parents are different.
 * - gc cleanup is call cleanup(), or call destructe().
 * - used for thread-safety.
 *
 * - unused memory can be free by calling cleanup().
 * - call at the right time.
 * - NOTE: it is actual memory deallocate.
 ******************************************************************************/
class pool {
public:
    pool(const pool&)            = delete;
    pool& operator=(const pool&) = delete;
    pool(pool&&)                 = delete;
    pool& operator=(pool&&)      = delete;

private:
    /**
     * @brief memory pool block node
     * @note  4 pointer = 32 byte in x64
     */
    struct block;

public:
    /**
     * @brief thread-safe static memory pool
     * @note  1. use thread_local, it is thread safe but pool count is same as thread count
     * @note  2. different template parameter result in different types
     */
    template<size_t Size, size_t Align = config::DEF_ALIGN, size_t Count = config::DEF_CACHE> static pool& statics();

public:
    /**
     * @brief construct a new pool object
     *
     * @param [in] chunk - chunk size, it is padded to the pointer size.
     * @param [in] align - chunk align, it is adjusted to the power of 2.
     * @param [in] count - chunk count in block.
     */
    pool(size_t chunk, size_t align = config::DEF_ALIGN, size_t count = config::DEF_CACHE) noexcept;

public:
    /// @brief destroy the pool object.
    ~pool() noexcept;

public:
    /// @brief get memory, call malloc when top is null and garbage collector is empty.
    template<typename T = void, typename... Args> T* construct(Args&&...) noexcept;

public:
    /// @brief return memory, if not child of pool, push to garbage collector of parent pool.
    template<typename T = void> void destruct(T*) noexcept;

public:
    /// @brief cleaning idle blocks and garbage collector.
    void cleanup() noexcept;

private:
    /// @brief call memory allocate function.
    block* setup() noexcept;

private:
    /// @brief chunk to block.
    void recycle(void*) noexcept;

public:
    /// @brief auto return memory to parent pool
    template<typename T = void> static void release(T*) noexcept;

private:
    const size_t ALIGN;
    const size_t BLOCK;
    const size_t CHUNK;
    const size_t COUNT;
    const size_t TOTAL;

private:
    /// @brief stack: current using block
    block* top = nullptr;

private:
    /// @brief idle blocks
    data::deque<block*> idle;

private:
    /// @brief generated blocks
    std::unordered_set<block*> all;

private:
    /// @brief temp lock-free queue for windows
    moodycamel::ConcurrentQueue<void*> gc;
};

} // namespace mem
} // namespace lwe

#include "pool.inl"
#endif