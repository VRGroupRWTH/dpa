#!/bin/sh

INITIAL_DIRECTORY = $PWD

# Enable modules.
module swap intelmpi openmpi
module load gcc/9 cmake/3.13.2

# Install git.
cd ~
mkdir packages
cd packages

git clone --branch v2.24.0 https://github.com/git/git.git
cd git
make configure
./configure --prefix=/home/ad784563/packages
make
make install
export PATH=~/packages/bin:$PATH

cd $INITIAL_DIRECTORY