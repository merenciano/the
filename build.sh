#!/bin/bash

# Config
BUILD_PATH=./build
BUILD_TYPE=Debug
COMPILER=gcc
GENERATOR=make

function help()
{
	echo "build.sh - Generate and compile the CMake project"
	echo "example usage: ./build.sh"
	echo "verbose example equivalent: ./build.sh -t Debug -c gcc -g make"
	echo ""
	echo "Default options:"
	echo "  Compiler - gcc"
	echo "  Generator - make"
	echo "  Build type - Debug"
	echo ""
	echo "Args:"
	echo "  -t [Debug/Release/RelWithDebInfo/MinSizeRel]: Specify the build type"
	echo "  -g [make/gmake/ninja/...]: Specify the native build system (can be an absolute path)"
	echo "  -c [gcc/clang/...]: Specify the C compiler (can be an absolute path)"
	echo "  -h: Show this message"
	echo "  -r: Equivalent to -t Release"
	echo "  -d: Equivalent to -t Debug"
	echo ""
}

while getopts ":hdrc:g:t:" ARGOPT; do
	case $ARGOPT in
		d)
			BUILD_TYPE=Debug
			;;
		r)
			BUILD_TYPE=Release
			;;
		c)
			COMPILER=${OPTARG}
			;;
		g)
			GENERATOR=${OPTARG}
			;;
		t)
			BUILD_TYPE=${OPTARG}
			;;
		h|\?)
			help
			exit 0
			;;
	esac
done

mkdir -p ${BUILD_PATH}/${BUILD_TYPE}
cd ${BUILD_PATH}/${BUILD_TYPE}
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_COMPILER=${COMPILER} -DCMAKE_MAKE_PROGRAM=${GENERATOR} ../..
cmake --build .
