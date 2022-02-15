cmake_minimum_required(VERSION 3.20.0)

set(BOARD qemu_x86)

find_package(Zephyr)
project(Zephyr_Game_Of_Life)

target_sources(app PRIVATE src/Main.c)
