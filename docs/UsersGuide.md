[afb-bindings-writing]: https://gerrit.automotivelinux.org/gerrit/gitweb?p=src/app-framework-binder.git;a=blob;f=doc/afb-bindings-writing.md;h=6327734efab9019e047c7999003a9abbb2eeae6f;hb=refs/heads/chinook
[afb-application-writing]: https://gerrit.automotivelinux.org/gerrit/gitweb?p=src/app-framework-binder.git;a=blob;f=doc/afb-application-writing.md;h=14199f62c5f0778c128d67595b117032e3a7291d;hb=refs/heads/chinook

# Contents

- [1. Introduction](#1-introduction)
- [2. Product Summary](#2-product-summary)
    - [2.1. Overview](#21-overview)
    - [2.2. Feature](#22-feature)
- [3. Use Cases](#3-use-cases)
    - [3-1. Download](#31-download)
    - [3-2. Use the downloaded file](#32-use-the-downloaded-file)
    - [3-3. Decrypt the downloaded file](#33-decrypt-the-downloaded-file)
- [4. API Specification](#4-api-specification)
    - [4.1. Overview](#41-overview)
    - [4.2. Download API](#42-download-api)
        - [download/download](#downloaddownload)

# 1. Introduction

This document provides the following details of download binding:

- [Product Summary](#2-product-summary)
- [Use Cases](#3-use-cases)
- [API Specification](#4-api-specification)


# 2. Product Summary

## 2.1. Overview

This is a binding for a binder included in AGL.(*1)

![Figure: component](pictures/component.png)

This binding is dynamically loaded libraries in the binder process.

This binding provides an API to download files.

*1) http://docs.automotivelinux.org/docs/apis_services/en/dev/reference/af-binder/afb-overview.html


## 2.2. Feature

This binding:
- downloads files in parallel.
- controlls to start, stop, resume or cancel donwload.
- deletes a downloaded file.
- gets a download progress.
- supports SSL.

(The following functions supported only in commercial version.)
- limits download speed.
- decrypts a downloaded file.


# 3. Use Cases

This section shows several use cases using this binding.

## 3.1. Download

This is the basic use case to download.
The AGL application can:
- download files from the cloud server.
- download files in parallel.
- controll(stop, resume or cancel) to download.

Optionally:
- a download speed is able to limit.

![Figure: usecase - control](pictures/uc_download.png)

| Use Case              | How to implement the use case for the Application                             |
|-----------------------|-------------------------------------------------------------------------------|
| start to download     | call [download/download](#downloaddownload)                                   |
| download in parallel  | call [download/download](#downloaddownload) during other download             |
| limit download speed  | call [download/download](#downloaddownload) with the option parameter         |
| get download id       | call download/info                                                            |
| control to download   | call download/stop or resume or cancel                                        |
| stop to download      | call download/stop                                                            |
| resume to download    | call download/resume                                                          |
| cancel to download    | call download/cancel                                                          |


## 3.2. Use the downloaded file

If The AGL application would like to use the downloaded file, it can use the file using other bindings.

If the used file is unnecessary, it can be deleated.

ex) The AGL application downloads an AGL application(.wgt), and installs it in the AGL using the binding that installs a widget file.

![Figure: usecase - use file](pictures/uc_use_file.png)

| Use Case                    | How to implement the use case for the Application |
|-----------------------------|---------------------------------------------------|
| get download progress       | call download/info                                |
| check download is completed | check the download progress is 100%               |
| get file name               | call download/info                                |
| delete file                 | call download/delete                              |
| use file                    | call other API with the file name as a parameter  |


## 3.3. Decrypt the downloaded file

The AGL application can decrypt the downloaded encrypted file.

However, it is a precondition that the public key is exchanged with the cloud server in advance.

![Figure: usecase - decrypt](pictures/uc_decrypt.png)

| Use Case                    | How to implement the use case for the Application |
|-----------------------------|---------------------------------------------------|
| decrypt file                | call download/decrypt                             |


# 4. API Specification

## 4.1. Overview

This binding provides an API with the following name:

- API name : download

and contains the following verbs:

- Verbs

| Verb name                         | Description                                                    |
|-----------------------------------|----------------------------------------------------------------|
| setting                           | set download options                                           |
| [download](#downloaddownload)     | download a file                                                |
| info                              | get download information                                       |
| stop                              | stop to download                                               |
| resume                            | resume to download                                             |
| cancel                            | cancel to download                                             |
| delete                            | delete the downloaded file                                     |
| decrypt                           | decrypt the downloaded encrypted file                          |


## 4.2. Download API

These are the download API Verbs specification.

The following is a description of each item:

- Resource URL

  This is URL for calling the verb.

- Session Constant

  This is ["authorisation and session requirements of the method"][afb-bindings-writing].

- Parameters

  These are parameters with calling the verb.

- Responses

    - Sucsess

      This is a response that will be returned when the verb succeeds.
      This is the response object included in the [afb-reply][afb-application-writing],
      when request.status is "sucsess".

    - Failure

      This is a response that will be returned when the verb fails.
      This is the request.info message included in the [afb-reply][afb-application-writing],
      when request.status is "failed".

- Example Request

  This is a request example.

- Example Response

  This is a response example.


### download/download

Download a file from the cloud server.

#### ***Resource URL***

http://$BOARDIP:$PORT/download/download

#### ***Session Constant***

AFB_SESSION_CHECK

#### ***Parameters***

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| url         | required | string   | file's URL                             | none          |
| filename    | required | string   | output file name                       | none          |
| max_speed   | optional | number   | max download speed (bytes per second)  | 0(unlimited)  |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| url         | maximum length: 2083                       |
| filename    | maximum length: 255                        |
| max_speed   | range: 0(unlimited) - 104857600(100Mbps)   |

#### ***Responses***

- Sucsess

| Name        | Type     | Description                            |
|-------------|----------|----------------------------------------|
| id          | number   | the ID of the download                 |

- Failure

| Message                          |
|----------------------------------|
| url is invalid value             |
| filename is invalid value        |
| filename already exists          |
| max_speed is invalid value       |


#### ***Example Request***

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/download/download?uuid=$UUID\&token=$TOKEN\&url="http://www.xxxxxx.co.jp/file"\&filename="sample"
```

#### ***Example Response***

```
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


---