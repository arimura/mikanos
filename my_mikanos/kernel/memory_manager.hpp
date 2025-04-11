#pragma once

#include <array>
#include <limits>

#include "error.hpp"

namespace {
  constexpr unsigned long long operator""_KiB(unsigned long long kib) {
    return kib * 1024;
  }

  constexpr unsigned long long operator""_MiB(unsigned long long mib) {
    return mib * 1024_KiB;
  }

  constexpr unsigned long long operator""_Gib(unsigned long long gib) {
    return gib * 1024_MiB;
  }
}