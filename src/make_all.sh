#!/bin/bash
# make_all.sh

arch_cpu=x86-64-avx2
make --no-print-directory -j build ARCH=${arch_cpu} COMP=mingw
strip napoleon.exe
mv napoleon.exe napoleon-nnue_x64_avx2.exe
make gcc-profile-clean

arch_cpu=x86-64-bmi2
make --no-print-directory -j build ARCH=${arch_cpu} COMP=mingw
strip napoleon.exe
mv napoleon.exe napoleon-nnue_x64_bmi2.exe
make gcc-profile-clean
