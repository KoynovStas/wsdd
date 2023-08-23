/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2016, Koynov Stas - skojnov@yandex.ru
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "wsdd_param.h"
#include "net_utils.h"
#include "smacros.h"


// gsoap headers
#include "wsdd.nsmap"
#include "wsddapi.h"





static const char *help_str =
        " ===============  Help  ===============\n"
        " Daemon name:  " DAEMON_NAME          "\n"
        " Daemon  ver:  " DAEMON_VERSION_STR   "\n"
#ifdef  DEBUG
        " Build  mode:  debug\n"
#else
        " Build  mode:  release\n"
#endif
        " Build  date:  " __DATE__ "\n"
        " Build  time:  " __TIME__ "\n\n"
        "Options:                      description:\n\n"
        "       --no_chdir             Don't change the directory to '/'\n"
        "       --no_fork              Don't do fork\n"
        "       --no_close             Don't close standart IO files\n"
        "       --pid_file [value]     Set pid file name\n"
        "       --log_file [value]     Set log file name\n\n"
        "       --if_name  [interface] Set Network Interface for add to multicast group\n"
        "       --endpoint [uuid]      Set UUID for WS-Discovery (default generated a random)\n"
        "       --type     [type]      Set Type of ONVIF service\n"
        "       --scope    [scopes]    Set Scope(s) of ONVIF service\n"
        "       --xaddr    [URL]       Set address (or template URL) of ONVIF service [in template mode %s\n"
        "                              will be changed to IP of interfasec (see opt if_name)]\n"
        "       --metdata_ver [ver]    Set Meta data version of ONVIF service (default = 0)\n"
        "  -v,  --version              Display daemon version\n"
        "  -h,  --help                 Display this help\n\n";



// indexes for long_opt function
enum
{
    cmd_opt_help    = 'h',
    cmd_opt_version = 'v',

    //daemon options
    cmd_opt_no_chdir,
    cmd_opt_no_fork,
    cmd_opt_no_close,
    cmd_opt_pid_file,
    cmd_opt_log_file,

    //WS-Discovery Service options
    cmd_opt_if_name,
    cmd_opt_endpoint,
    cmd_opt_type,
    cmd_opt_scope,
    cmd_opt_xaddr,
    cmd_opt_metdata_ver
};



static const char *short_opts = "hv";


static const struct option long_opts[] =
{
    { "version",      no_argument,       NULL, cmd_opt_version  },
    { "help",         no_argument,       NULL, cmd_opt_help     },

    //daemon options
    { "no_chdir",     no_argument,       NULL, cmd_opt_no_chdir },
    { "no_fork",      no_argument,       NULL, cmd_opt_no_fork  },
    { "no_close",     no_argument,       NULL, cmd_opt_no_close },
    { "pid_file",     required_argument, NULL, cmd_opt_pid_file },
    { "log_file",     required_argument, NULL, cmd_opt_log_file },

    //WS-Discovery Service options
    { "if_name",      required_argument, NULL, cmd_opt_if_name     },
    { "endpoint",     required_argument, NULL, cmd_opt_endpoint    },
    { "type",         required_argument, NULL, cmd_opt_type        },
    { "scope",        required_argument, NULL, cmd_opt_scope       },
    { "xaddr",        required_argument, NULL, cmd_opt_xaddr       },
    { "metdata_ver",  required_argument, NULL, cmd_opt_metdata_ver },

    { NULL,           no_argument,       NULL, 0  }
};





static struct soap*         soap_srv;
static struct wsdd_param_t  wsdd_param;





void free_resources()
{
    //Here we free resources

    send_bye(soap_srv, &wsdd_param);
    soap_wsdd_listen(soap_srv, 1);

    soap_destroy(soap_srv);
    soap_end(soap_srv);

    unlink(daemon_info.pid_file);
}



void daemon_exit_handler(int sig)
{
    UNUSED(sig);
    free_resources();

    exit(EXIT_SUCCESS); // good job (we interrupted (finished) main loop)
}



void init_signals(void)
{
    set_sig_handler(SIGINT,  daemon_exit_handler); //for Ctrl-C in terminal for debug (need --no_fork)
    set_sig_handler(SIGTERM, daemon_exit_handler);

    set_sig_handler(SIGCHLD, SIG_IGN); // ignore child
    set_sig_handler(SIGTSTP, SIG_IGN); // ignore tty signals
    set_sig_handler(SIGTTOU, SIG_IGN);
    set_sig_handler(SIGTTIN, SIG_IGN);
    set_sig_handler(SIGHUP,  SIG_IGN);
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



void get_endpoint(void)
{
    if(wsdd_param.endpoint)
        return; // good job user set endpoint from cmd


    wsdd_param.endpoint = get_rand_endpoint(soap_srv);


    if(!wsdd_param.endpoint)
        daemon_error_exit("Error: cant get random UUID\n");
}



void processing_cmd(int argc, char *argv[])
{
    int opt;

    while( (opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1 )
    {
        switch( opt )
        {

            case cmd_opt_help:
                        puts(help_str);
                        exit_if_not_daemonized(EXIT_SUCCESS);
                        break;

            case cmd_opt_version:
                        puts(DAEMON_NAME "  version  " DAEMON_VERSION_STR "\n");
                        exit_if_not_daemonized(EXIT_SUCCESS);
                        break;


                 //daemon options
            case cmd_opt_no_chdir:
                        daemon_info.no_chdir = 1;
                        break;

            case cmd_opt_no_fork:
                        daemon_info.no_fork = 1;
                        break;

            case cmd_opt_no_close:
                        daemon_info.no_close_stdio = 1;
                        break;

            case cmd_opt_pid_file:
                        daemon_info.pid_file = optarg;
                        break;

            case cmd_opt_log_file:
                        daemon_info.log_file = optarg;
                        break;


                 //WS-Discovery Service options
            case cmd_opt_if_name:
                        wsdd_param.if_name = optarg;
                        break;

            case cmd_opt_endpoint:
                        wsdd_param.endpoint = optarg;
                        break;

            case cmd_opt_type:
                        wsdd_param.type = optarg;
                        break;

            case cmd_opt_scope:
                        wsdd_param.scope = optarg;
                        break;

            case cmd_opt_xaddr:
                        wsdd_param.xaddr = optarg;
                        break;

            case cmd_opt_metdata_ver:
                        wsdd_param.metadata_ver = strtoul(optarg, NULL, 10);
                        break;


            default:
                        puts("for more detail see help\n\n");
                        exit_if_not_daemonized(EXIT_FAILURE);
                        break;
        }
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
    // Note that this IP_ADD_MEMBERSHIP option must be
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
    UNUSED(data);
    init_signals();

    check_param();
    init_gsoap();

    // init static wsdd_param
    get_endpoint();

    send_hello(soap_srv, &wsdd_param);
}



int main(int argc, char *argv[])
{
    processing_cmd(argc, argv);
    daemonize2(init, NULL);


    while( !daemon_info.terminated )
    {
        if(soap_wsdd_listen(soap_srv, 1))
            soap_print_fault(soap_srv, stderr); // report the problem
    }


    free_resources();

    return EXIT_SUCCESS; // good job (we interrupted (finished) main loop)
}
