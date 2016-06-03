@echo off
if "%1"=="" ( 
	echo usage: makepb.cmd protobuf_dir [-fetch]
	exit /b
)

setlocal
set _PROTOBUF_DIR=%1

if not exist "%_PROTOBUF_DIR%\src\google\protobuf\" (
	if "%2" EQU "-fetch" (
		echo Pulling from GitHub...
		git clone "https://github.com/google/protobuf.git" "%_PROTOBUF_DIR%"
	) else (
		echo error: The specified directory does not contain protobuf source code.
		exit /b 1
	)
)

pushd
mkdir %_PROTOBUF_DIR%\cmake\build 
cd /d %_PROTOBUF_DIR%\cmake\build
cmake -G "Visual Studio 12 Win64" -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF -DCMAKE_INSTALL_PREFIX=./install ..
call "%ProgramFiles(x86)%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64
cd
msbuild /m protobuf.sln /p:Platform=x64 /p:Configuration=Debug
msbuild /m protobuf.sln /p:Platform=x64 /p:Configuration=Release
msbuild /m INSTALL.vcxproj /p:Platform=x64 /p:Configuration=Debug
msbuild /m INSTALL.vcxproj /p:Platform=x64 /p:Configuration=Release

popd

:end

