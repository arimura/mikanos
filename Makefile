setup: repo/mikanos
	xhost + 127.0.0.1

repo/mikanos:
	mkdir -p repo && \
	  git clone git@github.com:uchan-nos/mikanos.git repo/mikanos