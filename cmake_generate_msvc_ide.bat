mkdir .msvc_ide

cd .msvc_ide
SET BOOST_LIBRARYDIR=C:\MR\Boost\1.54.0\lib\msvc-11.0\x64
SET BOOST_INCLUDEDIR=C:\MR\Boost\1.54.0\include

%MR_ROOT%\CMake\2.8.11.1\bin\cmake -G "Visual Studio 11 Win64" ..

cd ..
