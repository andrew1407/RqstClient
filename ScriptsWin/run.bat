@echo off

call "%~dp0\config.bat"

set MODE_OPTION=%1

set CONFIGURATION=
set SCRIPT_NAME=
set TARGET=

if not defined MODE_OPTION set MODE_OPTION=ebd

IF "%MODE_OPTION%" == "ebd" (
    @REM Editor build options
    set CONFIGURATION=%PROJECT_NAME%Editor
    set SCRIPT_NAME=Build.bat
    set TARGET=Development
) else if "%MODE_OPTION%" == "pbd" (
    @REM Project build options
    set CONFIGURATION=%PROJECT_NAME%
    set SCRIPT_NAME=Build.bat
    set TARGET=Development
) else if "%MODE_OPTION%" == "pbs" (
    @REM Project build options
    set CONFIGURATION=%PROJECT_NAME%
    set SCRIPT_NAME=Build.bat
    set TARGET=Shipping
) else (
    echo "%MODE_OPTION%" is an unknown param.
    exit \b
)

set SCRIPT="%SCRIPTS_ROOT%%SCRIPT_NAME%"

%SCRIPT% %CONFIGURATION% %PLATFORM% %TARGET% %PROJECT_CONFIG% -waitmutex
