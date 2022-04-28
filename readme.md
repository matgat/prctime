## [prctime](https://github.com/matgat/prctime.git)
Measures process execution time
* Uses `GetProcessTimes()` api
* Prints result to `stdout`

_________________________________________________________________________
## Usage
Basic usage:
```
> prctime prctime prg.exe [prg-args ...]
```

Example:
```
> prctime notepad.exe
```


_________________________________________________________________________
## Build
```
> git clone https://github.com/matgat/prctime.git
> cd prctime/msvc
> msbuild prctime.vcxproj -t:prctime -p:Configuration=Release
> cl /std:c++latest /utf-8 /W4 /O2 /D_CRT_SECURE_NO_WARNINGS "source/prctime.cpp" /link /out:prctime.exe
```
