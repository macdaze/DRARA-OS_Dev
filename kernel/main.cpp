#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <numeric>
#include <vector>

#include "frame_buffer_config.hpp"
#include "memory_map.hpp"
#include "graphics.hpp"
#include "mouse.hpp"
#include "pci.hpp"
#include "logger.hpp"
#include "error.hpp"
#include "usb/memory.hpp"
#include "usb/device.hpp"
#include "usb/classdriver/mouse.hpp"
#include "usb/xhci/xhci.hpp"
#include "usb/xhci/trb.hpp"
#include "font.hpp"
#include "console.hpp"
#include "segment.hpp"
#include "paging.hpp"
#include "interrupt.hpp"
#include "asmfunc.h"

/*
void* operator new (size_t size, void* buf) noexcept {
     return buf;
}

void operator delete(void* obj) noexcept {
}

void operator delete(void* obj, unsigned long size) noexcept {
}
*/

const PixelColor kDesktopBGColor{70, 100, 160};
const PixelColor kDesktopFGColor{255, 255, 255};

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

char console_buf[sizeof(Console)];
Console *console;

int printk(const char *format, ...) {
    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console->PutString(s);
    return result;
}

char mouse_cursor_buf[sizeof(MouseCursor)];
MouseCursor *mouse_cursor;

void MouseObserver(int8_t displacement_x, int8_t displacement_y) {
    mouse_cursor->MoveRelative({displacement_x, displacement_y});
}

void SwitchEhci2Xhci(const pci::Device &xhc_dev) {
    bool intel_ehc_exist = false;
    for (int i = 0; i < pci::num_device; ++i) {
        if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x20u) &&
            0x8086 == pci::ReadVendorId(pci::devices[i])) {
            intel_ehc_exist = true;
            break;
        }
    }
    if (!intel_ehc_exist) {
        return;
    }

    uint32_t superspeed_ports = pci::ReadConfReg(xhc_dev, 0xdc);
    pci::WriteConfReg(xhc_dev, 0xd8, superspeed_ports);
    uint32_t ehci2xhci_ports = pci::ReadConfReg(xhc_dev, 0xd4);
    pci::WriteConfReg(xhc_dev, 0xd0, ehci2xhci_ports);
    Log(kDebug, "SwitchEhci2Xhci: SS = %02, xHCI = %02x\n",
        superspeed_ports, ehci2xhci_ports);
}

void ControlCaching(bool enable) {
    if (enable) {
        uint32_t CR0 = ReadCR0();
        SetCR0(CR0 &= ~0x60000000);
    } else {
        uint32_t CR0 = ReadCR0();
        SetCR0(CR0 |= 0x60000000);
    }
}

__attribute__((interrupt))
void ExceptionHandlerWithError(InterruptFrame *frame, uint64_t error_code) {
    FillRectangle(*pixel_writer,
                  {0, 0},
                  {600, 600},
                  {255, 255, 255});
    while (1) __asm__("HLT");
}

__attribute__((interrupt))
void ExceptionHandlerWithoutError(InterruptFrame *frame) {
    FillRectangle(*pixel_writer,
                  {0, 0},
                  {600, 600},
                  {255, 255, 255});
    while (1) __asm__("HLT");
}

usb::xhci::Controller* xhc;

