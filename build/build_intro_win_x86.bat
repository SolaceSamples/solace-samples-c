rmdir /s /q ..\bin
msbuild %~dp0Intro\Win\VS2008\Intro.sln /t:Clean,Build /p:Configuration=Release /p:Platform=Win32
%~dp0intro\win\installx86.bat