# MikanOS Class Diagram

## Overview
This document contains UML class diagrams for the MikanOS project classes implemented in `./my_mikanos`.

## Core System Classes

```mermaid
classDiagram
    %% Graphics and GUI Layer
    class PixelColor {
        +uint8_t r
        +uint8_t g
        +uint8_t b
    }
    
    class Vector2D~T~ {
        +T x
        +T y
        +Vector2D~T~ operator+(Vector2D~T~)
        +Vector2D~T~ operator-(Vector2D~T~)
    }
    
    class Rectangle~T~ {
        +Vector2D~T~ pos
        +Vector2D~T~ size
        +T Width()
        +T Height()
    }
    
    class PixelWriter {
        <<abstract>>
        +Write(Vector2D~int~ pos, PixelColor& c)*
        +Width()*
        +Height()*
    }
    
    class FrameBufferWriter {
        +Write(Vector2D~int~ pos, PixelColor& c)
        +Width()
        +Height()
    }
    
    class RGBResv8BitPerColorPixelWriter {
        +Write(Vector2D~int~ pos, PixelColor& c)
    }
    
    class BGRResv8BitPerColorPixelWriter {
        +Write(Vector2D~int~ pos, PixelColor& c)
    }
    
    class FrameBuffer {
        +FrameBuffer(Vector2D~int~ size, PixelFormat format)
        +Vector2D~int~ Size()
        +PixelWriter& Writer()
        +Move(Vector2D~int~ dst_pos, Rectangle~int~& src)
        +Copy(Vector2D~int~ dst_pos, FrameBuffer& src, Rectangle~int~& src_area)
        -Vector2D~int~ size_
        -PixelFormat pixel_format_
        -std::vector~uint8_t~ buffer_
        -std::unique_ptr~PixelWriter~ writer_
    }
    
    PixelWriter <|-- FrameBufferWriter
    FrameBufferWriter <|-- RGBResv8BitPerColorPixelWriter
    FrameBufferWriter <|-- BGRResv8BitPerColorPixelWriter
    FrameBuffer --> PixelWriter
    
    %% Window System
    class Window {
        +Window(int width, int height, PixelFormat shadow_format)
        +DrawTo(FrameBuffer& dst, Vector2D~int~ pos, Rectangle~int~& area)
        +SetTransparentColor(optional~PixelColor~ c)
        +WindowWriter* Writer()
        +At(Vector2D~int~ pos) PixelColor&
        +Write(Vector2D~int~ pos, PixelColor c)
        +Width() int
        +Height() int
        +Size() Vector2D~int~
        +Move(Vector2D~int~ dst_pos, Rectangle~int~& src)
        -int width_
        -int height_
        -vector~vector~PixelColor~~ data_
        -WindowWriter writer_
        -optional~PixelColor~ transparent_color_
        -FrameBuffer shadow_buffer_
    }
    
    class WindowWriter {
        +WindowWriter(Window& window)
        +Write(Vector2D~int~ pos, PixelColor& c)
        +Width() int
        +Height() int
        -Window& window_
    }
    
    Window *-- WindowWriter
    WindowWriter --|> PixelWriter
    Window --> FrameBuffer
    
    %% Layer Management
    class Layer {
        +Layer(unsigned int id)
        +unsigned int ID()
        +SetWindow(shared_ptr~Window~& window) Layer&
        +GetWindow() shared_ptr~Window~
        +GetPosition() Vector2D~int~
        +SetDraggable(bool draggable) Layer&
        +IsDraggable() bool
        +Move(Vector2D~int~ pos) Layer&
        +MoveRelative(Vector2D~int~ pos_diff) Layer&
        +DrawTo(FrameBuffer& screen, Rectangle~int~& area)
        -unsigned int id_
        -Vector2D~int~ pos_
        -shared_ptr~Window~ window_
        -bool draggable_
    }
    
    class LayerManager {
        +SetWriter(FrameBuffer* screen)
        +NewLayer() Layer&
        +Draw(Rectangle~int~& area)
        +Draw(unsigned int id)
        +Move(unsigned int id, Vector2D~int~ new_pos)
        +MoveRelative(unsigned int id, Vector2D~int~ pos_diff)
        +UpDown(unsigned int id, int new_height)
        +Hide(unsigned int id)
        +FindLayerByPosition(Vector2D~int~ pos, unsigned int exclude_id) Layer*
        -FrameBuffer* screen_
        -FrameBuffer back_buffer_
        -vector~unique_ptr~Layer~~ layers_
        -vector~Layer*~ layer_stack_
        -unsigned int latest_id_
        -FindLayer(unsigned int id) Layer*
    }
    
    LayerManager --> FrameBuffer
    LayerManager --> Layer
    Layer --> Window
```

