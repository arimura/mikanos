/**
 * @file main.cpp
 *
 * カーネル本体のプログラムを書いたファイル．
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <array>
#include <deque>
#include <limits>
#include <numeric>
#include <vector>

#include "acpi.hpp"
#include "asmfunc.h"
#include "console.hpp"
#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "interrupt.hpp"
#include "keyboard.hpp"
#include "layer.hpp"
#include "logger.hpp"
#include "memory_manager.hpp"
#include "memory_map.hpp"
#include "message.hpp"
#include "mouse.hpp"
#include "paging.hpp"
#include "pci.hpp"
#include "segment.hpp"
#include "task.hpp"
#include "timer.hpp"
#include "usb/xhci/xhci.hpp"
#include "window.hpp"

int printk(const char* format, ...) {
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

std::shared_ptr<Window> main_window;
unsigned int main_window_layer_id;
void InitializeMainWindow() {
  main_window = std::make_shared<Window>(
      160, 52, screen_config.pixel_format);
  DrawWindow(*main_window->Writer(), "Hello Window");

  main_window_layer_id = layer_manager->NewLayer()
                             .SetWindow(main_window)
                             .SetDraggable(true)
                             .Move({ 300, 100 })
                             .ID();

  layer_manager->UpDown(main_window_layer_id, std::numeric_limits<int>::max());
}

std::shared_ptr<Window> text_window;
unsigned int text_window_layer_id;
void InitializeTextWindow() {
  const int win_w = 160;
  const int win_h = 52;

  text_window = std::make_shared<Window>(
      win_w, win_h, screen_config.pixel_format);
  DrawWindow(*text_window->Writer(), "Text Box Test");
  DrawTextbox(*text_window->Writer(), { 4, 24 }, { win_w - 8, win_h - 24 - 4 });

  text_window_layer_id = layer_manager->NewLayer()
                             .SetWindow(text_window)
                             .SetDraggable(true)
                             .Move({ 350, 200 })
                             .ID();
  layer_manager->UpDown(text_window_layer_id, std::numeric_limits<int>::max());
}

int text_window_index;

void DrawTextCursor(bool visible) {
  const auto color = visible ? ToColor(0) : ToColor(0xffffff);
  const auto pos = Vector2D<int> { 8 + 8 * text_window_index, 24 + 5 };
  FillRectangle(*text_window->Writer(), pos, { 7, 15 }, color);
}

void InputTextWindow(char c) {
  if (c == 0) {
    return;
  }

  auto pos = []() { return Vector2D<int> { 8 + 8 * text_window_index, 24 + 6 }; };

  const int max_chars = (text_window->Width() - 16) / 8 - 1;
  if (c == '\b' && text_window_index > 0) {
    DrawTextCursor(false);
    --text_window_index;
    FillRectangle(*text_window->Writer(), pos(), { 8, 16 }, ToColor(0xffffff));
    DrawTextCursor(true);
  } else if (c >= ' ' && text_window_index < max_chars) {
    DrawTextCursor(false);
    WriteAscii(*text_window->Writer(), pos(), c, ToColor(0));
    ++text_window_index;
    DrawTextCursor(true);
  }
  layer_manager->Draw(text_window_layer_id);
}

std::shared_ptr<Window> task_b_window;
unsigned int task_b_window_layer_id;
void InitializeTaskBWindow() {
  task_b_window = std::make_shared<Window>(
      160, 52, screen_config.pixel_format);
  DrawWindow(*task_b_window->Writer(), "TaskB Window");

  task_b_window_layer_id = layer_manager->NewLayer()
                               .SetWindow(task_b_window)
                               .SetDraggable(true)
                               .Move({ 100, 100 })
                               .ID();

  layer_manager->UpDown(task_b_window_layer_id, std::numeric_limits<int>::max());
}

void TaskB(uint64_t task_id, int64_t data) {
  printk("TaskB: task_id=%ld, data=%ld\n", task_id, data);
  char str[128];
  int count = 0;

  __asm__("cli");
  Task& task = task_manager->CurrentTask();
  __asm__("sti");

  while (true) {
    ++count;
    sprintf(str, "%010d", count);
    FillRectangle(*task_b_window->Writer(), { 24, 28 }, { 8 * 10, 16 }, { 0xc6, 0xc6, 0xc6 });
    WriteString(*task_b_window->Writer(), { 24, 28 }, str, { 0, 0, 0 });

    Message msg { Message::kLayer, task_id };
    msg.arg.layer.layer_id = task_b_window_layer_id;
    msg.arg.layer.op = LayerOperation::Draw;
    __asm__("cli");
    task_manager->SendMessage(1, msg);
    __asm__("sti");

    while (true) {
      __asm__("cli");
      auto msg = task.ReceiceMessage();
      if (!msg) {
        task.Sleep();
        __asm__("sti");
        continue;
      }

      if (msg->type == Message::kLayerFinish) {
        break;
      }
    }
  }
}

void TaskIdle(uint64_t task_id, int64_t data) {
  printk("TaskIdle: task_id=%lu, data=%lx\n", task_id, data);
  while (true)
    __asm__("hlt");
}

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(
    const FrameBufferConfig& frame_buffer_config_ref,
    const MemoryMap& memory_map_ref,
    const acpi::RSDP& acpi_table) {
  MemoryMap memory_map { memory_map_ref };

  InitializeGraphics(frame_buffer_config_ref);
  InitializeConsole();

  printk("Welcome to MikanOS!\n");
  SetLogLevel(kWarn);

  InitializeSegmentation();
  InitializePaging();
  InitializeMemoryManager(memory_map);
  InitializeInterrupt();

  InitializePCI();

  InitializeLayer();
  InitializeMainWindow();
  InitializeTextWindow();
  InitializeTaskBWindow();
  layer_manager->Draw({ { 0, 0 }, ScreenSize() });

  acpi::Initialize(acpi_table);
  InitializeLAPICTimer();

  const int kTextboxCursorTimer = 1;
  const int kTimer05Sec = static_cast<int>(kTimerFreq * 0.5);
  timer_manager->AddTimer(Timer { kTimer05Sec, kTextboxCursorTimer });
  bool textbox_cursor_visible = false;

  InitializeTask();
  Task& main_task = task_manager->CurrentTask();
  const uint64_t taskb_id = task_manager->NewTask()
                                .InitContext(TaskB, 45)
                                .Wakeup()
                                .ID();
  task_manager->NewTask().InitContext(TaskIdle, 0xdeadbeef).Wakeup();
  task_manager->NewTask().InitContext(TaskIdle, 0xcafebabe).Wakeup();

  usb::xhci::Initialize();
  InitializeKeyboard();
  InitializeMouse();

  char str[128];

  while (true) {
    __asm__("cli");
    const auto tick = timer_manager->CurrentTick();
    __asm__("sti");

    sprintf(str, "%010lu", tick);
    FillRectangle(*main_window->Writer(), { 24, 28 }, { 8 * 10, 16 }, { 0xc6, 0xc6, 0xc6 });
    WriteString(*main_window->Writer(), { 24, 28 }, str, { 0, 0, 0 });
    layer_manager->Draw(main_window_layer_id);

    __asm__("cli");
    auto msg = main_task.ReceiceMessage();
    if (!msg) {
      main_task.Sleep();
      __asm__("sti");
      continue;
    }

    __asm__("sti");

    switch (msg->type) {
    case Message::kInterruptXHCI:
      usb::xhci::ProcessEvents();
      break;
    case Message::kTimerTimeout:
      if (msg->arg.timer.value == kTextboxCursorTimer) {
        __asm__("cli");
        timer_manager->AddTimer(
            Timer { msg->arg.timer.timeout + kTimer05Sec, kTextboxCursorTimer });
        __asm__("sti");
        textbox_cursor_visible = !textbox_cursor_visible;
        DrawTextCursor(textbox_cursor_visible);
        layer_manager->Draw(text_window_layer_id);
      }
      break;
    case Message::kKeyPush:
      InputTextWindow(msg->arg.keyboard.ascii);
      if (msg->arg.keyboard.ascii == 's') {
        printk("sleep Task: %s\n", task_manager->Sleep(taskb_id).Name());
      } else if (msg->arg.keyboard.ascii == 'w') {
        printk("wakeup TaskB: %s\n", task_manager->Wakeup(taskb_id).Name());
      }
      break;
    case Message::kLayer:
      ProcessLayerMessage(*msg);
      __asm__("cli");
      task_manager->SendMessage(msg->src_task, Message { Message::kLayerFinish });
      __asm__("sti");
      break;
    default:
      Log(kError, "Unknown message type: %d\n", msg->type);
    }
  }
}

extern "C" void __cxa_pure_virtual() {
  while (1)
    __asm__("hlt");
}