mkdir .msvc_ide

cd .msvc_ide
SET BOOST_VER=1.65.0
SET CMAKE_VER=3.10.1

SET USER_LIBS=%USERPROFILE%\libraries

SET BOOST_LIBRARYDIR=%USER_LIBS%\Boost\%BOOST_VER%\lib\msvc-14.1\x64
SET BOOST_INCLUDEDIR=%USER_LIBS%\Boost\%BOOST_VER%\include

REM %USER_LIBS%\CMake\%CMAKE_VER%\bin\cmake -G "Visual Studio 14 2015 Win64" ..
%USER_LIBS%\CMake\%CMAKE_VER%\bin\cmake -G "Visual Studio 15 2017 Win64" ..

cd ..
