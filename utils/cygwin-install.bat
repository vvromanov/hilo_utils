@echo off
SETLOCAL
SET CUR_DIR=%~dp0
call %CUR_DIR%\..\cygwin-path.bat
            
REM -- Configure our paths
SET SITE=http://mirrors.163.com/cygwin
 
REM -- Change to the directory of the executing batch file
CD %CUR_DIR%
mkdir %CYGWIN_PATH%

REM -- Download the Cygwin installer
IF NOT EXIST %CYGWIN_PATH%/cygwin-setup.exe (
	ECHO cygwin-setup.exe NOT found! Downloading installer...
	bitsadmin /transfer cygwinDownloadJob /download /priority FOREGROUND https://cygwin.com/setup-x86_64.exe %CYGWIN_PATH%\cygwin-setup.exe
) ELSE (
	ECHO cygwin-setup.exe found! Skipping installer download...
)
 
REM -- These are the packages we will install (in addition to the default packages)
SET PACKAGES=make,gdb,gcc-g++,gcc-core,cmake,doxygen,libev-devel,libminizip-devel,libxml2-devel,libxslt-devel,python39-devel,mc
REM gcovr,python39-devel,libxslt-devel
 
ECHO *** INSTALLING
%CYGWIN_PATH%\cygwin-setup --quiet-mode --no-desktop --download --local-install --no-admin --no-verify --upgrade-also --local-package-dir "%TEMP%/cygwin_packages" --site %SITE% --root %CYGWIN_PATH% -P %PACKAGES%

ECHO *** INSTALLING gcovr
%CYGWIN_PATH%\bin\bash --login -c "pip3 install --upgrade pip"
%CYGWIN_PATH%\bin\bash --login -c "python3.9 -m pip install gcovr"
del /Q %CYGWIN_PATH%\var\log\*
ENDLOCAL
EXIT /B 0