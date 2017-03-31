#include <stdio.h>


#include "wsdd_param.h"
#include "net_utils.h"





/*
 * This function converts the template parameter (option) --xaddr
 * to final URL for the ONVIF service. If this option is specified on the command line
 * does not contain a template parameter %s function snprintf() just make a copy in a static array.
 * If option contains template parameter %s, it will be replaced by
 * IP addres of Network interface specified by --if_name option.
 *
 * Example:
 * ./wsdd --xaddr "http://192.168.1.1:2000/onvif/device_service" - will not be changed
 *
 * ./wsdd --xaddr "http://%s:2000/onvif/device_service" --if_name eth1
 * template %s will be replaced to the IP address of the network interface eth1
*/
const char* get_xaddr(struct wsdd_param_t *wsdd_param)
{
    static char xaddr[128];
    char ip[INET_ADDRSTRLEN];


    if( get_ip_of_if(wsdd_param->if_name, AF_INET, ip) == 0 )
    {
        snprintf(xaddr, sizeof(xaddr), wsdd_param->xaddr, ip);
    }
    else
    {
        snprintf(xaddr, sizeof(xaddr), wsdd_param->xaddr, "127.0.0.1");
    }


    return xaddr;
}



const char* get_rand_endpoint(struct soap *soap_srv)
{
    static char tmp[48];      //UUID "urn:uuid:xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"

    const char *random_UUID = soap_wsa_rand_uuid(soap_srv);

    if(!random_UUID)
        return NULL;

    snprintf(tmp, sizeof(tmp), "%s", random_UUID);

    return tmp;
}



void send_hello(struct soap *soap_srv, struct wsdd_param_t *wsdd_param)
{
    int res = soap_wsdd_Hello(soap_srv,
                              SOAP_WSDD_ADHOC,             // mode
                              SOAP_WSDD_TS_ADDRESS,        // address of TS
                              soap_wsa_rand_uuid(soap_srv),// message ID
                              NULL,
                              wsdd_param->endpoint,
                              wsdd_param->type,
                              wsdd_param->scope,
                              NULL,
                              get_xaddr(wsdd_param),
                              wsdd_param->metadata_ver);


    if(res == SOAP_OK)
        soap_wsdd_listen(soap_srv, 1);
    else
        soap_print_fault(soap_srv, stderr);
}



void send_bye(struct soap *soap_srv, struct wsdd_param_t *wsdd_param)
{
    int res = soap_wsdd_Bye(soap_srv,
                            SOAP_WSDD_ADHOC,             // mode
                            SOAP_WSDD_TS_ADDRESS,        // address of TS
                            soap_wsa_rand_uuid(soap_srv),// message ID
                            wsdd_param->endpoint,
                            wsdd_param->type,
                            wsdd_param->scope,
                            NULL,
                            get_xaddr(wsdd_param),
                            wsdd_param->metadata_ver);


    if(res == SOAP_OK)
        soap_wsdd_listen(soap_srv, 1);
    else
        soap_print_fault(soap_srv, stderr);
}