## System Management Classes

```mermaid
classDiagram
    %% Memory Management
    class FrameID {
        +FrameID(size_t id)
        +ID() size_t
        +Frame() void*
        -size_t id_
    }
    
    class BitmapMemoryManager {
        +static kMaxPhysicalMemoryBytes
        +static kFrameCount
        +static kBitsPerMapLine
        +Allocate(size_t num_frames) FrameID
        +Free(FrameID start_frame, size_t num_frames) Error
        +MarkAllocated(FrameID start_frame, size_t num_frames)
        -SetBit(FrameID frame, bool allocated)
        -GetBit(FrameID frame) bool
    }
    
    %% Task Management
    class TaskContext {
        <<struct>>
        +uint64_t cr3
        +uint64_t rip
        +uint64_t rflags
        +uint64_t cs, ss, fs, gs
        +uint64_t rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp
        +uint64_t r8, r9, r10, r11, r12, r13, r14, r15
        +array~uint8_t, 512~ fxsave_area
    }
    
    class Task {
        +static kDefaultLevel
        +static kDefalultStackBytes
        +Task(uint64_t id)
        +InitContext(TaskFunc* f, int64_t data) Task&
        +Context() TaskContext&
        +ID() uint64_t
        +Sleep() Task&
        +Wakeup() Task&
        +SendMessage(Message& msg)
        +ReceiceMessage() optional~Message~
        +Level() int
        +Running() bool
        -uint64_t id_
        -vector~uint64_t~ stack_
        -TaskContext context_
        -deque~Message~ msgs_
        -unsigned int level_
        -bool running_
        -SetLevel(int level) Task&
        -SetRunning(bool running) Task&
    }
    
    class TaskManager {
        +static kMaxLevel
        +TaskManager()
        +NewTask() Task&
        +SwitchTask(bool current_sleep)
        +Sleep(Task* task)
        +Sleep(uint64_t id) Error
        +Wakeup(Task* task, int level)
        +Wakeup(uint64_t id, int level) Error
        +SendMessage(uint64_t id, Message& msg) Error
        +CurrentTask() Task&
        -vector~unique_ptr~Task~~ tasks_
        -uint64_t latest_id_
        -array~deque~Task*~, kMaxLevel + 1~ running_
        -int current_level_
        -bool level_changed_
        -ChangeLevelRunning(Task* task, int level)
    }
    
    Task --> TaskContext
    TaskManager --> Task
    
    %% Timer Management
    class Timer {
        +Timer(unsigned long timeout, int value)
        +Timeout() unsigned long
        +Value() int
        -unsigned long timeout_
        -int value_
    }
    
    class TimerManager {
        +TimerManager()
        +AddTimer(Timer& timer)
        +Tick() bool
        +CurrentTick() unsigned long
        -volatile unsigned long tick_
        -priority_queue~Timer~ timers_
    }
    
    TimerManager --> Timer
    
    %% Console
    class Console {
        +Console(PixelWriter& writer, PixelColor fg_color, PixelColor bg_color)
        +PutString(char* s)
        +SetWriter(PixelWriter* writer)
        +SetWindow(shared_ptr~Window~& window)
        -PixelWriter* writer_
        -PixelColor fg_color_
        -PixelColor bg_color_
        -shared_ptr~Window~ window_
        -Vector2D~int~ cursor_
        -int rows_
        -int columns_
        -Newline()
        -Refresh()
    }
    
    Console --> PixelWriter
    Console --> Window
    
    %% Error Handling
    class Error {
        +Error(Code code, char* file, int line)
        +operator bool()
        +Code Cause()
        +File() char*
        +Line() int
        -Code code_
        -int line_
        -char* file_
    }
    
    class WithError~T~ {
        <<struct>>
        +T value
        +Error error
    }
    
    WithError --> Error
```

## Message System and Hardware

