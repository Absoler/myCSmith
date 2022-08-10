setlocal enabledelayedexpansion
@echo off
set /p base=<base.txt
set buffer_len=3
for /f %%i in ('dir buffer /b ^|find /c ^"output^" ') do set len=%%i
:start
echo %len%
set /p flag=<flag.txt
if %len% GEQ 1 (
    for /f %%f in ('dir buffer /b') do (
        echo %%f
        set objfile=%%f
        echo !objfile:.c=.obj!
        cl buffer\%%f  /w /Zi /link /out:output.exe
        ..\..\pin-3.21-98484-ge7cd811fd-msvc-windows\pin.exe -t ..\..\pin-3.21-98484-ge7cd811fd-msvc-windows\source\tools\MyPinTool\Release\MyPinTool.dll -- .\output.exe func_
        set /p res=<result.txt
        if %res%==1 (
            copy buffer\%%f problem\output%base%.c
            copy checkRead.txt problem\checkRead%base%.txt
            set /a base+=1
            echo %base% > base.txt
        )
        @REM del buffer\%%f
        del !objfile:.c=.obj!
        echo.
        echo.
    )
)
echo 0 > flag.txt
@REM goto start
@echo on