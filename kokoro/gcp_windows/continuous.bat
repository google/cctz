echo on
set PATH=C:\Program Files\Java\jdk1.7.0_75\bin;%PATH%

pwd

cd %KOKORO_PIPER_DIR%\google3\experimental\users\misterg\cctz

call build.bat

echo off
exit %ERRORLEVEL%
