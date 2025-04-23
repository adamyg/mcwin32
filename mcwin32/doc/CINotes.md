
# ciappveyor integration

## Win32 - Visual Studio 2019 [ci.appveyor.com](https://ci.appveyor.com/project/adamyg/mcwin32-msvc)

```
C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

c:\msys64\usr\bin\pacman --noconfirm -S mingw-w64-x86_64-gettext-tools
c:\msys64\usr\bin\pacman --noconfirm -S zip

cd c:\projects\mcmsvc\mcwin32

git submodule update --init --recursive

set PERL=C:\Strawberry\perl\bin\perl
set PATH=c:\msys64\mingw64\bin;c:\msys64\usr\bin;%PATH%

.\support\vc2019config --inno="C:/Program Files (x86)/Inno Setup 6/iscc"
.\support\gmake-42 release
.\support\gmake-42 release package
```    

The following external tools are required, hence must exist within the path.

- perl
- gzip, unzip (*)
- tar (*)
- which (*)
- dosunix (*)
