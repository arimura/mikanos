#pragma once

#include <array>
#include <cstddef>

#include "error.hpp"

template <typename T>
class ArrayQueue {
 public:
  template <size_t N>
  ArrayQueue(std::array<T, N>& buf);
  ArrayQueue(T* buf, size_t size);
  Error Push(const T& value);
  Error Pop();
  size_t Count() const;
  size_t Capacity() const;
  const T& Front() const;
};