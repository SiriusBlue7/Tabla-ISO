#include "eth.h"
#include <rawnet.h>
#include <timerms.h>

#include "ipv4.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdbool.h>

#define ARP_REQ 0x01
#define ARP_REP 0x02

/* Cabecera de una peticion ARP */
struct arp_frame{
  uint16_t hard_addr;
  uint16_t prot_addr;
  uint8_t hard_length;
  uint8_t prot_length;

  uint16_t opcode;

  mac_addr_t src_mac_addr;  /* Dirección MAC origen */
  ipv4_addr_t src_ipv4_addr;  /* Dirección IPv4 origen */
  mac_addr_t dest_mac_addr; /* Dirección MAC destino*/
  ipv4_addr_t dest_ipv4_addr;  /* Dirección IPv4 destino */
};

/*
*  Dada la dirección IPv4, envie una peticion ARP por la interfaz Ethernet
*  especificado y rellene la dirección MAC con la respuesta obtenida, o
*  devuelva 0 si la respuesta no ha llegado despues de 2 segundos.
*/

int arp_resolve(eth_iface_t* iface, ipv4_addr_t dest, mac_addr_t mac,   ipv4_addr_t src_ipv4_addr){
  uint16_t type = 0x0806;
  struct arp_frame arp_message;
  arp_message.hard_addr = htons(0x0001);
  arp_message.prot_addr = htons(0x0800);
  arp_message.hard_length = 0x06;
  arp_message.prot_length = 0x04;
  mac_addr_t mac_src;
  eth_getaddr(iface, mac_src);//obtenemos mi direccion MAC
  arp_message.opcode = htons(ARP_REQ);
  memset(arp_message.dest_mac_addr, 0, MAC_ADDR_SIZE);//esto tiene que ser la direccion broadcast, porque es la direccion que hay que encotrar
  memcpy(arp_message.src_mac_addr , mac_src, MAC_ADDR_SIZE);
  memcpy(arp_message.dest_ipv4_addr, dest, IPv4_ADDR_SIZE);
  memcpy(arp_message.src_ipv4_addr, src_ipv4_addr, IPv4_ADDR_SIZE);

  char mac_origen[MAC_STR_LENGTH];
  mac_addr_str(mac_src, mac_origen);
  printf("\nDirección MAC origen: %s\n\n", mac_origen);

  int a = eth_send(iface, MAC_BCAST_ADDR, type, (unsigned char *) &arp_message, sizeof(struct arp_frame));
  if(a < 0){
    fprintf(stderr, "ERROR");
    return -1;
  }

  struct arp_frame arp_recibido;
  bool is_response;
  bool is_my_response;

  timerms_t timer;
  long int timeout = 2000;
  long long int time_left= timerms_reset(&timer, timeout);
  mac_addr_t assoc_mac;

  int r;
  do {
    time_left=timerms_left(&timer);
    r = eth_recv(iface, assoc_mac, type, (unsigned char *)&arp_recibido,
          sizeof(struct arp_frame), time_left);
//    printf("\nvalor de r=%d\n", r);//longitud en bytes de los datos de la trama recibida

    //Comprobar si la respuesta es ARP
    is_response =  arp_recibido.opcode == ntohs(ARP_REP);
//    printf("\nis_response=%d\n", is_response);//Miro si hay respuesta
    if(is_response){
      is_my_response = (memcmp(dest, arp_recibido.src_ipv4_addr, IPv4_ADDR_SIZE) == 0);//comparamos si son iguales
//      printf("\nis_my_response=%d\n", is_my_response);//Miro si es mi respuesta
      if(is_my_response){
          memcpy(mac,assoc_mac, MAC_ADDR_SIZE); //Guardo el valor de la MAC que he obtenido
      }
    }


  }while(!is_my_response && !(r>0));


  /* Declaracion de variables a usar */
  char str[MAC_STR_LENGTH];
  /* Imprimir la direccion MAC del servidor */
  if(r == -1){
    fprintf(stderr, "\nError en el mensaje ARP\n");
  }else if (r == 0) {
    printf("\nNo ha habido respuesta en dos segundos\n");
  } else {
    mac_addr_str(mac,str);
    printf("\nDirección MAC servidor: %s\n", str);
 }


  return r;
}
