#pragma once

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <new>
#include <optional>
#include <utility>

namespace edgesense {

// Minimum distance between two objects to avoid false sharing, as the compiler sees it for the
// architecture being compiled for (a compile-time value, not detected on the running machine).
#if defined(__cpp_lib_hardware_interference_size)
// GCC warns (-Winterference-size) about any use in a header: the value can change with
// -mcpu/-mtune, which changes class layout and so the ABI. EdgeSense is always built from source
// for its target, so a layout that follows the target is exactly what we want.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winterference-size"
#endif
inline constexpr std::size_t kCacheLine = std::hardware_destructive_interference_size;
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
#else
// Not provided by some standard libraries (e.g. libstdc++ when compiled with clang < 19).
inline constexpr std::size_t kCacheLine = 64;
#endif

template <class T, std::size_t N>
  requires std::default_initializable<T>
class SpscRing {
  static_assert(N >= 2, "one slot always stays empty, so N must be at least 2");

  std::array<T, N> buf_{};
  // Next slot to read; written only by the consumer.
  alignas(kCacheLine) std::atomic<std::size_t> head_{0};
  // Next slot to write; written only by the producer.
  alignas(kCacheLine) std::atomic<std::size_t> tail_{0};

  template <class U>
  bool push_impl(U&& v);

 public:
  // Producer thread only. Return false if the ring is full (v is left untouched).
  bool try_push(const T& v) { return push_impl(v); }
  bool try_push(T&& v) { return push_impl(std::move(v)); }

  // Consumer thread only. Returns nullopt if the ring is empty.
  std::optional<T> try_pop();
};

template <class T, std::size_t N>
  requires std::default_initializable<T>
template <class U>
bool SpscRing<T, N>::push_impl(U&& v) {
  const auto t = tail_.load(std::memory_order::relaxed);
  const auto next = (t + 1) % N;

  if (head_.load(std::memory_order::acquire) == next) return false;

  buf_[t] = std::forward<U>(v);
  tail_.store(next, std::memory_order::release);
  return true;
}

template <class T, std::size_t N>
  requires std::default_initializable<T>
std::optional<T> SpscRing<T, N>::try_pop() {
  auto h = head_.load(std::memory_order::relaxed);

  if (tail_.load(std::memory_order::acquire) == h) return std::nullopt;

  const auto next = (h + 1) % N;
  // Move out so the slot does not keep resources alive until it is overwritten.
  T v = std::move(buf_[h]);
  head_.store(next, std::memory_order::release);
  return v;
}

}  // namespace edgesense
