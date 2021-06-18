#include "arp.h"
#include "eth.h"
#include "ipv4.h"
#include "ipv4_route_table.h"
#include "ipv4_config.h"

#define UDP_HEADER_LENGTH 8


struct udp_frame{
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t length;
	uint16_t checksum;
	unsigned char data[ETH_MTU - IPv4_HEADER_LENGTH - UDP_HEADER_LENGTH];
};


typedef struct udp_layer udp_layer_t;

/*
* Funcion que abre la interfaz del nivel de transporte
*/
udp_layer_t* udp_open(char* file_conf, char* file_conf_route, uint16_t port);

/*
* Funcion que cierra la interfaz del nivel de transporte
*/
int udp_close(udp_layer_t* layer);

/*
* Funcion que envia el datagrama UDP
*/
int udp_send(udp_layer_t *layer, ipv4_addr_t dst,uint16_t port_dst, unsigned char *payload, int payload_length );
/*
* Funcion que recibe el datagrama UDP
*/
int udp_recv(udp_layer_t *layer,uint16_t port_dst, unsigned char buffer[], int buf_len, long int timeout );
