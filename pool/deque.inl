#include "deque.hh"

namespace lwe {
namespace data {

template<typename T, size_t Size, size_t Align>
mem::allocator& deque<T, Size, Align>::heap =
    mem::allocator::statics<sizeof(node), util::aligner::boundary(Align), config::DEF_CACHE>();

template<typename T, size_t Size, size_t Align> struct deque<T, Size, Align>::node {
    /// @brief union for ignore calls to constructor and destructor
    union {
        T       array[Size];
        uint8_t serialized[sizeof(T) * Size];
    };

    node*  next;
    node*  prev;
    size_t head;
    size_t tail;
};

template<typename T, size_t Size, size_t Align> deque<T, Size, Align>::deque() {
    first = create();
    if(first) {
        capacity = Size;
    } else capacity = 0;
    last = first;
}

template<typename T, size_t Size, size_t Align> deque<T, Size, Align>::~deque() {
    while(first != nullptr) {
        for(size_t i = first->head; i < first->tail; ++i) {
            first->array[i].~T();
        }

        node* temp = first;
        first      = first->next;
        heap.deallocate(temp);
    }
}

template<typename T, size_t Size, size_t Align> template<typename Arg> bool deque<T, Size, Align>::push(Arg&& in) {
    if(last->tail == Size) {
        last->next = create();
        if(last->next) {
            last->next->prev = last;
            last             = last->next;

            capacity += Size;
        } else return false;
    }

    last->array[last->tail] = std::forward<Arg>(in);

    ++last->tail;
    ++count;
    return true;
}

template<typename T, size_t Size, size_t Align> bool deque<T, Size, Align>::fifo(T* out) {
    // false: at empty
    if(!pop(front(), out)) {
        return false;
    }

    // empty: index initialized
    if(count == 0) {
        return true;
    }

    // node is empty: free node and move to next
    if(++first->head == Size) {
        // next
        if(first->next) {
            capacity -= Size;
            first     = first->next;
            heap.deallocate(first->prev);
            first->prev = nullptr;
        }
    }
    return true;
}

template<typename T, size_t Size, size_t Align> bool deque<T, Size, Align>::lifo(T* out) {
    if(!pop(top(), out)) {
        return false;
    }

    // empty: index initialized
    if(count == 0) {
        return true;
    }

    // node is empty: free node and move to previous
    if(--last->tail == 0) {
        if(last->prev) {
            capacity -= Size;
            last      = last->prev;
            heap.deallocate(last->next);
            last->next = nullptr;
        }
    }
    return true;
}

template<typename T, size_t Size, size_t Align> T* deque<T, Size, Align>::front() const {
    if(count) {
        return first->array + first->head;
    }
    return nullptr;
}

template<typename T, size_t Size, size_t Align> T* deque<T, Size, Align>::top() const {
    if(count) {
        return last->array + (last->tail - 1);
    }
    return nullptr;
}

template<typename T, size_t Size, size_t Align> inline bool deque<T, Size, Align>::pop(T* in, T* out) {
    if(count == 0) {
        return false;
    }

    // return;
    if(out) {
        *out = *in;
    } else in->~T();

    // if last node than recycle
    if(--count == 0) {
        first->head = 0;
        first->tail = 0;
    }
    return true;
}

template<typename T, size_t Size, size_t Align> auto deque<T, Size, Align>::create() -> node* {
    node* ptr = static_cast<node*>(heap.allocate());
    if(ptr) {
        ptr->head = 0;
        ptr->tail = 0;
        ptr->next = nullptr;
        ptr->prev = nullptr;
    }
    return ptr;
}

template<typename T, size_t Size, size_t Align> size_t deque<T, Size, Align>::size() const {
    return count;
}

} // namespace data
} // namespace lwe