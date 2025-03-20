#pragma once

enum class DescriptoType {
  kUpper8Bytes = 0,
  kLDT = 2,
  kTSSAvailable = 9,
  kTSSBusy = 11,
  kCallGate = 12,
  kInterruptGate = 14,
  kTrapGate = 15,
  kReadWrite = 2,
  kExecuteRead = 10,
};