@echo off

set sec=1
rem set prg=cmd /c "timeout /t %sec% /nobreak > NUL"
set prg=powershell -nop -c "& {sleep %sec%}"

rem A first launch to avoid possible optimizations differences
%prg%

echo Launching: %prg%

echo ----(time=%time%)
echo.

rem ..\msvc\x64-Debug\prctime.exe %prg%
..\msvc\x64-Release\prctime.exe %prg%

echo.
echo ----(time=%time%)
echo ----(ret=%errorlevel%)

echo.
echo PS ^> Measure-Command
powershell -nop -c "(Measure-Command {sleep %sec% | Out-Default}).ToString()"

pause