```mermaid
classDiagram
    %% Message System
    enum LayerOperation {
        Move
        MoveRelative
        Draw
    }
    
    class Message {
        <<struct>>
        +enum Type type
        +uint64_t src_task
        +union arg
    }
    
    class Mouse {
        +Mouse(unsigned int layer_id)
        +SetPosition(Vector2D~int~ position)
        +OnInterrupt(uint8_t buttons, int8_t displacement_x, int8_t displacement_y)
        -unsigned int layer_id_
        -Vector2D~int~ position_
        -unsigned int drag_layer_id_
        -Vector2D~int~ previous_drag_pos_
    }
    
    Message --> LayerOperation
    
    %% PCI System
    namespace pci {
        class ClassCode {
            <<struct>>
            +uint8_t base
            +uint8_t sub
            +uint8_t interface
            +Match(uint8_t b) bool
            +Match(uint8_t b, uint8_t s) bool
            +Match(uint8_t b, uint8_t s, uint8_t i) bool
        }
        
        class Device {
            <<struct>>
            +uint8_t bus
            +uint8_t device
            +uint8_t function
            +uint8_t header_type
            +ClassCode class_code
        }
        
        class MSICapability {
            <<struct>>
            +union header
            +uint32_t msg_addr
            +uint32_t msg_upper_addr
            +uint32_t msg_data
            +uint32_t mask_bits
            +uint32_t pending_bits
        }
        
        Device --> ClassCode
    }
    
    %% Interrupt System
    class InterruptDescriptor {
        <<struct>>
        +uint16_t offset_low
        +uint16_t segment_selector
        +uint16_t attr
        +uint16_t offset_middle
        +uint32_t offset_high
        +uint32_t reserved
    }
    
    class InterruptVector {
        +enum Number
    }
    
    class InterruptFrame {
        <<struct>>
        +uint64_t rip
        +uint64_t cs
        +uint64_t rflags
        +uint64_t rsp
        +uint64_t ss
    }
```

## USB System Classes

```mermaid
classDiagram
    %% USB Base Classes
    namespace usb {
        class Device {
            <<abstract>>
            +~Device()
            +ControlIn(EndpointID ep_id, SetupData setup_data, void* buf, int len, ClassDriver* issuer)* Error
            +ControlOut(EndpointID ep_id, SetupData setup_data, void* buf, int len, ClassDriver* issuer)* Error
            +InterruptIn(EndpointID ep_id, void* buf, int len)* Error
            +InterruptOut(EndpointID ep_id, void* buf, int len)* Error
            +StartInitialize() Error
            +IsInitialized() bool
            +EndpointConfigs() EndpointConfig*
            +NumEndpointConfigs() int
            +OnEndpointsConfigured() Error
            +Buffer() uint8_t*
            #OnControlCompleted(EndpointID ep_id, SetupData setup_data, void* buf, int len) Error
            #OnInterruptCompleted(EndpointID ep_id, void* buf, int len) Error
            -array~ClassDriver*, 16~ class_drivers_
            -array~uint8_t, 256~ buf_
        }
        
        class ClassDriver {
            <<abstract>>
            +ClassDriver(Device* dev)
            +~ClassDriver()
            +Initialize()* Error
            +SetEndpoint(EndpointConfig& config)* Error
            +OnEndpointsConfigured()* Error
            +OnControlCompleted(EndpointID ep_id, SetupData setup_data, void* buf, int len)* Error
            +OnInterruptCompleted(EndpointID ep_id, void* buf, int len)* Error
            +ParentDevice() Device*
            #Device* dev_
        }
        
        class HIDBaseDriver {
            +HIDBaseDriver(Device* dev, int interface_index)
            +~HIDBaseDriver()
            +Initialize() Error
            +SetEndpoint(EndpointConfig& config) Error
            +OnEndpointsConfigured() Error
            +OnControlCompleted(EndpointID ep_id, SetupData setup_data, void* buf, int len) Error
            +OnInterruptCompleted(EndpointID ep_id, void* buf, int len) Error
            #static kBufferSize
            #int interface_index_
            #EndpointID ep_interrupt_in_
            #EndpointID ep_interrupt_out_
            #bool initialize_phase_
            #array~uint8_t, kBufferSize~ buf_
            #array~uint8_t, kBufferSize~ previous_buf_
        }
        
        class HIDMouseDriver {
            +HIDMouseDriver(Device* dev, int interface_index)
            +OnInterruptCompleted(EndpointID ep_id, void* buf, int len) Error
            -int8_t displacement_x_
            -int8_t displacement_y_
            -uint8_t previous_buttons_
        }
        
        class HIDKeyboardDriver {
            +HIDKeyboardDriver(Device* dev, int interface_index)
            +OnInterruptCompleted(EndpointID ep_id, void* buf, int len) Error
        }
        
        Device --> ClassDriver
        ClassDriver <|-- HIDBaseDriver
        HIDBaseDriver <|-- HIDMouseDriver
        HIDBaseDriver <|-- HIDKeyboardDriver
        
        %% USB Descriptors
        class DeviceDescriptor {
            <<struct>>
            +uint8_t length
            +uint8_t descriptor_type
            +uint16_t usb_release
            +uint8_t device_class
            +uint8_t device_sub_class
            +uint8_t device_protocol
            +uint8_t max_packet_size
            +uint16_t vendor_id
            +uint16_t product_id
            +uint16_t device_release
            +uint8_t manufacturer
            +uint8_t product
            +uint8_t serial_number
            +uint8_t num_configurations
        }
        
        class ConfigurationDescriptor {
            <<struct>>
            +uint8_t length
            +uint8_t descriptor_type
            +uint16_t total_length
            +uint8_t num_interfaces
            +uint8_t configuration_value
            +uint8_t configuration_id
            +uint8_t attributes
            +uint8_t max_power
        }
        
        class InterfaceDescriptor {
            <<struct>>
            +uint8_t length
            +uint8_t descriptor_type
            +uint8_t interface_number
            +uint8_t alternate_setting
            +uint8_t num_endpoints
            +uint8_t interface_class
            +uint8_t interface_sub_class
            +uint8_t interface_protocol
            +uint8_t interface_id
        }
        
        class EndpointDescriptor {
            <<struct>>
            +uint8_t length
            +uint8_t descriptor_type
            +uint8_t endpoint_address
            +uint8_t attributes
            +uint16_t max_packet_size
            +uint8_t interval
        }
        
        class HIDDescriptor {
            <<struct>>
            +uint8_t length
            +uint8_t descriptor_type
            +uint16_t hid_release
            +uint8_t country_code
            +uint8_t num_descriptors
            +ClassDescriptor* GetClassDescriptor(size_t index)
        }
        
        class ClassDescriptor {
            <<struct>>
            +uint8_t descriptor_type
            +uint16_t descriptor_length
        }
        
        HIDDescriptor --> ClassDescriptor
    }
```

