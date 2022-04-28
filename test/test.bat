@echo off

set cmd=notepad.exe

echo launching %cmd%
echo waiting process end...

rem cmd /v:on /c "echo !TIME! & *mycommand* & echo !TIME!"
r
rem timeout /t 30 /nobreak > NUL
rem powershell -nop -c "& {sleep seconds}"
rem PS> Measure-Command { echo hi }
rem PS> (Measure-Command { echo hi | Out-Default }).ToString()
rem powershell -Command "Measure-Command {echo hi | Out-Default}"
echo time=%time%

..\msvc\x64-Debug\prctime.exe %cmd%

echo time=%time%
echo ret=%errorlevel%

pause
