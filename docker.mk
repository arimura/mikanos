# makefile executed within docker container
HOME=/home/vscode

run-qemu:
	

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

clean:
	rm -f disk.img

