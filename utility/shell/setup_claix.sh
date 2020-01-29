#!/bin/sh

INITIAL_DIRECTORY=$PWD
ROOT_DIRECTORY=/hpcwork/rwth0432

# Enable modules.
module swap intelmpi openmpi/4.0.2
module load gcc/9 cmake/3.13.2
module load python/3.6.0

# Install git.
cd $ROOT_DIRECTORY

if [ ! -d "packages" ]; then 
  mkdir packages; 
fi
cd packages

if [ ! -d "git" ]; then 
  mkdir git
  cd git

  git clone --branch v2.24.0 https://github.com/git/git.git; 
  cd git
  make configure
  ./configure --prefix=$ROOT_DIRECTORY/packages/git
  
  make
  make install
fi

export PATH=$ROOT_DIRECTORY/packages/git/bin:$PATH

cd $INITIAL_DIRECTORY
