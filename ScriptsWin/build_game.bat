@echo off

call "%~dp0\config.bat"
call "%~dp0\run.bat" pbs

set CLIENT_CONFIG=%1
if not defined CLIENT_CONFIG set CLIENT_CONFIG=Shipping

set ARCHIVE_PATH=%PROJECT_PATH%\Build\Archived
set STAGE_PATH=%PROJECT_PATH%\Build\Stage
set SCRIPT_NAME=RunUAT.bat

"%SCRIPTS_ROOT%%SCRIPT_NAME%" BuildCookRun ^
    -project="%PROJECT_CONFIG%" -platform="%PLATFORM%" -clientconfig="%CLIENT_CONFIG%" ^
    -archivedirectory="%ARCHIVE_PATH%" -stagingdirectory="%STAGE_PATH%" ^
    -build -cook -stage -archive -pak -allmaps
