#!/bin/bash

# Config
POSTFIX=_d
BUILD_PATH=./build
C_COMP=clang
CXX_COMP=clang++

# Defaults
BUILD_TYPE=Debug
GENERATE=1
BUILD=1
CLEAN=0
GENERATOR=/usr/bin/make


function BuildEmscripten()
{
  mkdir -p build/emsc
  cd build/emsc
  cmake -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DNYAS_EMSC=1 -DNYAS_GLES2=1 ../..
  emmake make
  exit
}

while getopts ":bdrecg:" ARGOPT; do
	case $ARGOPT in
		b)
			BUILD=1
			GENERATE=0
			;;
		g)
			GENERATE=1
			BUILD=0
			GENERATOR=/usr/bin/${OPTARG}
			;;
		d)
			BUILD_TYPE=Debug
			POSTFIX=_d
			;;
		r)
			BUILD_TYPE=Release
			POSTFIX=
			;;
		c)
			BUILD=0
			GENERATE=0
			CLEAN=1
			;;
	  e)
	    BuildEmscripten
	    ;;
	esac
done

if [[ ${CLEAN} -eq 1 ]]; then
	rm -drf ${BUILD_PATH}
fi

mkdir -p ${BUILD_PATH}/${BUILD_TYPE}
cd ${BUILD_PATH}/${BUILD_TYPE}

if [[ GENERATE -eq 1 ]]; then
	cmake -DNYAS_GL3=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_C_COMPILER=${C_COMP} -DCMAKE_CXX_COMPILER=${CXX_COMP} -DCMAKE_MAKE_PROGRAM=${GENERATOR} ../..
fi

if [[ BUILD -eq 1 ]]; then
	cmake --build .
fi

