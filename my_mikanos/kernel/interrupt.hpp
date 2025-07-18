#pragma once

#include "message.hpp"
#include "x86_descriptor.hpp"
#include <array>
#include <cstdint>
#include <deque>

union InterruptDescriptionAttribute {
  uint16_t data;
  struct {
    uint16_t interrupt_stack_table : 3;
    uint16_t : 5;
    DescriptorType type : 4;
    uint16_t : 1;
    uint16_t descriptor_privilege_level : 2;
    uint16_t present : 1;
  } __attribute__((packed)) bits;
} __attribute__((packed));

struct InterruptDescriptor {
  uint16_t offset_low;
  uint16_t segment_selector;
  InterruptDescriptionAttribute attr;
  uint16_t offset_middle;
  uint32_t offset_high;
  uint32_t reserved;
} __attribute__((packed));

extern std::array<InterruptDescriptor, 256> idt;

constexpr InterruptDescriptionAttribute MakeIDTAttr(
    DescriptorType type,
    uint8_t descriptor_privilege_level,
    bool present = true,
    uint8_t interrupt_stack_table = 0) {
  InterruptDescriptionAttribute attr {};
  attr.bits.interrupt_stack_table = interrupt_stack_table;
  attr.bits.type = type;
  attr.bits.descriptor_privilege_level = descriptor_privilege_level;
  attr.bits.present = present;
  return attr;
}

void SetIDTEntry(InterruptDescriptor& desc,
    InterruptDescriptionAttribute attr,
    uint64_t offset,
    uint16_t segment_selector);

class InterruptVector {
 public:
  enum Number {
    kXHCI = 0x40,
    kLAPICTimer = 0x41,
  };
};

struct InterruptFrame {
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
};

void NotifyEndOfInterrupt();

void InitializeInterrupt();
