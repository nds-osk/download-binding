[agl-source-code]: https://wiki.automotivelinux.org/agl-distro/source-code
[meta-afb-bindings]: TBD

# Contents

- [1. Introduction](#1-introduction)
- [2. Installing download binding](#2-installing-download-binding)
    - [Download AGL source code](#download-agl-source-code)
    - [Set up build environment](#set-up-build-environment)
    - [Add yocto layer](#add-yocto-layer)
    - [Append yocto variable](#append-yocto-variable)
    - [Build AGL](#build-agl)
- [3. Using download binding](#3-using-download-binding)


# 1. Introduction

Here are the steps on how to get started with the download binding.


# 2. Installing download binding

## Download AGL source code

Download the AGL source code.
For more information, please check [here][agl-source-code].

Here is an example:
```
repo init -b chinook -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo
repo sync
```

## Set up build environment

Set up the development environment.
For more information, please check [here][agl-source-code].

Here is an example:
```
source meta-agl/scripts/aglsetup.sh -m $MACHINE -b build agl-devel agl-demo agl-appfw-smack
```

## Add yocto layer
Add the yocto layer for the download binding.

Add needed layer to bblayers.conf:
- [meta-afb-bindings][meta-afb-bindings]

## Append yocto variable
Append IMAGE_INSTALL in meta-afb-bindings/recipes-platform/images/agl-demo-platform.bbappend.
```
IMAGE_INSTALL_append = " download-binding"
```

## Build AGL

Build the AGL demo platform.

```
bitbake agl-demo-platform
```


# 3. Using download binding

(TBD)