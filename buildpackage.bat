@echo off
SET COPYCMD=/Y

if [%1] ==[] goto noconfig
if [%2] ==[] goto noplatform

msbuild Engine.vcxproj /p:configuration=%1 /p:platform=%2 || exit /B 1

@echo [92mCopying assets to build folder...[0m
Xcopy /E /I /Q Data Build\%1\Data\
@echo [92mDone Copying assets[0m

@echo.

@echo [92mCopy .dll files to build folder[0m
Xcopy assimp-vc140-mt.dll Build\%1\
@echo [92mDone copying .dll files[0m

exit /B 1

:noconfig
@echo ERROR: No configuration passed in
exit /B 1

:noplatform
@echo ERROR: No platform passed in
exit /B 1