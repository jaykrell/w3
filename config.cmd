@echo off

SetLocal

echo int config; > config.cpp

echo _MSC_VER > mscver.cpp
for /f %%a in ('cl /nologo /EP mscver.cpp') do set mscver=%%a

::  not very useful currently
::  Visual C++ 15.0 has typed enums but __cplusplus=199711L
::  might still use it later
::  echo __cplusplus > cplusplus.cpp
::  for /f %%a in ('cl /nologo /TP /EP cplusplus.cpp') do set cplusplus=%%a

set HAS_TYPED_ENUM=1
echo enum a : int { }; > typedenum.cpp
if errorlevel 1 set HAS_TYPED_ENUM=0

set Wall=/Wall
cl /nologo %Wall% /c config.cpp
if errorlevel 1 set Wall=
echo Wall=%Wall%

del config.mk 2>nul

cl 2>&1 | findstr /e /i x64 >nul: && goto :x64
cl 2>&1 | findstr /e /i amd64 >nul: && goto :amd64
cl 2>&1 | findstr /e /i x86 >nul: && goto :x86
cl 2>&1 | findstr /e /i arm >nul: && goto :arm
echo ERROR: Failed to configure.
goto :eof

:x86
:amd64
echo ARM=0 >>config.mk
echo AMD64=0 >>config.mk
echo 386=1 >>config.mk
goto :end

:arm
echo ARM=1 >>config.mk
echo AMD64=0 >>config.mk
echo 386=0 >>config.mk
goto :end

:x64
:amd64
echo ARM=0 >>config.mk
echo AMD64=1 >>config.mk
echo 386=0 >>config.mk
goto :end

:end
::  echo cplusplus=%cplusplus% >>config.mk
del config.h 2>nul
echo mscver=%mscver% >>config.mk
echo Wall=%Wall% >>config.mk
echo CONFIG_H=1 >>config.mk
echo HAS_TYPED_ENUM=%HAS_TYPED_ENUM% >>config.mk
echo #define HAS_TYPED_ENUM %HAS_TYPED_ENUM% >>config.h

type config.mk
type config.h
goto :eof
