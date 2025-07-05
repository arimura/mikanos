setup: mikanos
	xhost + 127.0.0.1

mikanos:
	git clone git@github.com:uchan-nos/mikanos.git mikanos

validate-uml:
	docker run --rm -u `id -u`:`id -g` -v ./doc:/data minlag/mermaid-cli -i gui.mmd