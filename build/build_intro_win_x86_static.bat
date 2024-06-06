rmdir /s /q ..\bin
msbuild %~dp0Intro\Win\VS2015\Intro.sln /t:Clean,Build /p:Configuration=ReleaseStatic /p:Platform=Win32
