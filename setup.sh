#!/bin/bash

# keep original directory
ORIGINAL_DIR=$(pwd)

# edk build setup
cd $HOME/edk2 && source edksetup.sh
echo "Setup EDK2 environment"

# kernel build setup
cd $HOME/osbook/devenv && source buildenv.sh 
echo "Setup kernel build environment"

# back to original directory
cd $ORIGINAL_DIR
