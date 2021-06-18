#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <netinet/in.h>

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
    printf("       <file_conf>: Archivo config del cliente\n");
    printf("       <file_conf_route>: Archivo tablas de ruta cliente\n");
    printf("        <ip>: Direccion IP destino\n");
    exit(-1);
  }

  /* 1. Procesar los argumentos de la línea de comandos */
  char* file_conf = argv[1];
  char* file_conf_route  = argv[2];
  printf("\n\nIP dest :%s\n",argv[3] );
  ipv4_addr_t dst;
  ipv4_str_addr (argv[3], dst);
  uint8_t protocol =0x11;
  long int timeout = -1;
  ipv4_addr_t sender;
  char str[IPv4_STR_MAX_LENGTH];
  //char str_route[IPv4_STR_MAX_LENGTH];

  /* 2. Abrir la interfaz Ethernet */
  ipv4_layer_t * ipv4_layer = ipv4_open(file_conf, file_conf_route);

/*Tenemos en ipv4_layer:
  eth_iface_t *iface; Interface eth1
  ipv4_addr_t addr; IPv4Address 192.100.100.101
  ipv4_addr_t netmask; SubnetMask 255.255.255.0
  ipv4_route_table_t *routing_table; SubnetAddr        SubnetMask      Iface    Gateway
                                     0.0.0.0           0.0.0.0         eth1     192.100.100.102
                                     192.100.100.0     255.255.255.0   eth1     0.0.0.0
*/
ipv4_route_table_print ( ipv4_layer->routing_table );

  /* 3. Crear frame IP */
  struct ipv4_frame ipv4_message;

  int payload_len = DEFAULT_PAYLOAD_LENGTH - IPv4_HEADER_LENGTH;//1480
  if (argc == 5) {
    char * payload_len_str = argv[4];
    payload_len = atoi(payload_len_str);
    if ((payload_len < 0) || (payload_len > 1500)) {
      fprintf(stderr, "%s: Longitud de payload incorrecta: '%s'\n",
              myself, payload_len_str);
      exit(-1);
    }
  }
  /* Generar payload */
 unsigned char payload[payload_len];
 int i;
 for (i=0; i<payload_len; i++) {
   payload[i] = (unsigned char) i;
 }

  /*4. Enviar frame ip */
  int num_bytes_sent = ipv4_send(ipv4_layer, dst, protocol, payload, payload_len);
  printf("Bytes enviados: %d\n", num_bytes_sent );
  /* 5. Recibir frame ip */
  int num_bytes_recv = ipv4_recv(ipv4_layer, protocol,(unsigned char *) &ipv4_message, sender, sizeof(struct ipv4_frame), timeout);
  printf("Bytes recibidos: %d\n", num_bytes_recv );

  ipv4_addr_str ( sender, str );
  printf("IP origen del mensaje recibido: %s\n", str );



  /* 6. Cerrar la interfaz Ethernet */
  printf("Cerrando interfaz IP.\n");
  ipv4_close(ipv4_layer);

  /* Cerrar la aplicacion */
  return 0;
}
