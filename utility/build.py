
import enum
import subprocess

class CMakeLogLevel:
	warning="WARNING"
	notice="NOTICE"
	error="ERROR"
	status="STATUS"
	verbose="VERBOSE"
	debug="DEBUG"
	trace="TRACE"

def make_cmake_generate_command(definitions,
	generator 	: str = "Ninja",
	source_root : str = ".",
	build_root 	: str = "_build",
	log_level 	: CMakeLogLevel = "VERBOSE"):

	command = [
		"cmake",
		"-G", generator,
		"--log-level=" + str(log_level)
	]
	command.extend(definitions)
	command.extend(["-S", source_root])
	command.extend(["-B", build_root])
	return command

cmake_generate_command_definitions=[
	"-DC=clang",
	"-DCXX=clang++",
]

cmake_generate_command = make_cmake_generate_command(
	cmake_generate_command_definitions,
	log_level=CMakeLogLevel.verbose
)

cmake_build_command = [
	"cmake",
	"--build",
	"_build",
]
cmake_install_command = [
	"cmake",
	"--install",
	"_build",
	"--prefix",
	"_install",
]

def generate():
	result = subprocess.run(cmake_generate_command)
	if result.returncode == 0:
		return True
	else:
		return False

def build():
	result = subprocess.run(cmake_build_command)
	if result.returncode == 0:
		return True
	else:
		return False

def install():
	result = subprocess.run(cmake_install_command)
	if result.returncode == 0:
		return True
	else:
		return False


if not generate():
	exit(1)
if not build():
	exit(1)
if not install():
	exit(1)
