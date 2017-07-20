# Index

1. Introduction
2. Product Summary
3. Use Cases
4. API Specification
5. Notices
6. Restrictions


# 1. Introduction

This document provides the following details of download binding:

- Product Summary
- Use Cases
- API Specification


# 2. Product Summary


## 2.1. Overview

This is a binding for a binder included in AGL.(*1)

![Figure: component](pictures/component.png)

This binding is dynamically loaded libraries in the binder process.

This binding provides an API to download files.

*1) http://docs.automotivelinux.org/docs/apis_services/en/dev/reference/af-binder/afb-overview.html


## 2.2. Terget

This binding supports the following AGL version:

- Charming Chinook

Please refer to ReleaseNotes.md for the tested devices.


## 2.3. Feature

This binding:
- downloads files in parallel.
- controlls to start, stop, resume or cancel donwload.
- deletes a downloaded file.
- gets a download progress.
- supports SSL.

(The following functions supported only in commercial version.)
- limits download speed.
- encrypts a downloaded file.


# 3. Use Cases

This section shows several use cases using this binding.

## 3-1. Download

This is the basic use case to download.
The AGL application can:
- download files from the cloud server.
- download files in parallel.
- controll(stop, resume or cancel) to download.

Optionally:
- a download speed is able to limit.

![Figure: usecase - control](pictures/uc_download.png)


## 3-2. Use the downloaded file

If The AGL application would like to use the downloaded file, it can use the file using other bindings.

If the used file is unnecessary, it can be deleated.

ex) The AGL application downloads an AGL application(.wgt), and installs it in the AGL using the binding that installs a widget file.

![Figure: usecase - use file](pictures/uc_use_file.png)

## 3-3. Encrypt or Decrypt the downloaded file

The AGL application can encrypt the downloaded file, and decrypt the encrypted file.

Only when the AGL application encrypting or decrypting the file, it needs to send a key(a common key). 

![Figure: usecase - use file](pictures/uc_encrypt.png)


# 4. API Specification

## Overview

This binding provides an API with the following name:

- API name : download

and contains the following verbs:

- Verbs

| Verb name   | Description                                                    |
|-------------|----------------------------------------------------------------|
| setting     | set download options                                           |
| download    | download a file                                                |
| info        | get download information                                       |
| stop        | stop to download                                               |
| resume      | resume to download                                             |
| cancel      | cancel to download                                             |
| delete      | delete the downloaded file                                     |
| encrypt     | encrypt or decrypt the downloaded file                         |


# 5. Notices

None

# 6. Restrictions

None



