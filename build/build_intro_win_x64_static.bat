rmdir /s /q ..\bin
msbuild %~dp0Intro\Win\VS2008\Intro.sln /t:Clean,Build /p:Configuration=ReleaseStatic /p:Platform=x64