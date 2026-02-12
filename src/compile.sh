#!/usr/bin/env bash

echo "==> 1. Building runtime library"
cd ./runtime && ( make clean && make ) && cd ..

echo "==> 2. Building LLVM pass plugin"
( make clean && make )
