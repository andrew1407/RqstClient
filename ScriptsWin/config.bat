@echo off

set UE_VERSION=5.2
set UE_ROOT=C:\Program Files\Epic Games\UE_%UE_VERSION%\
set SCRIPTS_ROOT=%UE_ROOT%Engine\Build\BatchFiles\

set PROJECT_PATH=%~dp0..\
set CONTAINER_DIR=ScriptsWin
for %%I in (.) do set PROJECT_NAME=%%~nxI

IF "%PROJECT_NAME%" == "%CONTAINER_DIR%" (
    for %%I in (..) do set PROJECT_NAME=%%~nxI
)

set PROJECT_CONFIG=%PROJECT_PATH%%PROJECT_NAME%.uproject

set PLATFORM=Win64
