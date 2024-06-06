rmdir /s /q ..\bin
msbuild %~dp0Intro\Win\VS2015\Intro.sln /t:Clean,Build /p:Configuration=Release /p:Platform=x64
%~dp0intro\win\installx64.bat
