# WSDD - ONVIF WS-Discovery server


## Description

wsdd is Linux daemon for ONVIF WS-Discovery service (server side).

ONVIF official website: [https://www.onvif.org](https://www.onvif.org)
and their [github presence](https://github.com/onvif/).

The web services data binding is generated using [gSOAP](https://www.genivia.com)

For more details about it see the [gSOAP WS-Discovery plugin official manual.](https://www.genivia.com/doc/wsdd/html/wsdd_0.html)


## Build

#### Dependencies
Most Linux systems for building this project require the following packages to be installed: `make m4 flex bison byacc yacc`

If you need support for encryption and WS-Security then you also need: `openssl zlib libcrypto`


For example, on ubuntu 20.04, you needed to install:
```console
sudo apt install flex bison byacc make m4

#for support encryption and WS-Security
sudo apt install openssl libssl-dev libcrypto++6
```

To start build you have to choose your compiler (or toolchain) in the [Makefile](./Makefile) (see variable `$CC`).

For build daemon in release and debug mode:
```console
make all
```

For build daemon in release mode (strip):
```console
make release
```

For build daemon in debug mode (#define DEBUG 1):
```console
make debug
```

Show all supported commands(targets) for Makefile:
```console
make help
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

[BSD-3-Clause](./LICENSE).


## Copyright
Copyright (C) 2016 Koynov Stas - skojnov@yandex.ru
