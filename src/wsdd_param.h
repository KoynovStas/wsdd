#ifndef WSDD_PARAM_H
#define WSDD_PARAM_H

#include <stdint.h>


#include "wsddapi.h"





#define WSDD_MULTICAST_IP     "239.255.255.250"
#define WSDD_SERVER_PORT      3702

#define SOAP_WSDD_TS_ADDRESS  "soap.udp://239.255.255.250:3702"





struct wsdd_param_t
{
    const char* if_name;        // bind this interface for Multicast
    const char* endpoint;       // my address - where they can find me for WS-Discovery
    const char* type;           // type of ONVIF service
    const char* scope;          // scope(s) of ONVIF service
    const char* xaddr;          // address of ONVIF service

    uint32_t    metadata_ver;
};




const char* get_xaddr(struct wsdd_param_t *wsdd_param);
const char* get_rand_endpoint(struct soap *soap_srv);


void send_hello(struct soap *soap_srv, struct wsdd_param_t *wsdd_param);
void send_bye(struct soap *soap_srv, struct wsdd_param_t *wsdd_param);





#endif // WSDD_PARAM_H
