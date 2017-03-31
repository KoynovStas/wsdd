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
