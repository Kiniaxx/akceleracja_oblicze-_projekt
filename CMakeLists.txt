cmake_minimum_required(VERSION 4.2)
project(AO_projekt)

set(CMAKE_CXX_STANDARD 20)

add_executable(AO_projekt main.cpp
        MD5_seq.h
        Brute_Force_seq.h
        Consts.h
        MD5_avx2.h)
