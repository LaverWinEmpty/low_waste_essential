#ifndef LWE_ALLOCATOR_HEADER
#define LWE_ALLOCATOR_HEADER

#include "config.hpp"

namespace lwe {
namespace mem {

/**
 * @brief the purpose is to optimize system call
 */
template<size_t Size, size_t Align = config::DEF_ALIGN> class allocator {
    template<size_t Storage> class object {
    public:
        object(const object&)            = delete;
        object(object&&)                 = delete;
        object& operator=(const object&) = delete;
        object& operator=(object&&)      = delete;

    public:
        object() noexcept;
        ~object();

    public:
        void* allocate() const;
        void  deallocate(void*) const;

    private:
        /**
         * storage counter
         */
        mutable size_t count;

    private:
        /**
         * @brief does not affect ownership.
         */
        mutable void* store[Storage];
    };

public:
    /**
     * @brief call by default instance
     * @return failed: nullptr
     */
    static void* allocate(size_t) noexcept;

public:
    /**
     * @brief call by default instance
     */
    static void deallocate(void*) noexcept;

private:
    /**
     * @brief default instance
     */
    static thread_local object<config::DEF_COUNT> instance;
};

}}
#include "allocator.hxx"
#endif