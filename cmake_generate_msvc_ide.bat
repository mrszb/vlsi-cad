mkdir .msvc_ide

cd .msvc_ide
SET BOOST_VER=1.56.0

SET BOOST_LIBRARYDIR=C:\MR\Boost\%BOOST_VER%\lib\msvc-12.0\x64
SET BOOST_INCLUDEDIR=C:\MR\Boost\%BOOST_VER%\include

%MR_ROOT%\CMake\3.0.1\bin\cmake -G "Visual Studio 12 Win64" ..

cd ..
