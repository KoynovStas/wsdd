#ifndef NET_UTILS_HEADER
#define NET_UTILS_HEADER


#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>





struct socket_param_t
{
    const char *host_or_IP;   // "google.com"  or "192.168.1.1"
    uint16_t    port;
    int         domain;       // PF_INET | PF_INET6 | PF_LOCAL  see man 3 socket
    int         type;         // SOCK_STREAM (TCP) | SOCK_DGRAM (UDP)
    int         protocol;     // IPPROTO_TCP | IPPROTO_UDP ...
    uint16_t    queue_len;    // len for listen()
    int         non_block;    // != 0 - set non_block mode
};





int host_or_ip_to_addr(const char *host_or_IP, int af, void *addr);
int get_ip_of_host(const char *host_name, int af, char *IP);

int get_addr_of_if(const char *if_name, int af, void *addr);
int get_ip_of_if(const char *if_name, int af, char *IP);


int wait_connect(int sd, unsigned int timeout_in_ms);


int connect_to_ipv4_socket(struct socket_param_t *socket_param);
int connect_to_ipv6_socket(struct socket_param_t *socket_param);
int connect_to_socket(struct socket_param_t *socket_param);


int create_passive_ipv4_socket(struct socket_param_t *socket_param);
int create_passive_ipv6_socket(struct socket_param_t *socket_param);
int create_passive_socket(struct socket_param_t *socket_param);


int send_loop(int sd, char *buf, size_t buf_len, int timeout_in_usec, int count_loop);
int recv_loop(int sd, char *buf, size_t buf_len, int timeout_in_usec, int count_loop);





#endif //NET_UTILS_HEADER