## System Initialization and Configuration

```mermaid
classDiagram
    %% Memory Map Structures
    class MemoryMap {
        <<struct>>
        +unsigned long long buffer_size
        +void* buffer
        +unsigned long long map_size
        +unsigned long long map_key
        +unsigned long long descriptor_size
        +uint32_t descriptor_version
    }
    
    class MemoryDescriptor {
        <<struct>>
        +uint32_t type
        +uintptr_t physical_start
        +uintptr_t virtual_start
        +uint64_t number_of_pages
        +uint64_t attribute
    }
    
    enum MemoryType {
        kEfiReservedMemoryType
        kEfiLoaderCode
        kEfiLoaderData
        kEfiBootServicesCode
        kEfiBootServicesData
        kEfiRuntimeServicesCode
        kEfiRuntimeServicesData
        kEfiConventionalMemory
        kEfiUnusableMemory
        kEfiACPIreclaimMemory
        kEfiACPMemoryNVS
        kEfiMemoryMappedIO
        kEfiMemoryMappedIOPortSpace
        kEfiPalCode
        kEfiPersitentMemory
        kEfiMaxMemoryType
    }
    
    MemoryMap --> MemoryDescriptor
    MemoryDescriptor --> MemoryType
    
    %% Frame Buffer Configuration
    class FrameBufferConfig {
        <<struct>>
        +uint8_t* frame_buffer
        +uint32_t pixels_per_scan_line
        +uint32_t horizontal_resolution
        +uint32_t vertical_resolution
        +PixelFormat pixel_format
    }
    
    %% ACPI System
    namespace acpi {
        class RSDP {
            <<struct>>
            +char signature[8]
            +uint8_t checksum
            +char oem_id[6]
            +uint8_t revision
            +uint32_t rsdt_address
            +uint32_t length
            +uint64_t xsdt_address
            +uint8_t extended_checksum
            +char reserved[3]
        }
        
        class DescriptionHeader {
            <<struct>>
            +char signature[4]
            +uint32_t length
            +uint8_t revision
            +uint8_t checksum
            +char oem_id[6]
            +char oem_table_id[8]
            +uint32_t oem_revision
            +uint32_t creator_id
            +uint32_t creator_revision
        }
        
        class XSDT {
            <<struct>>
            +DescriptionHeader header
            +uint64_t entries[]
        }
        
        class FADT {
            <<struct>>
            +DescriptionHeader header
            +char reserved1[76 - sizeof(header)]
            +uint32_t pm_tmr_blk
            +char reserved2[112 - 80]
            +uint32_t flags
            +char reserved3[276 - 116]
        }
        
        XSDT --> DescriptionHeader
        FADT --> DescriptionHeader
    }
```

This UML class diagram shows the complete class structure of the MikanOS project, organized into logical groups:

1. **Core System Classes**: Graphics, window management, and layer system
2. **System Management Classes**: Memory, task, timer, console, and error handling
3. **Message System and Hardware**: Message passing, mouse handling, PCI devices, and interrupts
4. **USB System Classes**: USB device management, class drivers, and descriptors
5. **System Initialization and Configuration**: Memory mapping, frame buffer, and ACPI structures

The diagrams show the inheritance relationships, composition relationships, and the overall architecture of the MikanOS system.