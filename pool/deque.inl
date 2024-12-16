#include "deque.hh"

namespace lwe {
namespace data {

template<typename T, size_t Size, size_t Align> struct deque<T, Size, Align>::node {
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
        allocator::deallocate(temp);
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

    last->array[last->tail++] = std::forward<Arg>(in);

    ++count;
    return true;
}

template<typename T, size_t Size, size_t Align> bool deque<T, Size, Align>::fifo(T* out) {
    if(!count) {
        return false;
    }

    // return
    if(!out) {
        first->array[first->head].~T(); // call destructor
    } else *out = first->array[first->head];
    --count;

    //
    if(++first->head == Size) {
        // next
        if(first->next) {
            first = first->next;
            allocator::deallocate(first->prev);
            first->prev  = nullptr;
            capacity    -= Size;
        }
        // recycle
        else {
            first->head = 0;
            first->tail = 0;
        }
    }

    // init
    else if(count == 0) {
        first->head = 0;
        first->tail = 0;
    }

    return true;
}

template<typename T, size_t Size, size_t Align> bool deque<T, Size, Align>::lifo(T* out) {
    if(!count) {
        return false;
    }

    // return
    --last->tail;
    if(!out) {
        last->array[last->tail].~T(); // call destructor
    } else *out = last->array[last->tail];
    --count;

    // check
    if(last->tail == 0) {
        // prev
        if(last->prev) {
            last = last->prev;
            allocator::deallocate(last->next);
            last->next = nullptr;
            --capacity;
        }
        // recycle
        else {
            last->head = 0;
            last->tail = 0;
        }
    }

    // init
    else if(count == 0) {
        first->head = 0;
        first->tail = 0;
    }

    return true;
}

template<typename T, size_t Size, size_t Align> T* deque<T, Size, Align>::front() const {
    if(size()) {
        return first->array + first->head;
    }
    return nullptr;
}

template<typename T, size_t Size, size_t Align> T* deque<T, Size, Align>::top() const {
    if(size()) {
        return last->array + (last->tail - 1);
    }
    return nullptr;
}

template<typename T, size_t Size, size_t Align> auto deque<T, Size, Align>::create() -> node* {
    node* ptr = static_cast<node*>(allocator::allocate());
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