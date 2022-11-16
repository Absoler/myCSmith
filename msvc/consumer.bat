setlocal enabledelayedexpansion
@echo off
set /p base=<base.txt
set buffer_len=15000
for /f %%i in ('dir buffer /b ^|find /c ^"output^" ') do set len=%%i
:start
echo %len%
set /p flag=<flag.txt
if %len% GEQ 1 (
    for /f %%f in ('dir buffer /b') do (
        echo %%f
        set objfile=%%f
        echo !objfile:.c=.obj!
        cl buffer\%%f  /w /O2 /Zi /link /out:output.exe
        ..\..\pin-3.21-98484-ge7cd811fd-msvc-windows\pin.exe -t ..\..\pin-3.21-98484-ge7cd811fd-msvc-windows\source\tools\MyPinTool\Release\MyPinTool.dll -- .\output.exe func_
        echo now check
        set /p res=<result.txt
        echo res !res!
        if !res!==1 (
            echo buffer\%%f
            copy buffer\%%f o2\!base!_output.c
            copy checkRead.txt o2\!base!_checkRead.txt
            set /a base+=1
            echo !base!> base.txt
        )
        @REM del buffer\%%f
        del !objfile:.c=.obj!
        del output.*
        echo.
        echo.
    )
)
echo 0 > flag.txt
@REM goto start
@echo on