import subprocess

compiler = "clang++"
flags = ["-W", "-std=c++20"]
include_dirs = ["src/the"]
files = ["src/main.cpp",
		 "src/the/scene.cpp"]
out = "bin/the-test"

def include_dir_arg() :
	ret = []
	for arg in include_dirs:
		ret.append("-I" + arg)
	return ret

command = [compiler] + flags + ["-o"+out] + include_dir_arg() + files
#print(command)
subprocess.run(command)
