@echo on
if "%1"=="" ( 
	echo usage: makepb.cmd protobuf_dir [-fetch]
	exit /b
)

setlocal
set _PROTOBUF_DIR=%1
set PROGRAM_FILES=%ProgramFiles(x86)%
if "%PROGRAM_FILES%"=="" set PROGRAM_FILES=%ProgramFiles%

set _os_bitness=64
IF %PROCESSOR_ARCHITECTURE% == x86 (
    IF NOT DEFINED PROCESSOR_ARCHITEW6432 Set _os_bitness=32
  )

set TARGET_ARCH=x64
set TARGET_CMAKE_ARCH=Win64
if %_os_bitness%==32 (
    set TARGET_ARCH=x86
    set TARGET_CMAKE_ARCH=Win32
  )

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
cmake -G "Visual Studio 12 %TARGET_CMAKE_ARCH%" -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF -DCMAKE_INSTALL_PREFIX=./install ..
call "%PROGRAM_FILES%\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %TARGET_ARCH%
cd
msbuild /m protobuf.sln /p:Platform=%TARGET_ARCH% /p:Configuration=Debug
msbuild /m protobuf.sln /p:Platform=%TARGET_ARCH% /p:Configuration=Release
msbuild /m INSTALL.vcxproj /p:Platform=%TARGET_ARCH% /p:Configuration=Debug
msbuild /m INSTALL.vcxproj /p:Platform=%TARGET_ARCH% /p:Configuration=Release

popd

:end

