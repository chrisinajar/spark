@echo off

setlocal enabledelayedexpansion 

set ORIGINALDIRECTORY=%cd%
set MYBATCHFILEDIRECTORY=%~dp0
pushd
cd "%MYBATCHFILEDIRECTORY%"

rem GOTO SkipBuild

echo ----- Building ----
call "%MYBATCHFILEDIRECTORY%\lmbr_waf.bat" configure
IF ERRORLEVEL 1 GOTO Failed
call "%MYBATCHFILEDIRECTORY%\lmbr_waf.bat" build_win_x64_vs2017_profile_dedicated -p game_and_engine
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

:SkipPaks

if exist "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\" RD /S /Q "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows"

mkdir "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows"
mkdir "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\bin"
xcopy /s "%MYBATCHFILEDIRECTORY%\spark_pc_paks" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows"
mkdir "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\spark\gamemode"
xcopy /s "%MYBATCHFILEDIRECTORY%\spark\gamemode" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\spark\gamemode"
mkdir "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\spark\equ8_server"
xcopy /s "%MYBATCHFILEDIRECTORY%\spark\equ8_server" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\spark\equ8_server"
xcopy /s "%MYBATCHFILEDIRECTORY%%BINFOLDER%.Dedicated\*.dll" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\bin"
xcopy /s "%MYBATCHFILEDIRECTORY%%BINFOLDER%.Dedicated\*.txt" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\bin"
xcopy /s "%MYBATCHFILEDIRECTORY%%BINFOLDER%.Dedicated\sparkLauncher_Server.exe" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\bin"

xcopy /s "%MYBATCHFILEDIRECTORY%\Gems\SteamWorks\External\SteamWorks\redistributable_bin\*.exe" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\"
xcopy /s "%MYBATCHFILEDIRECTORY%\Gems\SteamWorks\External\SteamWorks\redistributable_bin\*.dll" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\bin\"
XCOPY /s "%MYBATCHFILEDIRECTORY%\Tools\Redistributables\Visual Studio 2017\VC_redist.x64.exe" "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\"

ECHO mkdir "C:\Program Files (x86)\Steam\" > "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\install.bat"
ECHO VC_redist.x64.exe /q >> "%MYBATCHFILEDIRECTORY%\GameLiftPackageWindows\install.bat"

echo ----- Done -----
cd "%ORIGINALDIRECTORY%"
popd
exit /b 0

:Failed
cd "%ORIGINALDIRECTORY%"
popd
exit /b 1
