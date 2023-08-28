# Change Log

[**WSDD - ONVIF WS-Discovery server**](https://github.com/KoynovStas/wsdd) - is Linux daemon for ONVIF WS-Discovery service (server side).

ONVIF official website: [http://www.onvif.org](http://www.onvif.org)

The web services data binding is generated using [gSOAP](http://www.genivia.com)

For more details about it see the [gSOAP WS-Discovery plugin official manual.](https://www.genivia.com/doc/wsdd/html/wsdd_0.html)


License: [GPLv2](./LICENSE).

Copyright (c) 2016, Koynov Stas - skojnov@yandex.ru



---



## [v3.0](https://github.com/KoynovStas/wsdd/tree/v3.0) (2023-08-28)
[Full Changelog](https://github.com/KoynovStas/wsdd/compare/v2.1...v3.0)


### New

 - **changed** license to **GPLv2** (gsoap requirements) see: [issue 8](https://github.com/KoynovStas/wsdd/issues/8)
 - update `gsoap` to ver `2.8.66` (was `2.8.55`)
 - add `systemd` script
 - add `smacros.h` - list of simple macros
 - add `set_sig_handler` function
 - add process Ctrl-C in terminal(`SIGINT` signal) for debug
 - add various `PATCH_VERSION` processing options


### Refactoring

 - now we can set extern toolchain (compiller)
 - update mirrors for `gSOAP` arhive


### BugFix

 - Some need to set the interface index for multicast see [issue 1](https://github.com/KoynovStas/wsdd/issues/1)
 - Fixed a potential stack overflow see [PR 3](https://github.com/KoynovStas/wsdd/pull/3)
 - add `soapcpp2_lex.l.patch` For more details see: https://sourceforge.net/p/gsoap2/bugs/1269/




## [v2.1](https://github.com/KoynovStas/wsdd/tree/v2.1) (2017-11-23)
[Full Changelog](https://github.com/KoynovStas/wsdd/compare/v2.0...v2.1)


### New

 - update `gsoap` to ver `2.8.55` (was `2.8.44`)
 - use `daemon.c` ver 1.1




## [v2.0](https://github.com/KoynovStas/wsdd/tree/v2.0) (2017-04-03)
[Full Changelog](https://github.com/KoynovStas/wsdd/compare/v1.0...v2.0)


### New

 - `get_xaddr` in dynamic (was only static)
 - Uses new ver of `gsoap 2.8.44` (was `2.8.43`)


### Refactoring

 - support parallel make (see opt -j)
 - add func `free_resources()`
 - move call `send_hello` to `init()`
 - func `send_hello/bye`
 - add func `init_gsoap()`
 - add space in static help need for C++11
 - `process_cmd` func, add enum for cmd options


### BugFix

 - Need use struct `ip_mreqn` not `ip_mreq`




## [v1.0](https://github.com/KoynovStas/wsdd/tree/v1.0) (2017-03-03)
