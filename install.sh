#!/usr/bin/env bash
# author:	Jacob Xie
# date:	2024/12/03 12:44:24 Tuesday
# brief:

mkdir build
cd build
# install prefix
cmake -DCMAKE_INSTALL_PREFIX=$HOME/cpp-todo-deploy ..
make
make install
