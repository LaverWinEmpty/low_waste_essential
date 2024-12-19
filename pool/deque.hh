#ifndef LWE_QUEUE_HEADER
#define LWE_QUEUE_HEADER

#include "allocator.hh"
#include "aligner.hh"

namespace lwe {
namespace data {

/*******************************************************************************
 * a fixed array with a size of the parameter 'Size'.
 * when full, it creates a new array and behaves like a doubly linked list.
 * the created array is stored and reused by the allocator when destroyed.
 * the allocator itself uses a static object.
 *
 * NOTE: allocator cache is fixed to 'config::DEF_CACHE'
 ******************************************************************************/
template<typename T, size_t Size = config::DEF_CACHE, size_t Align = config::DEF_ALIGN> class deque {
    struct node;

public:
    deque();
    ~deque();

public:
    /// @brief push back
    template<typename Arg> bool push(Arg&&);

public:
    /// @brief pop_front
    /// @param  [out] nullptr: not get, call destructor
    /// @return false: at empty
    bool fifo(T* = nullptr);

public:
    /// @brief pop_back
    /// @param  [out] nullptr: not get, call destructor
    /// @return false: at empty
    bool lifo(T* = nullptr);

public:
    /// @brief peek first: like deque
    /// @note  safe: not reallocate
    T* front() const;

public:
    /// @brief peek last: like stack
    /// @note  safe: not reallocate
    T* top() const;

private:
    /// @brief actual delete (fifo / lifo)
    bool pop(T* in, T* out);

private:
    /// @brief allocate
    static node* create();

public:
    /// @brief current size, useable like bool type
    /// @return size_t
    size_t size() const;

private:
    node*  first;
    node*  last;
    size_t count;
    size_t capacity;

private:
    static mem::allocator& heap;
};

} // namespace data
} // namespace lwe

#include "deque.inl"
#endif