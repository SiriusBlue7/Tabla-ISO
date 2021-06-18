#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>
#include <arpa/inet.h>

#include "ipv4.h"
#include "ipv4_route_table.h"
#include "ipv4_config.h"
#define DEFAULT_PAYLOAD_LENGTH 1500

int main ( int argc, char * argv[] )
{
  /* Mostrar mensaje de ayuda si el número de argumentos es incorrecto */
  char * myself = basename(argv[0]);
  if (argc != 4) {
    printf("Uso: %s <file_conf> <file_conf_route>\n", myself);
    printf("       <file_conf>: Archivo config del servidor\n");
    printf("       <file_conf_route>: Archivo tablas de ruta servidor\n");
    printf("        <protocol>: Protocolo de envio\n");
    exit(-1);
  }

  /* 1. Procesar los argumentos de la línea de comandos */
  char* file_conf = argv[1];
  char* file_conf_route  = argv[2];
  uint8_t protocol = atoi(argv[3]);
  long int timeout = -1;
  ipv4_addr_t sender;
  char str[IPv4_STR_MAX_LENGTH];
  int num_bytes_recv;
  int num_bytes_sent;
  int payload_len = DEFAULT_PAYLOAD_LENGTH - IPv4_HEADER_LENGTH;//1480
  //bool my_response;
  //char str_route[IPv4_STR_MAX_LENGTH];

  /* 2. Abrir la interfaz Ethernet */
  ipv4_layer_t * ipv4_layer = ipv4_open(file_conf, file_conf_route);
/*Tenemos en ipv4_layer:
  eth_iface_t *iface; Interface eth1
  ipv4_addr_t addr; IPv4Address 192.100.100.102
  ipv4_addr_t netmask; SubnetMask 255.255.255.0
  ipv4_route_table_t *routing_table; SubnetAddr        SubnetMask      Iface    Gateway
                                     0.0.0.0           0.0.0.0         eth1     192.100.100.101
                                     192.100.100.0     255.255.255.0   eth1     0.0.0.0
*/
  ipv4_route_table_print ( ipv4_layer->routing_table );

  /* 3. Crear frame IP */
  struct ipv4_frame ipv4_message;

  /* 4. Recibir frame ip */

  num_bytes_recv = ipv4_recv(ipv4_layer, protocol,(unsigned char *) &ipv4_message, sender, sizeof(struct ipv4_frame), timeout);
  printf("Bytes recibidos: %d\n", num_bytes_recv );


  //Si he recivido algo
  ipv4_addr_str ( sender, str );
  printf("IP origen del mensaje recibido: %s\n", str );

  /* 5. Enviar frame ip */
  num_bytes_sent = ipv4_send(ipv4_layer, sender, protocol, ipv4_message.payload, payload_len);
  printf("Bytes enviados: %d\n", num_bytes_sent );




  /* 6. Cerrar la interfaz Ethernet */
  printf("Cerrando interfaz IP.\n");
  ipv4_close(ipv4_layer);

  /* Cerrar la aplicacion */
  return 0;
}
