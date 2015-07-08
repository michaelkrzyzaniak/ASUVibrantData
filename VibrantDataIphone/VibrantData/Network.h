//
//  TCP.h
//  
//
//  Created by Michael Krzyzaniak on 6/17/15.
//
//

#ifndef ____NETWORK__
#define ____NETWORK__

#ifdef cplusplus
extern "C"{
#endif /*cplusplus*/

#ifndef      BOOL
#define BOOL int
#define NO   0
#define YES  (!NO)
#endif       //BOOL

typedef struct opaque_network_struct Network;

Network*  net_new                ();
Network*  net_destroy            (Network* self);

BOOL  net_tcp_connect_to_host    (Network* self, char* ip_address, int port);
BOOL  net_tcp_receive_connections(Network* self, int port                  );
BOOL  net_udp_connect            (Network* self, int receive_port          );
void  net_disconnect             (Network* self);

int   net_tcp_send               (Network* self, void* data, int num_bytes );
int   net_tcp_receive            (Network* self, void* data, int num_bytes );
int   net_udp_send               (Network* self, char* data, int num_bytes, char* ip_address, int port);
int   net_udp_receive            (Network* self, void* data, int num_bytes );

char* net_get_remote_address     (Network* self);
int   net_get_remote_port        (Network* self);



#ifdef cplusplus
}
#endif /*cplusplus*/

#endif /*____NETWORK__*/
