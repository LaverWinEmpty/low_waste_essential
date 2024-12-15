namespace lwe {

template<typename T, size_t Size, size_t Align> queue<T, Size, Align>::queue(): count(0) {
    first = create();
    if(first) {
        capacity = Size;
    } else capacity = 0;
    last = first;
}

template<typename T, size_t Size, size_t Align> queue<T, Size, Align>::~queue() {
    while(first != nullptr) {
        for(size_t i = first->head; i < first->tail; ++i) {
            first->array[i].~T();
        }

        node* temp = first;
        first      = first->next;
        _aligned_free(temp);
    }
}

template<typename T, size_t Size, size_t Align> template<typename Arg> bool queue<T, Size, Align>::enqueue(Arg&& in) {
    if(last->tail == Size) {
        node* ptr = create();
        if(ptr) {
            last->next  = ptr;
            last        = ptr;
            capacity   += Size;
        } else return false;
    }

    last->array[last->tail++] = std::forward<Arg>(in);
    ++count;
    return true;
}

template<typename T, size_t Size, size_t Align> bool queue<T, Size, Align>::dequeue(T* out) {
    if(count == 0) {
        return false;
    }

    // return
    if(!out) {
        first->array[first->head].~T(); // call destructor
    } else *out = first->array[first->head];

    // ran out
    ++first->head;
    if(first->head == Size) {
        capacity -= Size;
        // next
        if(first != last) {
            pool.push(first);
            first = first->next;
        }
        // recycle
        else {
            first->head = 0;
            first->tail = 0;
        }
    }

    --count;
    return true;
}

template<typename T, size_t Size, size_t Align> T* queue<T, Size, Align>::front() const {
    if(count) {
        return first->array + first->head;
    }
    return nullptr;
}

template<typename T, size_t Size, size_t Align> auto queue<T, Size, Align>::create() -> node* {
    node* ptr;
    if(pool.empty()) {
        ptr = static_cast<node*>(_aligned_malloc(sizeof(node), ALIGNMENT));
    } else pool.pop(&ptr);
    if(ptr) {
        ptr->head = 0;
        ptr->tail = 0;
        ptr->next = nullptr;
    }
    return ptr;
}

template<typename T, size_t Size, size_t Align> size_t queue<T, Size, Align>::size() const {
    return count;
}

} // namespace lwe