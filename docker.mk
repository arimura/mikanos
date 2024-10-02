# makefile executed within docker container
HOME=/home/vscode

all: disk.img run-qemu

run-qemu:
	qemu-system-x86_64 \
	  -drive if=pflash,file=$(HOME)/osbook/devenv/OVMF_CODE.fd \
	  -drive if=pflash,file=$(HOME)/osbook/devenv/OVMF_VARS.fd \
	  -hda disk.img

EFI=
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
       -e '/DEBUG_CLANG38_X64_CC_FLAGS/s/\(.*\)/\1 -I\/usr\/x86_64-linux-gnu\/include/' /home/vscode/edk2/Conf/tools_def.txt

clean:
	rm -f disk.img

