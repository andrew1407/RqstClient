@echo off

set MODE=%1

set CURRENT_DIR=%CD%

set UE_VERSION=5.2
set UE_ROOT=C:\Program Files\Epic Games\UE_%UE_VERSION%

SET PROJECT_NAME=RqstClient
set PLATFORM=Win64

set SCRIPTS_ROOT=Engine\Build\BatchFiles\
set PROJECT_PATH=X:\Progects\UnrealEngine\%PROJECT_NAME%\%PROJECT_NAME%.uproject

set CONFIGURATION=
set SCRIPT=
SET TARGET=

cd /d "%UE_ROOT%"

IF NOT DEFINED MODE set MODE=be

IF "%MODE%" == "be" (
    set CONFIGURATION=%PROJECT_NAME%Editor
    set SCRIPT=Build.bat
    SET TARGET=Development
) ELSE IF "%MODE%" == "bp" (
    set CONFIGURATION=%PROJECT_NAME%
    set SCRIPT=Build.bat
    SET TARGET=Development
)

call %SCRIPTS_ROOT%%SCRIPT% %CONFIGURATION% %PLATFORM% %TARGET% %PROJECT_PATH% -waitmutex

cd /d "%CURRENT_DIR%"
