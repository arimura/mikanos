DOC_DIR=doc

setup: mikanos
	xhost + 127.0.0.1

mikanos:
	git clone git@github.com:uchan-nos/mikanos.git mikanos

$(DOC_DIR)/%.mmd.pdf:
	docker run --rm -u `id -u`:`id -g` -v ./doc:/data minlag/mermaid-cli -i $*.mmd -e pdf 

show-%.mmd.pdf:  $(DOC_DIR)/%.mmd.pdf
	open $<


