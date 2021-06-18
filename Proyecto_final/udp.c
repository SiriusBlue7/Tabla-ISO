#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <timerms.h>

#include "udp.h"
#include "ipv4.h"
#include "ipv4_route_table.h"
#include "ipv4_config.h"


typedef struct udp_layer{
 ipv4_layer_t *ipv4_layer;
 uint16_t port;
}udp_layer_t;

/*
* Funcion que abre la interfaz del nivel de transporte
*/
udp_layer_t* udp_open(char* file_conf, char* file_conf_route, uint16_t port){
  udp_layer_t *layer = malloc(sizeof(struct udp_layer));//reserva el espacio en la memoria que ocupa la estructura de udp_layer

  if(port != 525){//en el caso de que sea un servidor, elpuerto en el que escucha siempre es el mismo, el 525
    printf("El numero de puerto origen del envio es: %i\n", port);
    layer->port = port;
  }else{//en el caso de que sea un cliente, el puerto por el que envia los mensajes es cada vez diferente, con lo cual riene que ser un puerto random >1024
		/* Generar número aleatorio entre 0 y RAND_MAX */
		int dice = rand();
		/* Número entero aleatorio entre 1024 y aleatorio */
		dice = 1024 + (int) (100.0 * dice / (RAND_MAX));
		printf("El numero de puerto origen del envio es: %i\n", dice);
    layer->port = dice;//Número random mayor que 1024
  }
  layer->ipv4_layer = ipv4_open(file_conf, file_conf_route);//en el campo de ipv4_layer se rrellena llamando a su vez aipv4_open
  return layer;
/*
* Funcion que cierra la interfaz del nivel de transporte
*/
}


int udp_close(udp_layer_t* layer){

  int err = -1;
  if(layer->ipv4_layer != NULL){
    /*1. Cerrar la IPv4*/
    printf("Cerrando IPv4.\n");
    ipv4_close(layer->ipv4_layer);
    err = 0;//no hay ningun error
  }
  return err;
}

/*
* Funcion que envia el datagrama UDP
*/
int udp_send(udp_layer_t *layer, ipv4_addr_t dst,uint16_t port_dst, unsigned char *payload, int payload_length ){

    struct udp_frame udp_message;//Se crea la estructura del mensaje y se rellenan los campos

    udp_message.src_port = htons(layer->port);
  	udp_message.dst_port = htons(port_dst);
  	udp_message.length = htons(payload_length+UDP_HEADER_LENGTH) ;
  	udp_message.checksum = 0;
  	memcpy(udp_message.data,payload, payload_length );
  	int udp_msg_length = UDP_HEADER_LENGTH + payload_length;// se define el tamaño que va a tener para asi poder pasarselo al ip_send()

    uint8_t protocol = 0x11;

  	int r = ipv4_send(layer->ipv4_layer, dst, protocol, (unsigned char*) &udp_message, udp_msg_length );
    if (r == -1 || r ==0) {
      	return r;
    }
    return r - UDP_HEADER_LENGTH;//si el mensaje se envia bien , se devuelve el tamaño de toda la carga que se envia e ip - el tamaño de la cabecera udp
}


/*
* Funcion que recibe el datagrama UDP
*/
int udp_recv(udp_layer_t *layer, uint16_t port_dst, unsigned char buffer[], int buf_len, long int timeout){
	struct udp_frame udp_recibido;// se crea la estructura en la que se va a almacenar el mensaje recibido
	bool is_my_response;
	uint8_t protocol = 0x11;

	timerms_t timer;
	long int time_left = timerms_reset(&timer, timeout);
	int r;
  ipv4_addr_t dst;
	do{
		time_left = timerms_left(&timer);
		r = ipv4_recv(layer->ipv4_layer, protocol, (unsigned char *) &udp_recibido, dst, buf_len, time_left);
		if (r == -1) {
			fprintf(stderr, "ERROR en ipv4_recv()\n");
		}else if(r == 0 ){
			fprintf(stderr, "ERROR: No hay respuesta del Servidor IPv4\n");
		}
		is_my_response  = (port_dst == udp_recibido.dst_port);
		if(is_my_response){
			memcpy(buffer, udp_recibido.data, buf_len);
			printf("Es la respuesta que espero:\n");
		}
	}while(!is_my_response && !(r>0));

	if (r == -1 || r == 0){
		return r;
	}
	return (r - UDP_HEADER_LENGTH );
}
