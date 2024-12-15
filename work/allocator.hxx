namespace lwe { namespace mem {

template<size_t Size, size_t Align>
thread_local typename allocator<Size, Align>::template object<config::DEF_COUNT> allocator<Size, Align>::instance;

template<size_t Size, size_t Align>
template<size_t Cache>
allocator<Size, Align>::object<Cache>::object() noexcept: count{ 0 } {}

template<size_t Size, size_t Align> template<size_t Cache> allocator<Size, Align>::object<Cache>::~object() {
    for(size_t i = 0; i < count; i++) {
        _aligned_free(store[i]);
    }
}

template<size_t Size, size_t Align>
template<size_t Cache>
void* allocator<Size, Align>::object<Cache>::allocate() const {
    if(count > 0) {
        return store[count--];
    }
    return _aligned_malloc(Size, Align);
}

template<size_t Size, size_t Align>
template<size_t Cache>
void allocator<Size, Align>::object<Cache>::deallocate(void* ptr) const {
    if(count >= Cache) {
        _aligned_free(ptr);
    } else store[count++] = ptr;
}

template<size_t Size, size_t Align> void* allocator<Size, Align>::allocate(size_t size) noexcept {
    return instance.allocate(size);
}

template<size_t Size, size_t Align> void allocator<Size, Align>::deallocate(void* ptr) noexcept {
    instance.deallocate(ptr);
}


}}