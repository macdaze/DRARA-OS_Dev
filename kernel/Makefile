SHELL := /bin/zsh

TARGET = kernel.elf
OBJS = main.o graphics.o mouse.o font.o hankaku.o console.o paging.o\
       pci.o asmfunc.o libcxx_support.o newlib_support.o logger.o \
       usb/memory.o usb/device.o usb/xhci/ring.o usb/xhci/trb.o usb/xhci/xhci.o \
       usb/xhci/port.o usb/xhci/device.o usb/xhci/devmgr.o usb/xhci/registers.o \
       usb/classdriver/base.o usb/classdriver/hid.o usb/classdriver/keyboard.o \
       usb/classdriver/mouse.o
DEPENDS = $(join $(dir $(OBJS)),$(addprefix .,$(notdir $(OBJS:.o=.d))))
CFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
			-fno-exceptions -fno-rtti -std=c++17 \
			--sysroot=/Users/yuji/workspace/DRARA-OS_Dev/x86_64-elf \
			-I. \
			-I/Users/yuji/workspace/DRARA-OS_Dev/x86_64-elf/usr/include/c++/v1
LDFLAGS += --entry KernelMain -z norelro --image-base 0x100000 --static -z separate-code


.PHONY: all
all: edk2 $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o

.PHONY: run
run: kernel.elf
	$(HOME)/osbook/devenv/run_qemu.sh \
		$(HOME)/edk2/Build/MikanLoaderX64/DEBUG_CLANGPDB/X64/Loader.efi \
		$(HOME)/workspace/DRARA-OS_Dev/kernel/kernel.elf

edk2: ../MikanLoaderPkg/Main.c
	zsh -c 'cd $(HOME)/edk2 && source edksetup.sh && build'

kernel.elf: $(OBJS) Makefile
	ld.lld $(LDFLAGS) \
  -L/Users/yuji/workspace/DRARA-OS_Dev/x86_64-elf/usr/lib \
  -o kernel.elf $(OBJS) -lc

main.o: main.cpp
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c main.cpp

hankaku.o: hankaku.txt
	../tools/makefont.py -o hankaku.bin hankaku.txt
	x86_64-elf-objcopy -I binary -O elf64-x86-64 -B i386:x86-64 hankaku.bin hankaku.o

%.o: %.cpp %.hpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	clang -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone --sysroot=/Users/yuji/workspace/DRARA-OS_Dev/x86_64-elf -std=c11 -c $<


.%.d: %.cpp
	clang++ $(CPPFLAGS) $(CXXFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed -I '' -e 's|$(notdir $(OBJ))|$(OBJ)|' $@

%.o: %.asm Makefile
	nasm -f elf64 -o $@ $<

.PHONY: depends
depends:
	$(MAKE) $(DEPENDS)

-include $(DEPENDS)
