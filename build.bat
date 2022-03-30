
@echo off
call python3 utility\build.py 
if %errorlevel% == 1 ( exit /B 1)
exit /B 0
