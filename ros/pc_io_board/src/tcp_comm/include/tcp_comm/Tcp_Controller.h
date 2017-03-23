#ifndef TCP_COMM_TCP_CONTROLLER
#define TCP_COMM_TCP_CONTROLLER

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define ETH_Tcp_Controller_IP       		"192.168.1.52"
#define ETH_Tcp_Controller_SERVER_PORT   5001

class Tcp_Controller {
public:
    static Tcp_Controller* get_instance();

    int send_all(char* data, int length);
    int receive(char *data, int max_length);
    virtual ~Tcp_Controller();


protected:
    static Tcp_Controller * _Tcp_Controller_instance;
    Tcp_Controller();
    void init_eth_interface();

};

#endif //TCP_COMM_TCP_CONTROLLER