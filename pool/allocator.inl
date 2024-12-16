namespace lwe {
namespace mem {

template<size_t Size, size_t Align, size_t Cache>
thread_local allocator<Size, Align, Cache> allocator<Size, Align, Cache>::singleton;

template<size_t Size, size_t Align, size_t Cache>
void* allocator<Size, Align, Cache>::uncaching::allocate(size_t in) noexcept {
    if(in) {
        return _aligned_malloc(Size * in, Align);
    }
    return nullptr;
}

template<size_t Size, size_t Align, size_t Cache>
void allocator<Size, Align, Cache>::uncaching::deallocate(void* in) noexcept {
    _aligned_free(in);
}

template<size_t Size, size_t Align, size_t Cache> allocator<Size, Align, Cache>::allocator() noexcept: count{ 0 } {}

template<size_t Size, size_t Align, size_t Cache> allocator<Size, Align, Cache>::~allocator() noexcept {
    for(size_t i = 0; i < count; ++i) {
        uncaching::deallocate(store[i]);
    }
}

template<size_t Size, size_t Align, size_t Cache> void* allocator<Size, Align, Cache>::get() const noexcept {
    if(count != 0) {
        return store[--count];
    }
    return uncaching::allocate(1);
}

template<size_t Size, size_t Align, size_t Cache> void allocator<Size, Align, Cache>::free(void* in) const noexcept {
    if(count >= Cache) {
        uncaching::deallocate(in);
    } else store[count++] = in;
}

template<size_t Size, size_t Align, size_t Cache> void* allocator<Size, Align, Cache>::allocate() noexcept {
    return singleton.get();
}

template<size_t Size, size_t Align, size_t Cache> void allocator<Size, Align, Cache>::deallocate(void* in) noexcept {
    singleton.free(in);
}

} // namespace mem
} // namespace lwe