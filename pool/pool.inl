#include "pool.hh"

namespace lwe {
namespace mem {

struct pool::block {
    void  initialize(pool*, size_t) noexcept;
    void* get() noexcept;
    void  set(void*) noexcept;

    static block* find(void*) noexcept;

    pool*  from;
    void*  curr;
    block* next;
    block* prev;
};

void pool::block::initialize(pool* parent, size_t count) noexcept {
    from = parent;
    next = nullptr;
    prev = nullptr;

    uint8_t* data = reinterpret_cast<uint8_t*>(this) + from->BLOCK; // pass header
    uint8_t* meta = data - sizeof(void*);                           // pass pointer

    curr = reinterpret_cast<void*>(data); // save

    // write parent end next
    size_t loop = count - 1;
    for(size_t i = 0; i < loop; ++i) {
        *reinterpret_cast<void**>(meta) = reinterpret_cast<void*>(this);               // parent
        *reinterpret_cast<void**>(data) = reinterpret_cast<void*>(data + from->CHUNK); // next

        meta += from->CHUNK;
        data += from->CHUNK;
    }

    *reinterpret_cast<void**>(meta) = reinterpret_cast<void*>(this);
    *reinterpret_cast<void**>(data) = nullptr;
}

void* pool::block::get() noexcept {
    if(curr == nullptr) {
        return nullptr;
    }

    // get current
    void* out = curr;

    // set next
    curr = *reinterpret_cast<void**>(curr);

    return out;
}

void pool::block::set(void* in) noexcept {
    if(!in) return;

    // set next
    *(reinterpret_cast<void**>(in)) = curr;

    curr = in;
}

auto pool::block::find(void* in) noexcept -> block* {
    void* ptr = reinterpret_cast<void**>(in) - 1;
    return *reinterpret_cast<block**>(ptr);
}

// clang-format off
pool::pool(size_t chunk, size_t align, size_t count) noexcept :
    ALIGN{ util::aligner::boundary(align) },
    BLOCK{ util::aligner::padding(sizeof(block) + sizeof(void*), ALIGN) },
    CHUNK{ util::aligner::padding(chunk + sizeof(void*), ALIGN) },
    COUNT{ util::aligner::padding(count, config::DEF_CACHE) },
    TOTAL{ BLOCK + (CHUNK * COUNT) }
{}
//clang-format on

pool::~pool() noexcept {
    for(auto i = all.begin(); i != all.end(); ++i) {
        allocator::free(*i);
    }
}

auto pool::setup() noexcept->block* {
    if(block* self = static_cast<block*>(allocator::malloc(TOTAL, ALIGN))) {
        self->initialize(this, COUNT);
        all.insert(self);
        return self;
    }
    return nullptr;
}

template<size_t Size, size_t Align, size_t Count> pool& pool::statics() {
    static thread_local pool instance(Size, Align, Count);
    return instance;
}

template<typename T, typename... Args> inline T* pool::construct(Args&&... args) noexcept {
    void* ptr = nullptr;
    
    // check has block
    if(!top) {
        if(idle.size()) {
            idle.fifo(&top);
        }
        
        // check has chunk in garbage collector
        else if (gc.try_dequeue(ptr) == false) {
            top = setup();
        }
    }

    // EXCEPTION
    if (!top && !ptr) {
        return nullptr;
    }

    // if got => pass
    if (ptr == nullptr) {
        ptr = top->get();
        if(top->curr == nullptr) {
            if(top->next) {
                top             = top->next; // next
                top->prev->next = nullptr;   // unlink
                top->prev       = nullptr;   // unlink
            }

            // not leak
            else top = nullptr;
        }
    }

    // return
    T* ret = reinterpret_cast<T*>(ptr);
    if constexpr(!std::is_pointer_v<T> && !std::is_same_v<T, void>) {
        new(ret) T({ std::forward<Args>(args)... }); // new
    }
    return ret;
}

template<typename T>
void pool::destruct(T* in) noexcept {
    if(!in) return;

    // delete
    if constexpr(!std::is_pointer_v<T> && !std::is_void_v<T>) {
        in->~T();
    }

    // if from this
    pool* self = block::find(in)->from;
    if(this == self) {
        recycle(in);
    }

    // to correct pool
    else self->gc.enqueue(in);

    // from other pools
    while(gc.try_dequeue(in)) {
        recycle(in);
    }
}

void pool::cleanup() noexcept {
    void* garbage;
    while(gc.try_dequeue(garbage)) {
        block::find(garbage)->set(garbage); // child blocks
        (*reinterpret_cast<block**>(reinterpret_cast<void**>(garbage) - 1))->set(garbage);
    }

    while(idle.size()) {
        block* ptr = nullptr;
        idle.fifo(&ptr);
        if(ptr) {
            all.erase(ptr);
            allocator::free(ptr);
        }
    }
}

void pool::recycle(void* in) noexcept {
    block* parent = block::find(in);

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

template<typename T> void pool::release(T* in) noexcept {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(in) - sizeof(void*);

    pool* parent = (*reinterpret_cast<block**>(ptr))->from;
    // delete
    if constexpr(!std::is_pointer_v<T> && !std::is_void_v<T>) {
        in->~T();
    }
    parent->gc.enqueue(in); // lock-free
}

} // namespace mem
} // namespace lwe