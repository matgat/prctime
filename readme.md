## [prctime](https://github.com/matgat/prctime.git)

Measures process execution time
* Uses `GetProcessTimes()` api
* Prints result to `stdout`

_________________________________________________________________________
## Usage

> [!NOTE]
> Windows binary is dynamically linked to Microsoft c++ runtime,
> so needs the installation of
> [`VC_redist.x64.exe`](https://aka.ms/vs/17/release/vc_redist.x64.exe)
> as prerequisite.

Invocation:

```bat
> prctime prctime prg.exe [prg-args ...]
```

Example:

```bat
> prctime timeout /t 1 /nobreak
```

_________________________________________________________________________
## Build
With Microsoft Visual Studio 2022 (Community Edition),
with `msbuild` visible in path:

```sh
> git clone https://github.com/matgat/prctime.git
> cd prctime
> msbuild build/prctime.vcxproj -t:Rebuild -p:Configuration=Release -p:Platform=x64
```

or

```bat
> python build/build.py
```
