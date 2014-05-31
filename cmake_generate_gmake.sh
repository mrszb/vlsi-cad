#!/bin/sh

mkdir -p .gmake/release
mkdir -p .gmake/debug

cd .gmake/release

cmake -G "Unix Makefiles" -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_BUILD_TYPE=Release ../..

cd ../..

cd .gmake/debug

cmake -G "Unix Makefiles" -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) -DCMAKE_BUILD_TYPE=Debug ../..

cd ../..

