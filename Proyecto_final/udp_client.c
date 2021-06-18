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
  if (argc != 5) {
    printf("Uso: %s <file_conf> <file_conf_route>\n", myself);
    printf("       <file_conf>: Archivo config del cliente\n");
    printf("       <file_conf_route>: Archivo tablas de ruta cliente\n");
    printf("        <ip>: Direccion IP destino\n");
    printf("       <port>: Numero puerto del Servidor\n");
    exit(-1);
  }

  /* 1. Procesar los argumentos de la línea de comandos */
  char* file_conf = argv[1];
  char* file_conf_route  = argv[2];
  uint16_t port = atoi(argv[4]);
  printf("\n\nPort dest :%d\n",port);
  printf("\n\nIP dest :%s\n",argv[3] );
  ipv4_addr_t dst;
  ipv4_str_addr (argv[3], dst);

  long int timeout = -1;

  /* 2. Abrir la interfaz Ethernet */
  udp_layer_t * udp_layer = udp_open(file_conf, file_conf_route, port);

  /* 3. Crear frame UDP */
  struct udp_frame udp_message;

  int payload_len = DEFAULT_PAYLOAD_LENGTH - IPv4_HEADER_LENGTH - UDP_HEADER_LENGTH;//1472
  //En caso de que pasemos por parametros la longitud de los datos a enviar
  if (argc == 6) {
    char * payload_len_str = argv[5];
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

  /*4. Enviar frame UDP */
  int num_bytes_sent = udp_send(udp_layer, dst, port, payload, payload_len);
  printf("Bytes enviados: %d\n", num_bytes_sent);
  /* 5. Recibir frame UDP */
  int num_bytes_recv = udp_recv(udp_layer, udp_message.src_port, (unsigned char *) &udp_message, sizeof(struct udp_frame), timeout);
  printf("Bytes recibidos: %d\n", num_bytes_recv);

  /* 6. Cerrar la interfaz UDP */
  printf("Cerrando UDP.\n");
  udp_close(udp_layer);

  /* Cerrar la aplicacion */
  return 0;
}
