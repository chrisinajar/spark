@echo off

setlocal enabledelayedexpansion 

set ORIGINALDIRECTORY=%cd%
set MYBATCHFILEDIRECTORY=%~dp0
pushd
cd "%MYBATCHFILEDIRECTORY%"

rem GOTO SkipBuild

echo ----- Configuring build ----
call "%MYBATCHFILEDIRECTORY%\lmbr_waf.bat" configure
IF ERRORLEVEL 1 GOTO Failed
echo ----- Building game and engine ----
call "%MYBATCHFILEDIRECTORY%\lmbr_waf.bat" build_win_x64_vs2017_release -p game_and_engine
IF ERRORLEVEL 1 GOTO Failed
echo ----- Building shadercachegen ----
call "%MYBATCHFILEDIRECTORY%\lmbr_waf.bat" build_win_x64_vs2017_profile -p shadercachegen
IF ERRORLEVEL 1 GOTO Failed

:SkipBuild

REM Attempt to determine the best BinFolder for rc.exe and AssetProcessorBatch.exe
call "%MYBATCHFILEDIRECTORY%\DetermineRCandAP.bat" SILENT

REM If a bin folder was registered, validate the presence of the binfolder/rc/rc.exe
IF ERRORLEVEL 1 (
    ECHO unable to determine the locations of AssetProcessor and rc.exe.  Make sure that they are available or rebuild from source
    GOTO Failed
)
ECHO Detected binary folder at %MYBATCHFILEDIRECTORY%%BINFOLDER%
rem GOTO SkipPaks

echo ----- Processing Assets Using Asset Processor Batch ----
.\%BINFOLDER%\AssetProcessorBatch.exe /gamefolder=spark /platforms=pc
IF ERRORLEVEL 1 GOTO Failed

echo ----- Creating Packages ----
rem lowercase is intentional, since cache folders are lowercase on some platforms
.\%BINFOLDER%\rc\rc.exe /job=%BINFOLDER%\rc\RCJob_Generic_MakePaks.xml /p=pc /game=spark
IF ERRORLEVEL 1 GOTO Failed

echo ----- Packing up shaders ----
rem
if exist "%MYBATCHFILEDIRECTORY%\Build\" RD /S /Q "%MYBATCHFILEDIRECTORY%\Build\"
call "%MYBATCHFILEDIRECTORY%\lmbr_pak_shaders.bat" spark D3D11 pc
IF ERRORLEVEL 1 GOTO Failed


:SkipPaks

if exist "%MYBATCHFILEDIRECTORY%\WindowsRelease\" RD /S /Q "%MYBATCHFILEDIRECTORY%\WindowsRelease"

mkdir "%MYBATCHFILEDIRECTORY%\WindowsRelease"
mkdir "%MYBATCHFILEDIRECTORY%\WindowsRelease\bin"
xcopy /s "%MYBATCHFILEDIRECTORY%\Build\pc" "%MYBATCHFILEDIRECTORY%\WindowsRelease"
xcopy /s "%MYBATCHFILEDIRECTORY%\spark_pc_paks" "%MYBATCHFILEDIRECTORY%\WindowsRelease"
mkdir "%MYBATCHFILEDIRECTORY%\WindowsRelease\spark\gamemode"
xcopy /s "%MYBATCHFILEDIRECTORY%\spark\gamemode" "%MYBATCHFILEDIRECTORY%\WindowsRelease\spark\gamemode"
mkdir "%MYBATCHFILEDIRECTORY%\WindowsRelease\spark\equ8_client"
xcopy /s "%MYBATCHFILEDIRECTORY%\spark\equ8_client" "%MYBATCHFILEDIRECTORY%\WindowsRelease\spark\equ8_client"
xcopy /s "%MYBATCHFILEDIRECTORY%%BINFOLDER%.Release\*.dll" "%MYBATCHFILEDIRECTORY%\WindowsRelease\bin"
xcopy /s "%MYBATCHFILEDIRECTORY%%BINFOLDER%.Release\*.txt" "%MYBATCHFILEDIRECTORY%\WindowsRelease\bin"
xcopy /s "%MYBATCHFILEDIRECTORY%%BINFOLDER%.Release\sparkLauncher.exe" "%MYBATCHFILEDIRECTORY%\WindowsRelease\bin"

echo pushd %%~dp0\bin >> "%MYBATCHFILEDIRECTORY%\WindowsRelease\spark.bat"
echo .\sparkLauncher.exe +map sandbox >> "%MYBATCHFILEDIRECTORY%\WindowsRelease\spark.bat"

if exist "%MYBATCHFILEDIRECTORY%\Build\" RD /S /Q "%MYBATCHFILEDIRECTORY%\Build\"

echo ----- Done -----
cd "%ORIGINALDIRECTORY%"
exit /b 0

:Failed
popd
cd "%ORIGINALDIRECTORY%"
exit /b 1
