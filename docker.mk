# makefile executed within docker container
HOME=/home/vscode

run-qemu:
	${HOME}/osbook/devenv/run_qemu.sh ${EFI}