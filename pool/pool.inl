namespace lwe {
namespace mem {

template<size_t Size, size_t Count, size_t Align>
thread_local pool pool::global<Size, Count, Align>::singleton{ Size, Count, Align };

struct pool::chunk {
    chunk* move() const;

    static void*  object(chunk*);
    static chunk* instance(void*);

    block* from;
    chunk* next;
};

struct pool::block {
    void   initialize(pool* parent, size_t count);
    chunk* get();
    void   set(chunk*);

    pool*  from;
    chunk* curr;
    block* next;
    block* prev;
};

template<size_t Size, size_t Count, size_t Align> inline void* pool::global<Size, Count, Align>::allocate() {
    return singleton.allocate();
}

template<size_t Size, size_t Count, size_t Align> inline void pool::global<Size, Count, Align>::deallocate(void* in) {
    singleton.deallocate(in);
}

template<size_t Size, size_t Count, size_t Align> inline void pool::global<Size, Count, Align>::cleanup() {
    singleton.cleanup();
}

template<typename T, size_t Count, size_t Align>
template<typename... Args>
T* pool::constructor<T, Count, Align>::construct(Args&&... args) {
    T* ptr = type::allocate(std::forward<Args>(args)...);
    new(ptr) T(std::forward<Args>(args)...);
    return ptr;
}

template<typename T, size_t Count, size_t Align> void pool::constructor<T, Count, Align>::destruct(T* ptr) {
    ptr->~T();
    type::deallocate(ptr);
}

template<typename T, size_t Count, size_t Align> void pool::constructor<T, Count, Align>::cleanup() {
    type::cleanup();
}

auto pool::chunk::move() const -> chunk* {
    size_t   size = from->from->SIZE;
    uint8_t* temp = reinterpret_cast<uint8_t*>(const_cast<chunk*>(this));
    return reinterpret_cast<chunk*>(temp + size);
}

void* pool::chunk::object(chunk* ptr) {
    uint8_t* byte = reinterpret_cast<uint8_t*>(ptr);
    return byte + sizeof(void*);
}

auto pool::chunk::instance(void* ptr) -> chunk* {
    uint8_t* byte = reinterpret_cast<uint8_t*>(ptr);
    return reinterpret_cast<chunk*>(byte - sizeof(void*));
}

void pool::block::initialize(pool* parent, size_t count) {
    from = parent;
    next = nullptr;
    prev = nullptr;

    printf("%p\n", from);

    chunk* temp = reinterpret_cast<chunk*>(this + 1);
    curr        = temp;
    size_t loop = count - 1;

    for(size_t i = 0; i < loop; ++i) {
        temp->from = this;
        temp->next = temp->move();
        temp       = temp->next;
    }
    temp->from = this;
    temp->next = nullptr;
}

auto pool::block::get() -> chunk* {
    if(curr == nullptr) {
        return nullptr;
    }

    chunk* ptr = curr;
    curr       = curr->next;
    return ptr;
}

void pool::block::set(chunk* in) {
    if(!in) return;
    in->next = curr;
    curr     = in;
}

pool::pool(size_t chunk, size_t count, size_t align):
    SIZE(util::aligner::adjust((chunk + sizeof(void*)), util::aligner::boundary(align))),
    COUNT(util::aligner::adjust(count, sizeof(void*))),
    ALIGNMENT(util::aligner::boundary(align))
{}

pool::~pool() {
    for(auto i = all.begin(); i != all.end(); ++i) {
        _aligned_free(*i);
    }
}

auto pool::generate() -> block* {
    if(block* self = static_cast<block*>(_aligned_malloc(sizeof(block) + SIZE * COUNT, ALIGNMENT))) {
        self->initialize(this, COUNT);
        all.insert(self);
        return self;
    }
    return nullptr;
}

void* pool::allocate() {
    // check
    if(!top) {
        if(idle.size()) {
            idle.fifo(&top);
        } else {
            chunk* ptr;
            if(gc.try_pop(ptr)) {
                return ptr;
            }
        }
        top = generate();
    }
    if(!top) return nullptr;

    // return
    void* ptr = chunk::object(top->get());
    if(top->curr == nullptr) {
        if(top->next) {
            top             = top->next; // next
            top->prev->next = nullptr;   // unlink
            top->prev       = nullptr;   // unlink
        }

        else
            top = nullptr;
    }
    return ptr;
}

void pool::deallocate(void* in) {
    if(!in) return;

    chunk* ptr   = chunk::instance(in);
    block* child = ptr->from;
    pool*  self  = child->from;

    // if from this
    if(this == self) {
        free(ptr);
    }

    // to correct pool
    else
        self->gc.push(ptr);

    // from other pools
    while(gc.try_pop(ptr)) {
        free(ptr);
    }
}

void pool::cleanup() {
    chunk* garbage;
    while(gc.try_pop(garbage)) {
        garbage->from->set(garbage);
    }

    while(idle.size()) {
        block* ptr = nullptr;
        idle.fifo(&ptr);
        if(ptr) {
            all.erase(ptr);
            _aligned_free(ptr);
        }
    }
}

void pool::free(chunk* in) {
    block* parent = in->from;

    // empty -> usable
    if(parent->curr == nullptr) {
        if(top) {
            top->prev = parent;
        }
        parent->next = top;
        parent->prev = nullptr;
        top          = parent;
    }

    // insert
    parent->set(in);

    // using -> full
    if(parent->curr == static_cast<void*>(parent + 1)) {
        if(parent == top) {
            return; // keep
        }
        if(parent->next) {
            parent->next->prev = parent->prev;
            parent->next       = nullptr;
        }
        if(parent->prev) {
            parent->prev->next = parent->next;
            parent->prev       = nullptr;
        }
        idle.push(parent);
    }
}

void pool::release(void* in) {
    chunk* ptr    = static_cast<chunk*>(in);
    pool*  parent = ptr->from->from;
    parent->gc.push(ptr);
}

} // namespace memory
} // namespace lwe