<<<<<<< HEAD
__attribute__((interrupt))
void IntHandlerXHCI(InterruptFrame* frame) {
    while (xhc->PrimaryEventRing()->HasFront()) {
        if (auto err = ProcessEvent(*xhc)) {
            Log(kError, "Error while Process Event: %s at %s:%d\n",
                err.Name(), err.File(), err.Line());
        }
    }
    NotifyEndOfInterrupt();
}

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(const FrameBufferConfig &frame_buffer_config_ref, const MemoryMap &memory_map_ref) {
    FrameBufferConfig frame_buffer_config{frame_buffer_config_ref};
    MemoryMap memory_map{memory_map_ref};
    SetLogLevel(kWarn);
=======
extern "C" void KernelMain(const FrameBufferConfig &frame_buffer_config) {
>>>>>>> parent of 73748f1 (修正：ログ出力)
    switch (frame_buffer_config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)
                    RGBResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
        case kPixelBGRResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)
                    BGRResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
    }

    const int kFrameWidth = frame_buffer_config.horizontal_resolution;
    const int kFrameHeight = frame_buffer_config.vertical_resolution;

    FillRectangle(*pixel_writer,
                  {0, 0},
                  {kFrameWidth, kFrameHeight - 50},
                  kDesktopBGColor);
    FillRectangle(*pixel_writer,
                  {0, kFrameHeight - 50},
                  {kFrameWidth, 50},
                  {190, 190, 190});
    DrawRectangle(*pixel_writer,
                  {10, kFrameHeight - 40},
                  {60, 30},
                  {100, 100, 100});

    console = new(console_buf) Console{*pixel_writer, kDesktopFGColor, kDesktopBGColor};

    SetLogLevel(kDebug);
    SetupIdentityPageTable();
    SetLogLevel(kWarn);

    printk("Welcome to DRARA-OS!\n");
    printk("\n");
    printk("2543 * 15 + 8 = %d\n", 2543 * 15 + 8);

    SetupSegments();
    Log(kInfo, "SetupSegments is finished\n");

    const uint16_t kernel_cs = 1 << 3;
    const uint16_t kernel_ss = 2 << 3;
    SetDSAll(0);
    SetCSSS(kernel_cs, kernel_ss);
    const uint16_t cs = GetCS();
    SetupBasicExceptionHandler(MakeIDTAttr(DescriptorType::kInterruptGate, 0),
                               reinterpret_cast<uint64_t>(ExceptionHandlerWithoutError),
                               reinterpret_cast<uint64_t>(ExceptionHandlerWithError), cs);

    LoadIDT(sizeof(idt) - 1, reinterpret_cast<uintptr_t>(&idt[0]));

    uintptr_t CR3 = ReadCR3();
    uintptr_t page_table_top = MakeIdentityPageTable();
    SetCR3(page_table_top);

    const std::array available_memory_types{
        MemoryType::kEfiBootServicesCode,
        MemoryType::kEfiBootServicesData,
        MemoryType::kEfiConventionalMemory,
    };

    Log(kDebug, "memory_map: %p\n", &memory_map);

    const auto memory_map_base = reinterpret_cast<uintptr_t>(memory_map.buffer);
    for (uintptr_t iter = reinterpret_cast<uintptr_t>(memory_map.buffer);
         iter < reinterpret_cast<uintptr_t>(memory_map.buffer) + memory_map.map_size;
         iter += memory_map.descriptor_size) {
        auto desc = reinterpret_cast<MemoryDescriptor *>(iter);
        for (int i = 0; i < available_memory_types.size(); ++i) {
            if (desc->type == available_memory_types[i]) {
                printk("type = %u, phys = %08lx - %08lx, pages = %lu\n",
                       desc->type,
                       desc->physical_start,
                       desc->physical_start + desc->number_of_pages * 4096 - 1,
                       desc->number_of_pages);
            }
        }
    }

    mouse_cursor = new(mouse_cursor_buf) MouseCursor{
        pixel_writer, kDesktopBGColor, {300, 200}
    };

    auto err = pci::ScanAllBus();
    SetLogLevel(kWarn);
    Log(kDebug, "ScanAllBus: %s\n", err.Name());

    for (int i = 0; i < pci::num_device; ++i) {
        const auto& dev = pci::devices[i];
        auto vendor_id = pci::ReadVendorId(dev);
        auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
        Log(kDebug, "%d.d.%d: vend %04x, class %08x, head %02x\n",
            dev.bus, dev.device, dev.function,
            vendor_id, class_code, dev.header_type);
    }

    pci::Device* xhc_dev = nullptr;

    for (int i = 0; i < pci::num_device; ++i) {
        if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u)) {
            xhc_dev = &pci::devices[i];

            if (0x8086 == pci::ReadVendorId(*xhc_dev)) {
                break;
            }
        }
    }

    if (xhc_dev) {
        Log(kInfo, "xHC has been found: %d.%d.%d\n",
            xhc_dev->bus, xhc_dev->device, xhc_dev->function);
    }

        SetIDTEntry(idt[InterruptVector::kXHCI], MakeIDTAttr(DescriptorType::kInterruptGate, 0),
            reinterpret_cast<uint64_t>(IntHandlerXHCI), cs);
        LoadIDT(sizeof(idt) -1, reinterpret_cast<uintptr_t>(&idt[0]));

        const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
        Log(kDebug, "ReadBar: %s\n", xhc_bar.error.Name());
        const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<uint64_t>(0xf);
        Log(kError, "xHC mmio_base = %08lx\n", xhc_mmio_base);
        Log(kDebug, "xHC mmio_base = %08lx\n", xhc_mmio_base);
        SetLogLevel(kWarn);

        const uint16_t cs = GetCS();
        SetupBasicExceptionHandler(MakeIDTAttr(DescriptorType::kInterruptGate, 0),
                                   reinterpret_cast<uint64_t>(ExceptionHandlerWithoutError),
                                   reinterpret_cast<uint64_t>(ExceptionHandlerWithError), cs);
        LoadIDT(sizeof(idt) - 1, reinterpret_cast<uintptr_t>(&idt[0]));

        usb::xhci::Controller xhc{xhc_mmio_base};

        if (0x8086 == pci::ReadVendorId(*xhc_dev)) {
            SwitchEhci2Xhci(*xhc_dev);
        }
        for (int i = 0; i < pci::num_device; ++i) {
            const auto &dev = pci::devices[i];
            auto vendor_id = pci::ReadVendorId(dev);
            auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
            Log(kDebug, "%d.%d.%d: vend %04x, class %08x, head %02x\n", dev.bus, dev.device, dev.function, vendor_id,
                class_code, dev.header_type);
        }

        ControlCaching(false);
        Log(kError, "Caching is disabled\n");
        Log(kError, "mmio_base's physical address is %p\n", FindPhysicalAddress(xhc_mmio_base, ReadCR3()));

        {
            SetLogLevel(kDebug);
            auto err = xhc.Initialize();
            Log(kInfo, "xhc.Initialize: %s\n", err.Name());
            SetLogLevel(kWarn);
        }

        ControlCaching(true);
        Log(kError, "Caching is enabled\n");

        Log(kInfo, "xHC starting\n");
        xhc.Run();

        ::xhc = &xhc;
        __asm__("STI");

        usb::HIDMouseDriver::default_observer = MouseObserver;

        /*
        for (int i = 1; i <= xhc.MaxPorts(); ++i) {
            auto port = xhc.PortAt(i);
            Log(kDebug, "Port %d: Is Connected=%d\n", i, port.IsConnected());

            if (port.IsConnected()) {
                if (auto err = ConfigurePort(xhc, port)) {
                    Log(kWarn, "Failed to Configure port: %s at %s:%d\n",
                        err.Name(), err.File(), err.Line());
                    continue;
                }
            }
        }
        */
        while (1) {
            if (auto err = ProcessEvent(xhc)) {
                Log (kError, "Erro while Process Event");
            }
        while (1) __asm__("HLT");
    }
}

extern "C" void __cxa_pure_virtual() {
    while (1) __asm__("HLT");
}
