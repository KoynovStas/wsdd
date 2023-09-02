# WSDD - ONVIF WS-Discovery server


## Description

**wsdd** is Linux daemon for ONVIF WS-Discovery service (server side).

ONVIF official website: [https://www.onvif.org](https://www.onvif.org)
and their [github presence](https://github.com/onvif/).

The web services data binding is generated using [gSOAP](https://www.genivia.com)

For more details about it see the [gSOAP WS-Discovery plugin official manual.](https://www.genivia.com/doc/wsdd/html/wsdd_0.html)



## Build

#### Dependencies
Most Linux systems for building this project require the following packages to be installed: `make m4 flex bison byacc yacc`

If you need support for encryption and WS-Security then you also need: `openssl zlib libcrypto`


For example, on ubuntu 22.04, you needed to install:
```console
sudo apt install flex bison byacc make cmake m4

#for support encryption and WS-Security
sudo apt install openssl libssl-dev zlib1g-dev libcrypto++8
```


For build use cmake for [CMakeLists.txt](./CMakeLists.txt):

You have 4 build variants:

1. By default, the build takes place in the old style (when there was a `Makefile`)
  We build our own version of `gSOAP` and use it (see [build_gsoap.cmake](./cmake/build_gsoap.cmake)).
  At the same time, we compile the necessary `gSOAP` functions with the project.

```console
cd repo_dir
cmake -B build .
cmake --build build
```


2. Analogue of the 1st, but we use the static library `gSOAP` (we link it)

```console
cd repo_dir
cmake -B build . -DUSE_GSOAP_STATIC_LIB=1
cmake --build build
```


3. We use the system `gSOAP`, for this we use the search module([FindgSOAP.cmake](./cmake/FindgSOAP.cmake)),
 this module should find the `gSOAP` in the system that we will use.

```console
cd repo_dir
cmake -B build . -DUSE_SYSTEM_GSOAP=1
cmake --build build
```

For example, in Ubuntu 22.04, to install `gSOAP`, you need to install the following packages:

```console
sudo apt install gsoap libgsoap-dev
```


4. Analogue of the 3rd, but we tell the [FindgSOAP.cmake](./cmake/FindgSOAP.cmake) search module to look for static libraries instead of shared ones.
  This will allow you not to depend on the `gSOAP` system libraries.

```console
cd repo_dir
cmake -B build . -DUSE_SYSTEM_GSOAP=1 -DUSE_GSOAP_STATIC_LIB=1
cmake --build build
```



## Usage

To start the daemon, you have to give him the parameters that are needed for work:

```console
./wsdd  --if_name eth0 --type tdn:NetworkVideoTransmitter --xaddr http://%s:1000/onvif/device_service --scope "onvif://www.onvif.org/name/Unknown onvif://www.onvif.org/Profile/Streaming"
```

For more details see help:
```console
./wsdd --help
```


Or you can use [S90wsdd](./start_scripts/S90wsdd) script to start the daemon (for old init.d script style):
```console
./start_scripts/S90wsdd start
```

If You use systemd see:
[wsdd.service](./start_scripts/wsdd.service)



## Testing

For testing daemon you need client application.

#### Linux:
1. [ONVIF Device Tool](http://lingodigit.com/onvif_nvc.html)

> **Note**: ONVIF Device Tool has a self wsdd demon and at the start and close of the application makes a call `killall wsdd`, which also is reflected in our demon.


#### Windows:
1. [ONVIF Device Manager](https://sourceforge.net/projects/onvifdm/)



## License

[GPLv2](./LICENSE).



## Copyright

Copyright (C) 2016 Koynov Stas - skojnov@yandex.ru
