#include "allocator.hh"
namespace lwe {
namespace mem {

template<size_t Size, size_t Align, size_t Cache>
thread_local allocator allocator::statics<Size, Align, Cache>::instance{ Size, Align, Cache };

void* allocator::malloc(size_t size, size_t align) noexcept {
    return _aligned_malloc(size, align);
}

void allocator::free(void* in) noexcept {
    _aligned_free(in);
}
    
allocator::allocator(size_t size, size_t align, size_t cache):
    SIZE{ util::aligner::adjust(size) }, ALIGNMENT{ util::aligner::boundary(align) }, CACHE{ cache } {
    stack = static_cast<void**>(malloc(CACHE * sizeof(void*), sizeof(void*)));
}

allocator::~allocator() noexcept {
    if(stack) {
        for(size_t i = 0; i < count; ++i) {
            free(stack[i]);
        }
        free(stack);
    }
}

void* allocator::allocate() noexcept {
    if(count) {
        if(stack) {
            return stack[--count];
        }
        return nullptr;
    }
    return malloc(SIZE, ALIGNMENT);
}

void allocator::deallocate(void* in) noexcept {
    if(stack && count < CACHE) {
        stack[count++] = in;
    } else free(in);
}

template<size_t Size, size_t Align, size_t Cache>
void* allocator::statics<Size, Align, Cache>::allocate() noexcept {
    return instance.allocate();
}

template<size_t Size, size_t Align, size_t Cache>
void allocator::statics<Size, Align, Cache>::deallocate(void* in) noexcept {
    instance.deallocate(in);
}

} // namespace mem
} // namespace lwe