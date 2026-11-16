cmake_minimum_required(VERSION 4.2)
project(AO_projekt)

set(CMAKE_CXX_STANDARD 20)

add_executable(AO_projekt main.cpp
        Seq/MD5_seq.h
        Seq/Brute_Force_seq.h
        Consts.h
        Avx2/MD5_avx2.h
        Avx2/MD5_Helper.h
        Avx2/Brute_force_avx2.h
        Pararrell/brute_force_Parrarell.h)
target_compile_options(AO_projekt PRIVATE -mavx2)

find_package(Threads REQUIRED)
target_link_libraries(AO_projekt PRIVATE Threads::Threads)