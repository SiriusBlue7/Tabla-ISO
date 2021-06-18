#include "arp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>


int main ( int argc, char * argv[] )
{
  /* Mostrar mensaje de ayuda si el número de argumentos es incorrecto */
  char * myself = basename(argv[0]);
  if (argc != 4) { //cambiar 4 por 3
    printf("\nUso: %s <iface> <IP>\n", myself);
    printf("\n       <iface>: Nombre de la interfaz Ethernet\n");
    printf("\n         <IP>: Dirección IP del servidor Ethernet\n");
    printf("\n         <IP>: Dirección IP del cliente Ethernet\n");
    exit(-1);
  }

  /* Procesar los argumentos de la línea de comandos */
  char * iface_name = argv[1];

  /* Procesar la entrada de la direccion IP */
  char* server_addr_str = argv[2];
  printf("\nDirección IP del servidor: %s\n", server_addr_str);
  ipv4_addr_t server_ip;
  int err = ipv4_str_addr(server_addr_str, server_ip);
  if (err != 0) {
    fprintf(stderr, "\n%s: Dirección IP del servidor incorrecta: '%s'\n",
            myself, argv[2]);
    exit(-1);
  }




  char* client_addr_str = argv[3];
  printf("\nDirección IP del cliente: %s\n", client_addr_str);
  ipv4_addr_t client_ip;
  int error = ipv4_str_addr(client_addr_str, client_ip);
  if (error != 0) {
    fprintf(stderr, "\n%s: Dirección IP del servidor incorrecta: '%s'\n",
            myself, argv[2]);
    exit(-1);
  }




  /* Declaracion de variables a usar */

  mac_addr_t server_mac;

  /* Abrir la interfaz Ethernet */
  eth_iface_t* eth_iface = eth_open(iface_name);

  /* Enviar la peticion ARP */
  arp_resolve(eth_iface, server_ip, server_mac, client_ip);




  /* Cerrar la interfaz Ethernet */
  printf("\nCerrando interfaz Ethernet %s.\n", iface_name);
  eth_close(eth_iface);

  /* Cerrar la aplicacion */
  return 0;
}
