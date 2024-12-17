#include "pool.hh"

namespace lwe {
namespace mem {

template<size_t Size, size_t Count, size_t Align>
thread_local pool pool::statics<Size, Count, Align>::singleton{ Size, Count, Align };

struct pool::block {
    void  initialize(pool*, size_t, size_t);
    void* get();
    void  set(void*);

    pool*    from;
    uint8_t* curr;
    block*   next;
    block*   prev;
};

template<size_t Size, size_t Count, size_t Align> inline void* pool::statics<Size, Count, Align>::allocate() {
    return singleton.construct();
}

template<size_t Size, size_t Count, size_t Align> inline void pool::statics<Size, Count, Align>::deallocate(void* in) {
    singleton.destruct(in);
}

template<size_t Size, size_t Count, size_t Align> inline void pool::statics<Size, Count, Align>::cleanup() {
    singleton.cleanup();
}

void pool::block::initialize(pool* parent, size_t count, size_t alignment) {
    from = parent;
    next = nullptr;
    prev = nullptr;

    uint8_t* cursor = reinterpret_cast<uint8_t*>(this);

    cursor += alignment;     // move
    cursor -= sizeof(void*); // start position;
    curr    = cursor;        // save

    // write parent end next
    size_t loop = count - 1;
    for(size_t i = 0; i < loop; ++i) {
        uint8_t* next                                     = cursor + parent->SIZE;
        *reinterpret_cast<void**>(cursor)                 = reinterpret_cast<void*>(this);
        *reinterpret_cast<void**>(cursor + sizeof(void*)) = reinterpret_cast<void*>(next);
        cursor                                            = next;
    }
    *reinterpret_cast<void**>(cursor)                 = reinterpret_cast<void*>(this);
    *reinterpret_cast<void**>(cursor + sizeof(void*)) = nullptr;
}

void* pool::block::get() {
    if(curr == nullptr) {
        return nullptr;
    }

    void* ptr = static_cast<void*>(curr);

    // set next
    // block: [[meta][next] ... ]
    curr = reinterpret_cast<uint8_t*>(*(reinterpret_cast<void**>(curr) + 1));
    return ptr;
}

void pool::block::set(void* in) {
    if(!in) return;

    // set next
    // block: [[meta][next] ... ]
    *(reinterpret_cast<uintptr_t*>(in) + 1) = reinterpret_cast<uintptr_t>(curr);

    curr = reinterpret_cast<uint8_t*>(in);
}

// clang-format off
pool::pool(size_t chunk, size_t count, size_t align):
    ALIGNMENT(util::aligner::boundary(align)),
    SIZE(     util::aligner::adjust  (chunk + sizeof(void*), align)),
    COUNT(    util::aligner::adjust  (count, config::DEF_CACHE)),

    // unit variable block + ptr : for chunk metadata(void*) pulling
    // e.g.
    // - alignment 16, block 56 => 64,  8 byte can pull
    // - alignment 16, block 64 => 80,  16 byte can pull
    // - alignment 64, block 56 => 64,  8 byte can pull
    // - alignment 64, block 64 => 128, 64 byte can pull
    // - alignment 8,  block 56 => 64: becauses minus sizeof(void*), and adjust to alignemnt
    ALLOCTATE {
        util::aligner::adjust(
            util::aligner::adjust(sizeof(block) + sizeof(void*), ALIGNMENT)
                + (SIZE * COUNT) - sizeof(void*),
            ALIGNMENT
        )
    }
{}
//clang-format on

pool::~pool() {
    for(auto i = all.begin(); i != all.end(); ++i) {
        allocator::free(*i);
    }
}

auto pool::setup() -> block* {
    if(block* self = static_cast<block*>(_aligned_malloc(ALLOCTATE, ALIGNMENT))) {
        self->initialize(this, COUNT, ALIGNMENT);
        all.insert(self);
        return self;
    }
    return nullptr;
}

template<typename T, typename... Args> inline T* pool::construct(Args&&... args) noexcept {
    void* ptr = nullptr;
    
    // check has block
    if(!top) {
        if(idle.size()) {
            idle.fifo(&top);
        }
        
        // check has released chunk
        else if (gc.try_pop(ptr) == false) {
            top = setup();
            if (!top) {
                return nullptr; // failed
            }
        }
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
    T* ret = reinterpret_cast<T*>(reinterpret_cast<uintptr_t*>(ptr) + 1);
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

    void*  ptr   = reinterpret_cast<void*>(reinterpret_cast<uintptr_t*>(in) - 1);
    block* child = *reinterpret_cast<block**>(ptr);
    pool*  self  = child->from;

    // if from this
    if(this == self) {
        release(ptr);
    }

    // to correct pool
    else self->gc.push(ptr);

    // from other pools
    while(gc.try_pop(ptr)) {
        release(ptr);
    }
}

void pool::cleanup() {
    void* garbage;
    while(gc.try_pop(garbage)) {
        (*reinterpret_cast<block**>(garbage))->set(garbage);
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

void pool::release(void* in) {
    block* parent = *reinterpret_cast<block**>(in);

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

void pool::revert(void* in) noexcept {
    pool*  parent = (*reinterpret_cast<block**>(in))->from;
    parent->gc.push(in);
}

} // namespace mem
} // namespace lwe