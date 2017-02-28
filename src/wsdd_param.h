#ifndef WSDD_PARAM_H
#define WSDD_PARAM_H





#define WSDD_MULTICAST_IP     "239.255.255.250"
#define WSDD_SERVER_PORT      3702

#define SOAP_WSDD_TS_ADDRESS  "soap.udp://239.255.255.250:3702"





struct wsdd_param_t
{
    const char* if_name;        // bind this interface for Multicast
    const char* endpoint;       // my address - where they can find me for WS-Discovery
    const char* type;
    const char* scope;
    const char* xaddr;

    int         metadata_ver;
};





#endif // WSDD_PARAM_H
