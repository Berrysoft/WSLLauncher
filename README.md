# WSLLauncher
Launcher for distributions on WSL.

## Build
The code is modified from [DistroLauncher](https://github.com/microsoft/WSL-DistroLauncher).

Use command below to generate a launcher for a distro:
```
PS> dotnet publish -f netcoreapp3.1 -c Release -p:AssemblyName=<DistroName> -p:ApplicationIcon=<IconPath>
```

## Install
Get a system image named `rootfs.tar.gz`. Copy `rootfs.tar.gz` with `<DistroName>.exe` and other files into the folder you would like to install.
Run command below to install:
```
PS> .\<DistroName>.exe install --file rootfs.tar.gz
```
