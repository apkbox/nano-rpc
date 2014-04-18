mkdir Win32\Debug
mkdir Win32\Release
mkdir x64\Debug
mkdir x64\Release

copy vs2010\Win32\Debug\* Win32\Debug\
copy vs2010\x64\Debug\* x64\Debug\

copy vs2010\Win32\Release\* Win32\Release\
copy vs2010\x64\Release\* x64\Release\
