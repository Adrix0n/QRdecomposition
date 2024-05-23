@FOR /f "tokens=1* delims==" %%a IN ('set') DO @set %%a=
@SET SystemRoot=C:\WINDOWS
@SET SystemDrive=C:
:: mkdir C:\temp
@SET TEMP=C:\temp
 
@IF NOT EXIST %TEMP% mkdir %TEMP%
 
@SET PATH=%PATH%;C:\WINDOWS\system32;C:\WINDOWS
 
:: nalezy dostosowac do swojej wersji
@SET PATH=%PATH%;C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.4\bin
::@SET PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.33.31629\bin\Hostx64\x64
@SET PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64
::@SET PATH=%PATH%;C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.38.33130\bin\Hostx64\x64
@SET LIB=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.0\lib\x64\
@SET INCLUDE=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.0\include
 
@cmd.exe