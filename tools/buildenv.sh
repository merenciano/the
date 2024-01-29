#!/bin/bash

gcc -I../extern/include/ -I../src ../extern/src/mathc.c ../extern/src/glad.c genenv.c -L../lib -lnyas -lGL -lX11 -lpthread -ldl -lm -o genenv
