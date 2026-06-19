#include <new>
#include <cerrno>
#include <cstddef>
#include <cstdlib>

void *operator new(std::size_t size) {
    return std::malloc(size);
}

void operator delete(void *obj) noexcept {
}

void operator delete(void *obj, unsigned long size) noexcept {
}

void operator delete(void *ptr, std::size_t size, std::align_val_t align) noexcept {
    std::free(ptr);
}

std::new_handler
std::get_new_handler() noexcept {
    return nullptr;
}

extern "C" int posix_memalign(void **, size_t, size_t) {
    return ENOMEM;
}
