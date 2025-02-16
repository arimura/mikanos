#pragma once
#include "error.hpp"

namespace usb {
  enum class EndpointType {
    kControl = 0,
    kIsochronous = 1,
    kBulk = 2,
    kInterrupt = 3,
  };

  class EndpointID {
   public:
    constexpr EndpointID()
        : addr_ { 0 } { }

   private:
    int addr_;
  };
}