#ifndef DM3_CORE_TCP_CONTROLLER
#define DM3_CORE_TCP_CONTROLLER

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define ETH_Tcp_Controller_IP       	"192.168.2.52"
#define ETH_Tcp_Controller_SERVER_PORT  5001

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
    int recv_to(int fd, char *buffer, int len, int flags, int to);
};

#endif //DM3_CORE_TCP_CONTROLLER