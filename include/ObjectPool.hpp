#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>
#include <vector>

template <typename T>
class ObjectPoolBase {
public:
    virtual void deallocate_raw(T* p) noexcept = 0;
protected:
    ~ObjectPoolBase() = default;
};

template <typename T, std::size_t N>
class ObjectPool : public ObjectPoolBase<T> {
public:
    ObjectPool() {
        free_list_.reserve(N);
        for (std::size_t i = N; i-- > 0; ) {
            free_list_.push_back(slot_ptr(i));
        }
    }

    ~ObjectPool() = default;

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    template <typename... Args>
    T* allocate(Args&&... args) {
        if (free_list_.empty()) return nullptr;
        T* slot = free_list_.back();
        free_list_.pop_back();
        return ::new (slot) T(std::forward<Args>(args)...);
    }

    void deallocate(T* p) noexcept {
        if (!p) return;
        p->~T();
        free_list_.push_back(p);
    }

    void deallocate_raw(T* p) noexcept override { deallocate(p); }

    std::size_t available() const noexcept { return free_list_.size(); }
    static constexpr std::size_t capacity() noexcept { return N; }

private:
    alignas(T) std::array<std::byte, sizeof(T) * N> storage_{};
    std::vector<T*> free_list_;

    T* slot_ptr(std::size_t i) noexcept {
        return reinterpret_cast<T*>(storage_.data() + i * sizeof(T));
    }
};

template <typename T>
struct PoolDeleter {
    ObjectPoolBase<T>* pool = nullptr;
    void operator()(T* p) const noexcept {
        if (p && pool) pool->deallocate_raw(p);
    }
};
