# makefile executed within docker container
HOME:=/home/vscode
SHELL:=/bin/bash
KERNEL:=kernel.elf
MY_WORKSPACE:=my_mikanos
KERNEL_DIR:=$(MY_WORKSPACE)/kernel
APP_DIR:=$(MY_WORKSPACE)/apps
PKG_LINK:=/home/vscode/edk2/MikanLoaderPkg
MY_PKG:=/workspaces/mikanos/my_mikanos/MikanLoaderPkg

all: clean build run

build:
	cd $(HOME)/edk2 && build 

run:
	cd $(MY_WORKSPACE) && ./build.sh run

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
	find $(KERNEL_DIR) -type f -name '*.d' -delete
	rm -f $(APP_DIR)/onlyhlt/onlyhlt
	rm -f disk.img
