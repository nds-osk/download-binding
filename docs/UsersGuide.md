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
        - [download/init](#downloadinit)
        - [download/setting](#downloadsetting)
        - [download/download](#downloaddownload)
        - [download/info](#downloadinfo)
        - [download/stop](#downloadstop)
        - [download/resume](#downloadresume)
        - [download/cancel](#downloadcancel)
        - [download/delete](#downloaddelete)
        - [download/decrypt](#downloaddecrypt)

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
- limits number of downloading in parallel in the system. (This is implemented in the future.)
- limits total save size per binder. (This is implemented in the future.)
- manages files retention period per binder.
- keeps the download information per session, when system rebooted.
- limits download speed.
- enables/disables HTTP redirect. (This is implemented in the future.)
- decrypts a downloaded file.

## 2.3. Note

- The supported protocols are as follows:
    - http (1.1 or more)
- If you shut down the system in an incorrect procedure, the download files may disappear.

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

| Use Case              | How to implement the use case for the Application                                             |
|-----------------------|-----------------------------------------------------------------------------------------------|
| start to download     | call [download/download](#downloaddownload)                                                   |
| download in parallel  | call [download/download](#downloaddownload) during other download                             |
| limit download speed  | call [download/download](#downloaddownload) with the option parameter                         |
| get download id       | call [download/info](#downloadinfo)                                                           |
| control to download   | call [download/stop](#downloadstop) or [resume](#downloadresume) or [cancel](#downloadcancel) |
| stop to download      | call [download/stop](#downloadstop)                                                           |
| resume to download    | call [download/resume](#downloadresume)                                                       |
| cancel to download    | call [download/cancel](#downloadcancel)                                                       |


## 3.2. Use the downloaded file

If The AGL application would like to use the downloaded file, it can use the file using other bindings.

If the used file is unnecessary, it can be deleated.

ex) The AGL application downloads an AGL application(.wgt), and installs it in the AGL using the binding that installs a widget file.

![Figure: usecase - use file](pictures/uc_use_file.png)

| Use Case                    | How to implement the use case for the Application |
|-----------------------------|---------------------------------------------------|
| get download progress       | call [download/info](#downloadinfo)               |
| check download is completed | check the download progress is 100%               |
| get file name               | call [download/info](#downloadinfo)               |
| delete file                 | call [download/delete](#downloaddelete)           |
| use file                    | call other API with the file name as a parameter  |


## 3.3. Decrypt the downloaded file

The AGL application can decrypt the downloaded encrypted file.

However, it is a precondition that the public key is exchanged with the cloud server in advance.

![Figure: usecase - decrypt](pictures/uc_decrypt.png)

| Use Case                    | How to implement the use case for the Application |
|-----------------------------|---------------------------------------------------|
| decrypt file                | call [download/decrypt](#downloaddecrypt)         |


### Use case example

- Cloud Server Side

1. Create the AES common key(128bit) and the Initial vector.
```
$ openssl rand 16 -hex > file.key
$ openssl rand 16 -hex > file.iv
```

2. Encrypt the file.
```
KEY=`cat file.key`
IV=`cat file.iv`
openssl aes-128-cbc -e -in file.txt -out file.txt.enc -K $KEY -iv $IV -nosalt
```

3. Encrypt the AES common key using the client's public key.
```
openssl rsautl -encrypt -pubin -inkey public-key.pem -in file.key -out file.ek
```

- Client Side

4. Call [download/download](#downloaddownload) to download the following files from the cloud server.
    - file.ek
    - file.iv
    - file.txt.enc

5. Call [download/decrypt](#downloaddecrypt) to decrypt the file.ek using the private key and get the AES common key.

6. Call [download/decrypt](#downloaddecrypt) to decrypt the file.txt.enc using the AES common key and the Initial vector(file.iv).


# 4. API Specification

## 4.1. Overview

This binding provides an API with the following name:

- API name : download

and contains the following verbs:

- Verbs

| Verb name                         | Description                                                    |
|-----------------------------------|----------------------------------------------------------------|
| [init](#downloadinit)             | initialize the download information.                           |
| [setting](#downloadsetting)       | set download options                                           |
| [download](#downloaddownload)     | download a file                                                |
| [info](#downloadinfo)             | get download information                                       |
| [stop](#downloadstop)             | stop to download                                               |
| [resume](#downloadresume)         | resume to download                                             |
| [cancel](#downloadcancel)         | cancel to download                                             |
| [delete](#downloaddelete)         | delete the downloaded file                                     |
| [decrypt](#downloaddecrypt)       | decrypt the downloaded encrypted file                          |


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


---

### download/init

Initialize the download information.

The download information (session context) is stored in the system.
Therefore, it keeps the download information per session, when system rebooted.

It is must be called first.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/init

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

| Name          | Required | Type     | Description                                                 | Default Value |
|---------------|----------|----------|-------------------------------------------------------------|---------------|
| ctx_id        | optional | string   | The ID for the download information (session context) (*1)  | none          |

| Name          | Validation                                 |
|---------------|--------------------------------------------|
| ctx_id        | none                                       |


*1) If you set the ID, it reads download informaton of the ID stored in the system.
If not, initializes the download information and gets the ID.

#### *Responses*

- Sucsess

| Name          | Type     | Description                            |
|---------------|----------|----------------------------------------|
| ctx_id        | string   | The ID for the download information    |


- Failure

| Message                          |
|----------------------------------|
| already initialized              |
| ctx_id=%s does not exist         |


#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/init?uuid=$UUID\&token=$TOKEN
```

#### *Example Response*

```
{
  "response": {
    "ctx_id": "ffffffff"
  },
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```
```
{
  "jtype": "afb-reply",
  "request": {
     "status": "failed",
     "info": "ctx_id is no exists"
  }
}
```


---


### download/setting

Set or Get default values of download options.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/setting

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

If you set no parameter, you only get default values.

| Name          | Required | Type     | Description                            | Default Value |
|---------------|----------|----------|----------------------------------------|---------------|
| max_speed     | optional | number   | max download speed (bytes per second)  | 0(unlimited)  |
| retention_period (*3) | optional | number   | files retention period per binder (days) | 0(indefinite) |
| redirect      | optional | number   | HTTP redirect enable(*1)/disable       | 0(disable)    |
| max_save_size | optional | number   | max save size per binder (*2)          | 0(unlimited)  |


| Name          | Validation                                 |
|---------------|--------------------------------------------|
| max_speed     | range: 0(unlimited) - 104857600(100Mbps)   |
| retention_period | 0 - 365                                 |
| redirect      | 0(disable) or 1(enable)                    |
| max_save_size | 0(unlimited) - 4,294,967,295               |

*1) follow any Location: header that the server sends as part of a HTTP header in a 3xx response.
If redirect is disabled and the binding recieves 3XX response, the binding downloads the html text(message body).

*2) Actually, it slightly exceeds setting value.

*3) The files will be deleted when retention_period days passes since the last modify time.
Those are the download files and download information files.

#### *Responses*

- Sucsess

The response includes default values of download options.

| Name          | Type     | Description                            |
|---------------|----------|----------------------------------------|
| max_speed     | number   | max download speed (bytes per second)  |
| retention_period | number   | files retention period per binder (days) |
| redirect      | number   | HTTP redirect enable/disable           |
| max_save_size | number   | max save size per binder               |

- Failure

| Message                          |
|----------------------------------|
| max_speed is invalid value       |
| retention_period is invalid value|
| redirect is invalid value        |
| max_save_size is invalid value   |
| download/init is must be called first |

#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/setting?uuid=$UUID\&token=$TOKEN\&max_speed=104857600
```

#### *Example Response*

```
{
  "response": {
    "max_speed": 104857600,
    "retention_period": 365,
    "redirect": 1,
    "max_save_size": 4294967295
  },
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```
```
{
  "jtype": "afb-reply",
  "request": {
     "status": "failed",
     "info": "max_speed is invalid value"
  }
}
```


---


### download/download

Download a file from the cloud server.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/download

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| url         | required | string   | file's URL                             | none          |
| filename    | required | string   | output file name                       | none          |
| max_speed   | optional | number   | max download speed (bytes per second)  | 0(unlimited)  |
| redirect    | optional | number   | HTTP redirect enable(*1)/disable       | 0(disable)    |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| url         | maximum length: 2083                       |
| filename    | maximum length: 255                        |
| max_speed   | range: 0(unlimited) - 104857600(100Mbps)   |


#### *Responses*

- Sucsess

| Name        | Type     | Description                            |
|-------------|----------|----------------------------------------|
| id          | number   | the ID of the download                 |

- Failure

| Message                          |
|----------------------------------|
| url is NULL                      |
| url is invalid value             |
| filename is NULL                 |
| filename is invalid value        |
| filename already exists          |
| max_speed is invalid value       |
| redirect is invalid value        |
| download/init is must be called first |
| max downloadable number in pararell in the system is (max_download_num) |

#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/download?uuid=$UUID\&token=$TOKEN\&url="http://www.xxxxxx.co.jp/file"\&filename="sample"
```

#### *Example Response*

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


### download/info

Get the download information or all download information list.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/info

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

When you set no parameter, you get all download information list.

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| id          | optional | number   | the ID of the download                 | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| id          | range: 0 - 999                             |

#### *Responses*

- Sucsess

The response includes the download information.

When the id is specified:

| Name        | Type     | Description                                  |
|-------------|----------|----------------------------------------------|
| downloading | object   | the download information                     |

When you set no parameter:

| Name         | Type                                    | Description                                  |
|--------------|-----------------------------------------|----------------------------------------------|
| downloadings | array (contains the downloading object) | all download information list                |
| system_info  | object                                  | system information about download            |
| save_size    | number                                  | total save size per binder                   |

- Failure

| Message                          |
|----------------------------------|
| download/init is must be called first |
| max_speed is invalid value       |
| max save size is (max_save_size) |

##### object definitions

- downloading

| Name        | Type     | Description                                                                          |
|-------------|----------|--------------------------------------------------------------------------------------|
| id          | number   | the ID of the download                                                               |
| url         | string   | file's URL                                                                           |
| filename    | string   | output file name                                                                     |
| size_total  | number   | total size of file to be downloaded [byte] (*3)                                      |
| size        | number   | current file size [byte]                                                             |
| max_speed   | number   | max download speed (bytes per second)                                                |
| redirect    | number   | HTTP redirect enable/disable                                                         |
| state       | number   | state of download (0:pause 1:running 2:done(*2) 3:error(*1))                         |
| progress    | number   | progress of download (%)                                                             |
| error_type  | string   | set error type when state becomes 3:error. Details are [here](#error-information).   |
| error_code  | number   | set error code when state becomes 3:error. Details are [here](#error-information).   |
| response_code | number | get the last HTTP response code (only when state of download is 2:done or 3:error)   |

*1) When the error occured, please call [download/delete](#downloaddelete) if you want to delete the download information.

*2) If the state is done, the downloaded file is not always what you intended. Please check the response_code to judge it.

*3) It is -1 until the binding gets the total size from the cloud server.

- system_info

| Name              | Type     | Description                                                                          |
|-------------------|----------|--------------------------------------------------------------------------------------|
| max_download_num  | number   | max downloadable number in pararell in the system (default is 6)(*1)                 |
| download_num      | number   | number of downloading in parallel in the system                                      |

*1) you can setting only in header file.

###### error information

| error_type  | error_code                                                                        | Description                            |
|-------------|-----------------------------------------------------------------------------------|----------------------------------------|
| curl        | see [CURLcode error code](https://curl.haxx.se/libcurl/c/libcurl-errors.html)     | error information in libcurl           |

#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/info?uuid=$UUID\&token=$TOKEN
```

#### *Example Response*

```
{
  "response": {
    "downloadings": [
      {
        "id": 39,
        "state": 1,
        "progress": 80,
        "url": "http://www.xxxxxx.co.jp/file1",
        "filename": "test2",
        "max_speed": 0
      },
      {
        "id": 507,
        "state": 2,
        "progress": 100,
        "url": "http://www.xxxxxx.co.jp/file2",
        "filename": "test1"
        "max_speed": 104857600
      }
    ]
  },
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```


---


### download/stop

Stop to download.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/stop

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| id          | required | number   | the ID of the download                 | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| id          | range: 0 - 999                             |


#### *Responses*

- Sucsess

None.

- Failure

| Message                            |
|------------------------------------|
| id is NULL                         |
| id is invalid value                |
| download/init is must be called first |
| id=%d is no exist                  |
| must call when state is 1:running  |


#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/stop?uuid=$UUID\&token=$TOKEN\&id=507
```

#### *Example Response*

```
{
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```


---


### download/resume

Resume to download.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/resume

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| id          | required | number   | the ID of the download                 | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| id          | range: 0 - 999                             |


#### *Responses*

- Sucsess

None.


- Failure

| Message                          |
|----------------------------------|
| id is NULL                       |
| id is invalid value              |
| download/init is must be called first |
| id=%d is no exist                |
| must call when state is 0:pause  |


#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/resume?uuid=$UUID\&token=$TOKEN\&id=507
```


#### *Example Response*

```
{
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```


---


### download/cancel

Cancel to download, and delete the downloaded file and the download information.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/cancel

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| id          | required | number   | the ID of the download                 | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| id          | range: 0 - 999                             |


#### *Responses*

- Sucsess

None.


- Failure

| Message                                        |
|------------------------------------------------|
| id is NULL                                     |
| id is invalid value                            |
| download/init is must be called first          |
| id=%d is no exist                              |
| must call when state is 0:pause or 1:running   |


#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/cancel?uuid=$UUID\&token=$TOKEN\&id=507
```


#### *Example Response*

```
{
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```


---


### download/delete

Delete the downloaded file and the download information.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/delete

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| id          | required | number   | the ID of the download                 | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| id          | range: 0 - 999                             |


#### *Responses*

- Sucsess

None.


- Failure

| Message                                        |
|------------------------------------------------|
| id is NULL                                     |
| id is invalid value                            |
| download/init is must be called first          |
| id=%d is no exist                              |
| must call when state is 2:done or 3:error      |


#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/delete?uuid=$UUID\&token=$TOKEN\&id=507
```


#### *Example Response*

```
{
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```


---


### download/decrypt

Decrypt the downloaded encrypted file using the openssl.

The decrypted file is overwritten to the original file.

#### *Resource URL*

http://$BOARDIP:$PORT/api/download/decrypt

#### *Session Constant*

AFB_SESSION_CHECK

#### *Parameters*

##### Common

| Name        | Required | Type     | Description                              | Default Value |
|-------------|----------|----------|------------------------------------------|---------------|
| id          | required | number   | the ID of the download.(to be decrypted) | none          |
| crypto      | required | number   | cryptography(0:RSA 1:AES)                | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| id          | range: 0 - 999                             |
| crypto      | range: 0 - 1                               |

##### if crypto is 0:RSA

| Name        | Required | Type     | Description                            | Default Value |
|-------------|----------|----------|----------------------------------------|---------------|
| key(*1)     | required | string   | file path of the private key (.pem)    | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| key         | maximum length: 255                        |

##### if crypto is 1:AES

| Name        | Required | Type     | Description                                 | Default Value |
|-------------|----------|----------|---------------------------------------------|---------------|
| key_length  | required | number   | key length (0:128 1:192 2:256)              | none          |
| mode        | required | number   | block cipher mode (0:CBC)                   | none          |
| key(*1)     | required | string   | file path of the common key (*2)            | none          |
| iv(*1)      | required | string   | file path of the initialization vector (*2) | none          |

| Name        | Validation                                 |
|-------------|--------------------------------------------|
| key_length  | range: 0 - 2                               |
| mode        | only 0                                     |
| key         | maximum length: 255                        |
| iv          | maximum length: 255                        |


*1) full path or downloaded file name.

*2) these files are composed of HEX character string. Here is an example:
```
$ cat file.key
46496a7b83339b7f01f757b333a8a1f17e9f491521476c21
$ cat file.iv
c39474ac9120ec3a95359b8cc28fc260
```

#### *Responses*

- Sucsess

None.

- Failure

##### if crypto is 0:RSA

| Message                                        |
|------------------------------------------------|
| id is NULL                                     |
| id is invalid value                            |
| crypto is NULL                                 |
| crypto is invalid value                        |
| key is NULL                                    |
| key is invalid value                           |
| download/init is must be called first          |
| id=%d is no exist                              |
| must call when state is 2:done                 |
| can't read key file                            |
| failed to decrypt                              |


##### if crypto is 1:AES

| Message                                        |
|------------------------------------------------|
| id is NULL                                     |
| id is invalid value                            |
| crypto is NULL                                 |
| crypto is invalid value                        |
| key_length is NULL                             |
| key_length is invalid value                    |
| mode is NULL                                   |
| mode is invalid value                          |
| key is NULL                                    |
| key is invalid value                           |
| iv is NULL                                     |
| iv is invalid value                            |
| download/init is must be called first          |
| id=%d is no exist                              |
| must call when state is 2:done                 |
| can't read key file                            |
| can't read iv file                             |
| failed to decrypt                              |

#### *Example Request*

```
BOARDIP="192.168.x.x"
PORT=1234
UUID="850c4594-1be1-4e9b-9fcc-38cc3e6ff015"
TOKEN="0aef6841-2ddd-436d-b961-ae78da3b5c5f"
curl http://$BOARDIP:$PORT/api/download/decrypt?uuid=$UUID\&token=$TOKEN\&id=507\&crypto=0\&key="/tmp/private-key.pem"
```


#### *Example Response*

```
{
  "jtype": "afb-reply",
  "request": {
    "status": "success"
  }
}
```
