# Change Log

[**WSDD - ONVIF WS-Discovery server**](https://github.com/KoynovStas/CRC_t) - is Linux daemon for ONVIF WS-Discovery service (server side).

ONVIF official website: [http://www.onvif.org](http://www.onvif.org)

The web services data binding is generated using [gSOAP](http://www.genivia.com)

For more details about it see the [gSOAP WS-Discovery plugin official manual.](https://www.genivia.com/doc/wsdd/html/wsdd_0.html)


License: [BSD-3-Clause](./LICENSE).

Copyright (c) 2016, Koynov Stas - skojnov@yandex.ru



---



## [v2.1](https://github.com/KoynovStas/wsdd/tree/v2.1) (2017-11-23)
[Full Changelog](https://github.com/KoynovStas/wsdd/compare/v2.0...v2.1)


### New

 - update `gsoap` to ver `2.8.55`
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
