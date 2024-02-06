clang -O3 -I../extern/include/ -I../src ../extern/src/mathc.c ../extern/src/glad.c genenv.c -L../lib -lnyas_d -lGL -lX11 -lm -o genenv
