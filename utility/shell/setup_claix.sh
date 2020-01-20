#!/bin/sh

INITIAL_DIRECTORY=$PWD

# Enable modules.
module swap intelmpi openmpi
module load gcc/9 cmake/3.13.2
module load python/3.6.0

# Install git.
cd ~
if [ ! -d "packages" ]; then mkdir packages; fi
cd packages
if [ ! -d "git" ]; then git clone --branch v2.24.0 https://github.com/git/git.git; fi
cd git
make configure
./configure --prefix=/home/ad784563/packages
make
make install
export PATH=~/packages/bin:$PATH

cd $INITIAL_DIRECTORY
