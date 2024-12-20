namespace lwe {
namespace mem {

template<size_t Size, size_t Align, size_t Cache> allocator& allocator::statics() noexcept {
    static thread_local allocator instance(Size, Align, Cache);
    return instance;
}

void* allocator::malloc(size_t size, size_t align) noexcept {
    align = util::aligner::boundary(align);
    size  = util::aligner::padding(size, align);

#if _WIN32
    return _aligned_malloc(size, align);
#else
    return std::aligned_alloc(align, size);
#endif
}

void allocator::free(void* in) noexcept {
#ifdef _WIN32
    _aligned_free(in);
#else
    std::free(in);
#endif
}

// clang-format off
allocator::allocator(size_t size, size_t align, size_t cache) noexcept:
    ALIGN{ util::aligner::boundary(align) },
    SIZE{  util::aligner::padding(size, ALIGN) },
    CACHE{ cache },
    stack(nullptr)
{}
// clanf-format on
    
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
    return malloc(SIZE, ALIGN);
}

void allocator::deallocate(void* in) noexcept {
    if(stack && count < CACHE) {
        stack[count++] = in;
    } else free(in);
}

} // namespace mem
} // namespace lwe