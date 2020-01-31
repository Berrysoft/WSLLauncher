# GentooWSL
Launcher for Gentoo on WSL.

## Build
The code is modified from [DistroLauncher](https://github.com/microsoft/WSL-DistroLauncher).

`wslapi.lib` is needed. See [WSLInstall](https://github.com/Biswa96/WSLInstall) to generate it.

## Install
Download [stage3](https://www.gentoo.org/downloads/). Use command below to convert it:

```
$ xz -dc stage3.tar.xz | gzip > rootfs.tar.gz
```

Copy `rootfs.tar.gz` with `Gentoo.exe` into the folder you would like to install. Run it.
