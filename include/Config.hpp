#pragma once

#include <cstddef>
#include <memory>
#include "Order.hpp"
#include "ObjectPool.hpp"

#ifndef HFT_USE_RAW_PTR
#define HFT_USE_RAW_PTR 0
#endif
#ifndef HFT_ALIGN_CACHE
#define HFT_ALIGN_CACHE 1
#endif
#ifndef HFT_USE_POOL
#define HFT_USE_POOL 1
#endif
#ifndef HFT_BOOK_IMPL_FLAT
#define HFT_BOOK_IMPL_FLAT 1
#endif

namespace hft {

inline constexpr std::size_t kPoolCapacity = 1u << 16;
using OrderPool = ObjectPool<OrderT, kPoolCapacity>;

#if HFT_USE_POOL
inline std::shared_ptr<OrderT> make_pool_shared(OrderPool& pool) {
    OrderT* raw = pool.allocate();
    if (!raw) return nullptr;
    return std::shared_ptr<OrderT>(raw, PoolDeleter<OrderT>{&pool});
}
#else
inline std::shared_ptr<OrderT> make_pool_shared(OrderPool&) {
    return std::make_shared<OrderT>();
}
#endif

}
