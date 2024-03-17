set dirName=gpshell-bundle
del /F /S /Q %dirName% > NUL
rmdir /S /Q %dirName%
mkdir %dirName%

for /f %%f in ('dir /a /b gpshell-binary-*.zip') do set gpshellZip=%%f

tar -xf %gpshellZip% -C %dirName%
tar -xf zlib-1.3.1/zlib-1.3.1.zip -C %dirName%

rem get gpshell directory 
set dirNamegpshell=""
for /f %%d in ('dir /ad /b %dirName%\gpshell-binary*') do set dirNamegpshell=%dirName%\%%d

copy "%dirNamegpshell%\lib\globalplatform.dll" "%dirNamegpshell%\bin"
copy "%dirNamegpshell%\lib\gppcscconnectionplugin.dll" "%dirNamegpshell%\bin"

copy "C:\Program Files (x86)\OpenSSL-Win32\libcrypto*.dll" "%dirNamegpshell%\bin"
copy "C:\Program Files (x86)\OpenSSL-Win32\libssl*.dll" "%dirNamegpshell%\bin"
copy "C:\Program Files (x86)\OpenSSL-Win32\libeay*.dll" "%dirNamegpshell%\bin"
copy "C:\Program Files (x86)\OpenSSL-Win32\bin\legacy*.dll" "%dirNamegpshell%\bin"
copy zlib-1.3.1\win32-build\zlib.dll "%dirNamegpshell%\bin"
rem when build directory is used
copy ..\zlib-1.3.1\win32-build\zlib.dll "%dirNamegpshell%\bin"
rem copy redistributable runtime files
set curDir=%CD%
pushd "%VCToolsRedistDir%%VSCMD_ARG_TGT_ARCH%"
for /r %%a in (vcruntime*.dll) do copy "%%a" "%curDir%\%dirNamegpshell%\bin"
popd

del /F %dirNamegpshell%\doc\CMakeLists.txt > NUL
del /F %dirNamegpshell%\doc\README.md > NUL
del /F /S /Q %dirNamegpshell%\doc\html > NUL
rmdir /s /Q %dirNamegpshell%\doc\html
del /F /S /Q %dirNamegpshell%\include > NUL
rmdir /s /Q %dirNamegpshell%\include
del /F /S /Q %dirNamegpshell%\lib > NUL
rmdir /s /Q %dirNamegpshell%\lib
del /F /S /Q %dirName%\zlib-1.3.1 > NUL
rmdir /s /Q %dirName%\zlib-1.3.1

tar -a -c -C %dirNamegpshell% -f %dirName%\%gpshellZip% *