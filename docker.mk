# makefile executed within docker container
HOME:=/home/vscode
SHELL:=/bin/bash
KERNEL:=kernel.elf
KERNEL_DIR:=my_mikanos/kernel

all: disk.img run-qemu

$(KERNEL_DIR)/$(KERNEL):
	$(MAKE) -C $(KERNEL_DIR) $(KERNEL) 

build:
	cd $(HOME)/edk2 && build 

EFI=$(HOME)/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi
run-qemu: $(KERNEL_DIR)/$(KERNEL)
ifeq ($(EFI),)
	$(error "EFI is not set")
endif
	/home/vscode/osbook/devenv/run_qemu.sh $(EFI) $(KERNEL_DIR)/$(KERNEL)

disk.img:
ifeq ($(EFI),)
	$(error "EFI is not set")
endif
	qemu-img create -f raw disk.img 200M
	mkfs.fat -n 'MIKAN OS' -s 2 -f 2 -R 32 -F 32 disk.img
	mkdir -p mnt
	sudo mount -o loop disk.img mnt
	sudo mkdir -p mnt/EFI/BOOT
	sudo cp $(EFI) mnt/EFI/BOOT/BOOTX64.EFI
	sudo umount mnt

update-tools_def:
	sed -i -e '/DEFINE CLANG38_X64_TARGET/s/-target x86_64-pc-linux-gnu/-target x86_64-linux-gnu/' \
       -e '/DEBUG_CLANG38_X64_CC_FLAGS/s/\([^[:cntrl:]]*\)/\1 -I\/usr\/x86_64-linux-gnu\/include/' /home/vscode/edk2/Conf/tools_def.txt

clean:
	rm -f $(KERNEL_DIR)/$(KERNEL)
	rm -f $(KERNEL_DIR)/*.o
	rm -f disk.img
