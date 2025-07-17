# makefile executed within docker container
HOME:=/home/vscode
SHELL:=/bin/bash
KERNEL:=kernel.elf
KERNEL_DIR:=my_mikanos/kernel
PKG_LINK:=/home/vscode/edk2/MikanLoaderPkg
MY_PKG:=/workspaces/mikanos/my_mikanos/MikanLoaderPkg

all: clean $(KERNEL_DIR)/$(KERNEL) build run-qemu


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

# P51の設定も行う
init: update-tools_def update-taget
	ln -s $(MY_PKG) $(PKG_LINK)

update-tools_def:
	sed -i -e '/DEFINE CLANG38_X64_TARGET/s/-target x86_64-pc-linux-gnu/-target x86_64-linux-gnu/' \
       -e '/DEBUG_CLANG38_X64_CC_FLAGS/s/\([^[:cntrl:]]*\)/\1 -I\/usr\/x86_64-linux-gnu\/include/' /home/vscode/edk2/Conf/tools_def.txt

update-taget:
	sed -i \
	  -e '/^ACTIVE_PLATFORM[[:space:]]*=/s|EmulatorPkg/EmulatorPkg.dsc|MikanLoaderPkg/MikanLoaderPkg.dsc|' \
	  -e '/^TARGET_ARCH[[:space:]]*=/s|IA32|X64|' \
	  -e '/^TOOL_CHAIN_TAG[[:space:]]*=/s|VS2015x86|CLANG38|' \
	  /home/vscode/edk2/Conf/target.txt

clean:
	rm -f $(KERNEL_DIR)/$(KERNEL)
	find $(KERNEL_DIR) -type f -name '*.o' -delete
	find $(KERNEL_DIR) -type f -name '*.d' -delete
	rm -f disk.img
