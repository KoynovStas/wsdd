/*
 *
 * Copyright (c) 2016, Koynov Stas - skojnov@yandex.ru
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1 Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  2 Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  3 Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>


// my headers
#include "daemon.h"
#include "net_utils.h"
#include "wsdd_param.h"


// gsoap headers
#include "wsdd.nsmap"
#include "wsddapi.h"





static const char *help_str =
        " ===============  Help  ===============\n"
        " Daemon name:  %s\n"
        " Daemon  ver:  %d.%d.%d\n"
#ifdef  DEBUG
        " Build  mode:  debug\n"
#else
        " Build  mode:  release\n"
#endif
        " Build  date:  "__DATE__"\n"
        " Build  time:  "__TIME__"\n\n"
        "Options:                      description:\n\n"
        "       --no_chdir             Don't change the directory to '/'\n"
        "       --no_close             Don't close standart IO files\n"
        "       --pid_file [value]     Set pid file name\n"
        "       --log_file [value]     Set log file name\n"
        "       --if_name  [interface] Set Network Interface for add to multicast group\n"
        "       --endpoint [uuid]      Set UUID for WS-Discovery (default generated a random)\n"
        "       --type     [type]      Set Type of ONVIF service\n"
        "       --scope    [scopes]    Set Scope(s) of ONVIF service\n"
        "       --xaddr    [URL]       Set address (or template URL) of ONVIF service [in template mode %s "
        "                              will be changed to IP of interfasec (see opt if_name)]\n"
        "       --metdata_ver [ver]    Set Meta data version of ONVIF service (default = 0)\n"
        "  -v   --version              Display daemon version information\n"
        "  -h,  --help                 Display this information\n\n";



static const char *short_opts = "hv";


static const struct option long_opts[] =
{
    { "version",      no_argument,       NULL, 'v' },
    { "help",         no_argument,       NULL, 'h' },
    { "no_chdir",     no_argument,       NULL,  1  },
    { "no_close",     no_argument,       NULL,  2  },
    { "pid_file",     required_argument, NULL,  3  },
    { "log_file",     required_argument, NULL,  4  },

    // wsdd param
    { "if_name",      required_argument, NULL,  5  },
    { "endpoint",     required_argument, NULL,  6  },
    { "type",         required_argument, NULL,  7  },
    { "scope",        required_argument, NULL,  8  },
    { "xaddr",        required_argument, NULL,  9  },
    { "metdata_ver",  required_argument, NULL,  10 },

    { NULL,           no_argument,       NULL,  0  }
};





struct soap*         soap_srv;
struct wsdd_param_t  wsdd_param;





void send_hello(void)
{
    int res = soap_wsdd_Hello(soap_srv,
                              SOAP_WSDD_ADHOC,             // mode
                              SOAP_WSDD_TS_ADDRESS,        // address of TS
                              soap_wsa_rand_uuid(soap_srv),// message ID
                              NULL,
                              wsdd_param.endpoint,
                              wsdd_param.type,
                              wsdd_param.scope,
                              NULL,
                              wsdd_param.xaddr,
                              wsdd_param.metadata_ver);


    if(res == SOAP_OK)
        soap_wsdd_listen(soap_srv, 1);
    else
        soap_print_fault(soap_srv, stderr);
}



void send_bye(void)
{
    int res = soap_wsdd_Bye(soap_srv,
                            SOAP_WSDD_ADHOC,             // mode
                            SOAP_WSDD_TS_ADDRESS,        // address of TS
                            soap_wsa_rand_uuid(soap_srv),// message ID
                            wsdd_param.endpoint,
                            wsdd_param.type,
                            wsdd_param.scope,
                            NULL,
                            wsdd_param.xaddr,
                            wsdd_param.metadata_ver);


    if(res == SOAP_OK)
        soap_wsdd_listen(soap_srv, 1);
    else
        soap_print_fault(soap_srv, stderr);
}



void daemon_exit_handler(int sig)
{
    //Here we release resources

    unlink(daemon_info.pid_file);

    daemon_info.terminated = 1;    //set flag terminate main loop
}



void init_signals(void)
{

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = daemon_exit_handler;
    if( sigaction(SIGTERM, &sa, NULL) != 0 )
        daemon_error_exit("Can't set daemon_exit_handler: %m\n");



    signal(SIGCHLD, SIG_IGN); // ignore child
    signal(SIGTSTP, SIG_IGN); // ignore tty signals
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
}



void check_param()
{
    if(!wsdd_param.if_name)
        daemon_error_exit("Error: network interface not set (see opt --if_name)\n");

    if(!wsdd_param.type)
        daemon_error_exit("Error: type of ONVIF service not set (see opt --type)\n");

    if(!wsdd_param.scope)
        daemon_error_exit("Error: scope of ONVIF service not set (see opt --scope)\n");

    if(!wsdd_param.xaddr)
        daemon_error_exit("Error: URL of ONVIF service not set (see opt --xaddr)\n");
}



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
void get_xaddr(void)
{
    static char tmp[128];
    char ip[INET_ADDRSTRLEN];


    if( get_ip_of_if(wsdd_param.if_name, AF_INET, ip) != 0 )
    {
        daemon_error_exit("Cant get addr for interface error: %m\n");
    }


    snprintf(tmp, sizeof(tmp), wsdd_param.xaddr, ip);

    wsdd_param.xaddr = tmp;
}



void get_endpoint(void)
{
    static char tmp[48];      //UUID "urn:uuid:xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"

    if(wsdd_param.endpoint)
        return; // good job user set endpoint from cmd


    const char *random_UUID = soap_wsa_rand_uuid(soap_srv);

    if(!random_UUID)
        daemon_error_exit("Error: cant get random UUID\n");


    snprintf(tmp, sizeof(tmp), "%s", random_UUID);

    wsdd_param.endpoint = tmp;
}



void processing_cmd(int argc, char *argv[])
{

    int opt, long_index;



    opt = getopt_long(argc, argv, short_opts, long_opts, &long_index);
    while( opt != -1 )
    {
        switch( opt )
        {

            case 'v':
                        printf("%s  version  %d.%d.%d\n", DAEMON_NAME, DAEMON_MAJOR_VERSION,
                                                                       DAEMON_MINOR_VERSION,
                                                                       DAEMON_PATCH_VERSION);
                        exit_if_not_daemonized(EXIT_SUCCESS);
                        break;

            case 'h':

                        printf(help_str, DAEMON_NAME, DAEMON_MAJOR_VERSION,
                                                      DAEMON_MINOR_VERSION,
                                                      DAEMON_PATCH_VERSION);
                        exit_if_not_daemonized(EXIT_SUCCESS);
                        break;

            case '?':
            case ':':
                        printf("for more detail see help\n\n");
                        exit_if_not_daemonized(EXIT_FAILURE);
                        break;

//            case 0:     // long options
//                        if( strcmp( "name_options", long_opts[long_index].name ) == 0 )
//                        {
//                            //Processing of "name_options"
//                            break;
//                        }

            case 1:     // --no_chdir
                        daemon_info.no_chdir = 1;
                        break;

            case 2:     // --no_close
                        daemon_info.no_close_stdio = 1;
                        break;

            case 3:     // --pid_file
                        daemon_info.pid_file = optarg;
                        break;

            case 4:     // --log_file
                        daemon_info.log_file = optarg;
                        break;


            case 5:     // --if_name
                        wsdd_param.if_name = optarg;
                        break;

            case 6:     // --endpoint
                        wsdd_param.endpoint = optarg;
                        break;

            case 7:     // --type
                        wsdd_param.type = optarg;
                        break;

            case 8:     // --scope
                        wsdd_param.scope = optarg;
                        break;

            case 9:     // --xaddr
                        wsdd_param.xaddr = optarg;
                        break;

            case 10:     // --metdata_ver
                        wsdd_param.metadata_ver = strtoul(optarg, NULL, 10);;
                        break;

            default:
                  break;
        }

        opt = getopt_long(argc, argv, short_opts, long_opts, &long_index);
    }

}



void init_gsoap()
{
    // init gsoap server for WS-Discovery service

    soap_srv = soap_new1(SOAP_IO_UDP);

    if(!soap_srv)
        daemon_error_exit("Can't get mem for soap: %m\n");


    in_addr_t addr               = inet_addr(WSDD_MULTICAST_IP);
    soap_srv->ipv4_multicast_if  = (char *)&addr;  // see setsockopt IPPROTO_IP IP_MULTICAST_IF
    soap_srv->ipv6_multicast_if  = addr;           // multicast sin6_scope_id
    soap_srv->ipv4_multicast_ttl = 1;              // see setsockopt IPPROTO_IP, IP_MULTICAST_TTL
    soap_srv->connect_flags      = SO_BROADCAST;   // for UDP multicast
    soap_srv->bind_flags         = SO_REUSEADDR;


    if(!soap_valid_socket(soap_bind(soap_srv, NULL, WSDD_SERVER_PORT, 100)))
    {
        soap_print_fault(soap_srv, stderr);
        exit(EXIT_FAILURE);
    }


    soap_srv->user = &wsdd_param;  // set our data for handlers



    // Join the multicast group 239.255.255.250 on the local interface
    // interface. Note that this IP_ADD_MEMBERSHIP option must be
    // called for each local interface over which the multicast
    // datagrams are to be received.
    struct ip_mreqn mcast;
    mcast.imr_multiaddr.s_addr = inet_addr(WSDD_MULTICAST_IP);
    if( get_addr_of_if(wsdd_param.if_name, AF_INET, &mcast.imr_address) != 0 )
    {
        daemon_error_exit("Cant get addr for interface error: %m\n");
    }

    setsockopt(soap_srv->master, IPPROTO_IP, IP_MULTICAST_IF, &mcast.imr_address.s_addr, sizeof(struct in_addr));

    if(setsockopt(soap_srv->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast)) != 0 )
    {
        daemon_error_exit("Cant adding multicast group error: %m\n");
    }
}



void init(void *data)
{
    init_signals();

    check_param();
    init_gsoap();

    // init static wsdd_param
    get_xaddr();
    get_endpoint();
}



int main(int argc, char *argv[])
{
    processing_cmd(argc, argv);
    daemonize2(init, NULL);


    send_hello();



    while( !daemon_info.terminated )
    {
        if(soap_wsdd_listen(soap_srv, 1))
            soap_print_fault(soap_srv, stderr); // report the problem
    }



    send_bye();

    soap_destroy(soap_srv);
    soap_end(soap_srv);


    return EXIT_SUCCESS; // good job (we interrupted (finished) main loop)
}
