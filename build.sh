#!/bin/bash

g++-4.9 -o $1 ${1}.cpp `pkg-config --cflags --libs hpx_application` -std=c++1y 
