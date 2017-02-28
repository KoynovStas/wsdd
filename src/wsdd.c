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

    { NULL,           no_argument,       NULL,  0  }
};





struct soap*         soap_srv;
struct wsdd_param_t  wsdd_param;





void daemon_exit_handler(int sig)
{

    //Here we release resources


    unlink(daemon_info.pid_file);

    _exit(EXIT_FAILURE);
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

            default:
                  break;
        }

        opt = getopt_long(argc, argv, short_opts, long_opts, &long_index);
    }

}



void init(void *data)
{
    init_signals();



    // init gsoap server for WS-Discovery service
    soap_srv = soap_new1(SOAP_IO_UDP);

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



    // Join the multicast group 239.255.255.250 on the local interface
    // interface. Note that this IP_ADD_MEMBERSHIP option must be
    // called for each local interface over which the multicast
    // datagrams are to be received.
    struct ip_mreq mcast;
    mcast.imr_multiaddr.s_addr = inet_addr(WSDD_MULTICAST_IP);
    if( get_addr_of_if(wsdd_param.if_name, AF_INET, &mcast.imr_interface) != 0 )
    {
        daemon_error_exit("Cant get addr for interface error: %m\n");
    }

    if(setsockopt(soap_srv->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mcast, sizeof(mcast)) < 0)
    {
        daemon_error_exit("Cant adding multicast group error: %m\n");
    }

}



int main(int argc, char *argv[])
{

    processing_cmd(argc, argv);
    daemonize2(init, NULL);



    while( !daemon_info.terminated )
    {

        // Here а routine of daemon

        printf("%s: daemon is run\n", DAEMON_NAME);
        sleep(10);
    }



    return EXIT_SUCCESS; // good job (we interrupted (finished) main loop)
}
