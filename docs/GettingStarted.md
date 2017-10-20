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

Here is an example of the steps on how to call download API.

## Launch afb-daemon

Launch the afb-daemon.

```
# afb-daemon --port=1234 --token=123456 [...]
```

## Connect

Connects with the initial token.

```
$ curl http://$BOARDIP:1234/api/auth/connect?token=123456
{
  "jtype": "afb-reply",
  "request": {
     "status": "success",
     "token": "850c4594-1be1-4e9b-9fcc-38cc3e6ff015",
     "uuid": "0aef6841-2ddd-436d-b961-ae78da3b5c5f"
  },
  "response": {"token": "A New Token and Session Context Was Created"}
}
```

## Call download API

Call the download API.
In the following example, call download/download.

```
$ UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
$ TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
$ curl http://$BOARDIP:1234/api/download/download?uuid=$UUID\&token=$TOKEN\&url="http://www.xxxxxx.co.jp/file"\&filename="sample"
{
  "response": {
    "id": 507
  },
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```
