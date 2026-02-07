#!/bin/bash

cmake -S . -B build
cmake --build ./build --target loopback
cmake --build ./build --target spisend
