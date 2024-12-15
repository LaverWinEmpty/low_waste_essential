#ifndef LWE_QUEUE_HEADER
#define LWE_QUEUE_HEADER

#include "allocator.hpp"

// TODO: queue -> deque
// TODO: stack(cache) -> allocator

namespace lwe {

/**
 * @brief non thread-safe 
 */
template<typename T, size_t Size = config::DEF_COUNT, size_t Align = config::DEF_ALIGN> class queue {
    static constexpr size_t ALIGNMENT = aligner::boundary(Align);

public:
    struct node {
        node*  next;
        size_t head;
        size_t tail;

        union {
            T       array[Size];
            uint8_t serialized[sizeof(T) * Size];
        };
    };
    
public:
    queue();
    ~queue();

public:
    template<typename Arg> bool enqueue(Arg&&);

public:
    bool dequeue(T* = nullptr);

public:
    T* front() const;
    
private:
    static node* create();

public:
    size_t size() const;
    
private:
    node*  first;
    node*  last;
    size_t count;
    size_t capacity;
};

}

#endif