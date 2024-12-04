#!/usr/bin/env bash
# author:	Jacob Xie
# date:	2024/12/04 09:05:02 Wednesday
# brief:	Copy files under ./complex to ./library/include

rsync -av --exclude='CMakeLists.txt' --exclude='Main.cpp' ./complex/ ./library/include/
