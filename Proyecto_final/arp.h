#include "eth.h"
#include "ipv4.h"

/*
*   Funciones que estan definidas en "ipv4.c"
*    - void ipv4_addr_str(ipv4_addr_t addr, char str[]);
*    - int ipv4_str_addr(char* str, ipv4_addr_t addr);
*/

/*
  Dada la dirección IPv4, envie una peticion ARP por la interfaz Ethernet
  especificado y rellene la dirección MAC con la respuesta obtenida, o
  devuelva 0 si la respuesta no ha llegado despues de 2 segundos.
*/
int arp_resolve(eth_iface_t* iface, ipv4_addr_t dest, mac_addr_t mac,  ipv4_addr_t src_ipv4_addr);
