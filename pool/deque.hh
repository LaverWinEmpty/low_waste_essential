#ifndef LWE_QUEUE_HEADER
#define LWE_QUEUE_HEADER

#include "allocator.hh"
#include "aligner.hh"

namespace lwe {
namespace data {

/**
 * @brief non thread-safe
 */
template<typename T, size_t Size = config::DEF_CACHE, size_t Align = config::DEF_ALIGN> class deque {
    struct node;
    using  allocator = lwe::mem::allocator::statics<sizeof(node), util::aligner::boundary(Align), Size>;

public:
    deque();
    ~deque();

public:
    /**
     * @brief push back
     */
    template<typename Arg> bool push(Arg&&);

public:
    /**
     * @brief pop_front
     *
     * @param  [out] nullptr: not get, call destructor
     * @return false: at empty
     */
    bool fifo(T* = nullptr);

public:
    /**
     * @brief pop_back
     *
     * @param  [out] nullptr: not get, call destructor
     * @return false: at empty
     */
    bool lifo(T* = nullptr);

public:
    /**
     * @brief peek first: like deque
     * @note  safe: not reallocate
     */
    T* front() const;

public:
    /**
     * @brief peek last: like stack
     * @note  safe: not reallocate
     */
    T* top() const;

private:
    /**
     * @brief actual delete (fifo / lifo)
     */
    bool pop(T* in, T* out);

private:
    /**
     * @brief allocate
     */
    static node* create();

public:
    /**
     * @brief current size, useable like bool type
     *
     * @return size_t
     */
    size_t size() const;

private:
    node*  first;
    node*  last;
    size_t count;
    size_t capacity;
};

} // namespace data
} // namespace lwe

#include "deque.inl"
#endif