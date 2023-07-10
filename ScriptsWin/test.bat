@echo off

call "%~dp0\config.bat"

set TEST_OPTION=%1
set SKIP_BUILD=%2

if not defined TEST_OPTION set TEST_OPTION=projectonly

set EDITOR_PATH=%UE_ROOT%\Engine\Binaries\%PLATFORM%\UnrealEditor.exe
set BUILD_SCRIPT_NAME=RunUAT.bat

if "%SKIP_BUILD%" == "skipbuild" goto skipbuild
call "%SCRIPTS_ROOT%%BUILD_SCRIPT_NAME%" BuildCookRun ^
    -project="%PROJECT_CONFIG%" -platform="%PLATFORM%" -clientconfig="Development" ^
    -build -cook
:skipbuild

set FILTER=All
set LOG_DIR=%PROJECT_PATH%Build\Tests\
set LOG_OUTPUT=%LOG_DIR%Tests.log

if "%TEST_OPTION%" == "all" (
    set TESTS_SCOPE_ACTION=RunAll
) else if "%TEST_OPTION%" == "projectonly" (
    set TESTS_SCOPE_ACTION=RunTests %PROJECT_NAME%
) else (
    set TESTS_SCOPE_ACTION=RunTests %TEST_OPTION%
)

"%EDITOR_PATH%" "%PROJECT_CONFIG%" ^
    -ExecCmds="Automation SetFilter %FILTER%; %TESTS_SCOPE_ACTION%; Quit" ^
    -log -abslog="%LOG_OUTPUT%" -nosplash -ReportOutputPath="%LOG_DIR%"
