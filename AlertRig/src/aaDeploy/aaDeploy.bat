rem 
rem
rem for %%f in ("%1:,=" "%") do copy /f /y %2\%%f %3\%%f
rem for %%f in (%~1) do copy /y %~2\%%f %~3\%%f
rem
rem new stuff here
rem call with command line in Custom Build Step
rem
rem call aaDeploy.bat "dir where bin and demo are"  "full path to output zip file"
rem 
rem I am only copying three exe files and demo/. To add another to deployment zip file
rem just change the 7z command line.
rem

@echo off
pushd %~dp0

rem get output of git describe
for /f %%i in ('git describe') do set VER=%%i
cd %1
7z a %2\alertrig-%VER%.zip bin\fixstim.exe bin\CableTest.exe bin\remote.exe demo

popd