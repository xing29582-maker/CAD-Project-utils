@echo off
setlocal

REM ==============================
REM 配置区（按需改）
REM ==============================
set "BUILD_DIR=build_vs"
set "GENERATOR=Visual Studio 17 2022"
set "ARCH=x64"

REM ==============================
REM 切到脚本所在目录
REM ==============================
cd /d "%~dp0"
echo [INFO] Source: "%cd%"
echo [INFO] Build : "%cd%\%BUILD_DIR%"

REM ==============================
REM 创建构建目录
REM ==============================
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM ==============================
REM 强制重新生成（删 cache）
REM ==============================
rmdir /s /q "%BUILD_DIR%\CMakeFiles" 2>nul
del /q "%BUILD_DIR%\CMakeCache.txt" 2>nul

REM ==============================
REM 生成 VS Solution / Project files（关键：这行你丢了）
REM ==============================
echo [INFO] Running CMake configure...
cmake -S "%cd%" -B "%BUILD_DIR%" -G "%GENERATOR%" -A %ARCH% ^
  -DCMAKE_PREFIX_PATH="%cd%\third_party\qt-5.14.0\5.14.0\msvc2017_64"

if errorlevel 1 (
  echo.
  echo [ERROR] CMake configure failed!
  pause
  exit /b 1
)

echo.
echo [INFO] Generated files:
dir "%BUILD_DIR%\*.sln" "%BUILD_DIR%\*.vcxproj" "%BUILD_DIR%\*.vcxproj.filters" 2>nul

echo.
echo [OK] Generated successfully.
pause
endlocal
