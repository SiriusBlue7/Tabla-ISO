#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <netinet/in.h>

#include "udp.h"
#include "arp.h"
#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#define DEFAULT_PAYLOAD_LENGTH 1500

int main ( int argc, char * argv[] )
{
  /* Mostrar mensaje de ayuda si el número de argumentos es incorrecto */
  char * myself = basename(argv[0]);
  if (argc != 3) {
    printf("Uso: %s <file_conf> <file_conf_route>\n", myself);
    printf("       <file_conf>: Archivo config del servidor\n");
    printf("       <file_conf_route>: Archivo tablas de ruta servidor\n");
    exit(-1);
  }

  /* 1. Procesar los argumentos de la línea de comandos */
  char* file_conf = argv[1];
  char* file_conf_route  = argv[2];
  uint16_t port = 525; // puerto distinto de 0 para que en el udp_open escuche por un puerto aleatorio

  long int timeout = -1;

  int num_bytes_recv;
  int num_bytes_sent;
  int payload_len = DEFAULT_PAYLOAD_LENGTH - IPv4_HEADER_LENGTH - UDP_HEADER_LENGTH;//1472

  /* 2. Abrir la interfaz Ethernet */
  udp_layer_t * udp_layer = udp_open(file_conf, file_conf_route, port);

  /* 3. Crear frame UDP */
  struct udp_frame udp_message;

  /* 4. Recibir frame UDP */
  num_bytes_recv = udp_recv(udp_layer, udp_message.dst_port, (unsigned char *) &udp_message, sizeof(struct udp_frame), timeout);
  printf("Bytes recibidos: %d\n", num_bytes_recv );



  /* 5. Enviar frame UDP */
  char str[IPv4_STR_MAX_LENGTH]="163.117.114.107";
  printf("\n\nIP origen que nos trajo el mensaje :%s\n",str);
  ipv4_addr_t dst;
  ipv4_str_addr (str, dst);
  num_bytes_sent = udp_send(udp_layer, dst, udp_message.src_port, udp_message.data, payload_len);
  //No se de donde sacar la ip dst; es decir la ip origen que mando el paquete recibido
  //num_bytes_sent = udp_send(udp_layer,                    , udp_message.src_port, (unsigned char *)udp_message.data, payload_len);
  printf("Bytes enviados: %d\n", num_bytes_sent );

  /* 6. Cerrar la interfaz UDP */
  printf("Cerrando UDP.\n");
  udp_close(udp_layer);


  /* Cerrar la aplicacion */
  return 0;
}
