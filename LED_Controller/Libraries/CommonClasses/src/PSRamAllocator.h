#include <cstdlib>
#include <new>
#include <iostream>

template <typename T>
struct PSRAMAllocator {
    using value_type = T;

    // Constructor
    PSRAMAllocator() noexcept {}

    // Allocate memory for n elements
    T* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        // Use ps_malloc to allocate memory from PSRAM
        T* ptr = (T*)ps_malloc(n * sizeof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    // Deallocate memory
    void deallocate(T* p, std::size_t n) noexcept {
        // Use ps_free to free memory
        free(p);
    }

    // Other functions required for C++ allocator (not typically needed for simple use)
    template <typename U>
    struct rebind {
        using other = PSRAMAllocator<U>;
    };
};