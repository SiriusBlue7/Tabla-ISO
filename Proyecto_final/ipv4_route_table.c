#include "ipv4.h"
#include "ipv4_route_table.h"
#include "ipv4_config.h"
#include "arp.h"

#include <timerms.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>


/* ipv4_route_t * ipv4_route_create
 * ( ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw );
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta IPv4 con los parámetros especificados:
 *   dirección de subred, máscara, nombre de interfaz y dirección de siguiente
 *   salto.
 *
 *   Esta función reserva memoria para la estructura creada. Debe utilizar la
 *   función 'ipv4_route_free()' para liberar dicha memoria.
 *
 * PARÁMETROS:
 *   'subnet': Dirección IPv4 de la subred destino de la nueva ruta.
 *     'mask': Máscara de la subred destino de la nueva ruta.
 *    'iface': Nombre del interfaz empleado para llegar a la subred destino de
 *             la nueva  ruta.
 *             Debe tener una longitud máxima de 'IFACE_NAME_LENGTH' caracteres.
 *       'gw': Dirección IPv4 del encaminador empleado para llegar a la subred
 *             destino de la nueva ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la ruta creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la ruta.
 */
ipv4_route_t * ipv4_route_create
( ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw )
{
  ipv4_route_t * route = (ipv4_route_t *) malloc(sizeof(struct ipv4_route));

  if ((route != NULL) &&
      (subnet != NULL) && (mask != NULL) && (iface != NULL) && (gw != NULL)) {
    memcpy(route->subnet_addr, subnet, IPv4_ADDR_SIZE);
    memcpy(route->subnet_mask, mask, IPv4_ADDR_SIZE);
    strncpy(route->iface, iface, IFACE_NAME_MAX_LENGTH);
    memcpy(route->gateway_addr, gw, IPv4_ADDR_SIZE);
  }

  return route;
}


/* int ipv4_route_lookup ( ipv4_route_t * route, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función indica si la dirección IPv4 especificada pertence a la
 *   subred indicada. En ese caso devuelve la longitud de la máscara de la
 *   subred.
 *
 * ************************************************************************
 * * Esta función NO está implementada, debe implementarla usted para que *
 * * funcione correctamente la función 'ipv4_route_table_lookup()'.       *
 * ************************************************************************
 *
 * PARÁMETROS:
 *   'route': Ruta a la subred que se quiere comprobar.
 *    'addr': Dirección IPv4 destino.
 *
 * VALOR DEVUELTO:
 *   Si la dirección IPv4 pertenece a la subred de la ruta especificada, debe
 *   devolver un número positivo que indica la longitud del prefijo de
 *   subred. Esto es, el número de bits a uno de la máscara de subred.
 *   La función devuelve '-1' si la dirección IPv4 no pertenece a la subred
 *   apuntada por la ruta especificada.
 */
int ipv4_route_lookup ( ipv4_route_t * route, ipv4_addr_t addr )
{
  int prefix_length = -1;
  ipv4_addr_t aux;
  for (int i = 0; i < IPv4_ADDR_SIZE; i++) {//IPv4_ADDR_SIZE=4
    aux[i]= addr[i]&route->subnet_mask[i];
  }

  bool pertenece;
  for (int i = 0; i < IPv4_ADDR_SIZE; i++) {//IPv4_ADDR_SIZE=4
    pertenece = (aux[i] == route->subnet_addr[i]);
    if (!pertenece){
      return prefix_length;//no pertenece, devolvemos -1
    }
  }

  int bit_cnt = 0;
  for (int i = 0; i < IPv4_ADDR_SIZE; i++) {//IPv4_ADDR_SIZE=4

    switch (route->subnet_mask[i]) {

      case 255:
        bit_cnt+=8;
        break;

      case 254:
        bit_cnt+=7;
        break;

      case 252:
        bit_cnt+=6;
        break;

      case 248:
        bit_cnt+=5;
        break;

      case 240:
        bit_cnt+=4;
        break;

      case 224:
        bit_cnt+=3;
        break;

      case 192:
        bit_cnt+=2;
        break;

      case 128:
        bit_cnt+=1;
        break;

      case 0:
        bit_cnt+=0;
        break;
    }//switch

  }//for


  prefix_length = bit_cnt;

  if(prefix_length == -1){
    printf("No son iguales.\n");
  }else{
    printf("El prefijo de red es: %d\n", prefix_length);
  }


  return prefix_length;
}

/* void ipv4_route_print ( ipv4_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime la ruta especificada por la salida estándar.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea imprimir.
 */
void ipv4_route_print ( ipv4_route_t * route )
{
  if (route != NULL) {
    char subnet_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->subnet_addr, subnet_str);
    char mask_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->subnet_mask, mask_str);
    char* iface_str = route->iface;
    char gw_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->gateway_addr, gw_str);

    printf("%s/%s via %s dev %s", subnet_str, mask_str, gw_str, iface_str);
  }
}


/* void ipv4_route_free ( ipv4_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la ruta especificada, que
 *   ha sido creada con 'ipv4_route_create()'.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea liberar.
 */
void ipv4_route_free ( ipv4_route_t * route )
{
  if (route != NULL) {
    free(route);
  }
}

/* ipv4_route_t* ipv4_route_read ( char* filename, int linenum, char * line )
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta IPv4 a partir de la línea del fichero
 *   de la tabla de rutas especificada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero de la tabla de rutas
 *    'linenum': Número de línea del fichero de la tabal de rutas.
 *       'line': Línea del fichero de la tabla de rutas a procesar.
 *
 * VALOR DEVUELTO:
 *   La ruta leída, o NULL si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función imprime un mensaje de error y devuelve NULL si se ha
 *   producido algún error al leer la ruta.
 */
ipv4_route_t* ipv4_route_read ( char* filename, int linenum, char * line )
{
  ipv4_route_t* route = NULL;

  char subnet_str[256];
  char mask_str[256];
  char iface_name[256];
  char gw_str[256];

  /* Parse line: Format "<subnet> <mask> <iface> <gw>\n" */
  int params = sscanf(line, "%s %s %s %s\n",
	       subnet_str, mask_str, iface_name, gw_str);
  if (params != 4) {
    fprintf(stderr, "%s:%d: Invalid IPv4 Route format: '%s' (%d params)\n",
	    filename, linenum, line, params);
    fprintf(stderr,
	    "%s:%d: Format must be: <subnet> <mask> <iface> <gw>\n",
	    filename, linenum);
    return NULL;
  }

  /* Parse IPv4 route subnet address */
  ipv4_addr_t subnet;
  int err = ipv4_str_addr(subnet_str, subnet);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <subnet> value: '%s'\n",
	    filename, linenum, subnet_str);
    return NULL;
  }

  /* Parse IPv4 route subnet mask */
  ipv4_addr_t mask;
  err = ipv4_str_addr(mask_str, mask);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <mask> value: '%s'\n",
	    filename, linenum, mask_str);
    return NULL;
  }

  /* Parse IPv4 route gateway */
  ipv4_addr_t gateway;
  err = ipv4_str_addr(gw_str, gateway);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <gw> value: '%s'\n",
	    filename, linenum, gw_str);
    return NULL;
  }

  /* Create new route with parsed parameters */
  route = ipv4_route_create(subnet, mask, iface_name, gateway);
  if (route == NULL) {
    fprintf(stderr, "%s:%d: Error creating the new route\n",
	    filename, linenum);
  }

  return route;
}


/* void ipv4_route_output ( ipv4_route_t * route, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la ruta IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *      'route': Ruta a imprimir.
 *     'header': '0' para imprimir una línea con la cabecera de la ruta.
 *        'out': Salida por la que imprimir la ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si la ruta se ha impreso correctamente.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 */
int ipv4_route_output ( ipv4_route_t * route, int header, FILE * out )
{
  int err;

  if (header == 0) {
    err = fprintf(out, "# SubnetAddr  \tSubnetMask    \tIface  \tGateway\n");
    if (err < 0) {
      return -1;
    }
  }

  char subnet_str[IPv4_STR_MAX_LENGTH];
  char mask_str[IPv4_STR_MAX_LENGTH];
  char* ifname = NULL;
  char gw_str[IPv4_STR_MAX_LENGTH];

  if (route != NULL) {
      ipv4_addr_str(route->subnet_addr, subnet_str);
      ipv4_addr_str(route->subnet_mask, mask_str);
      ifname = route->iface;
      ipv4_addr_str(route->gateway_addr, gw_str);

      err = fprintf(out, "%-15s\t%-15s\t%s\t%-15s\n",
		    subnet_str, mask_str, ifname, gw_str);
      if (err < 0) {
        return -1;
      }
  }

  return 0;
}

/* ipv4_route_table_t * ipv4_route_table_create();
 *
 * DESCRIPCIÓN:
 *   Esta función crea una tabla de rutas IPv4 vacía.
 *
 *   Esta función reserva memoria para la tabla de rutas creada, para
 *   liberarla es necesario llamar a la función 'ipv4_route_table_free()'.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la tabla de rutas creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la tabla de rutas.
 */
ipv4_route_table_t * ipv4_route_table_create()
{
  ipv4_route_table_t * table;

  table = (ipv4_route_table_t *) malloc(sizeof(struct ipv4_route_table));
  if (table != NULL) {
    int i;
    for (i=0; i<IPv4_ROUTE_TABLE_SIZE; i++) {
      table->routes[i] = NULL;
    }
  }

  return table;
}


/* int ipv4_route_table_add ( ipv4_route_table_t * table,
 *                            ipv4_route_t * route );
 * DESCRIPCIÓN:
 *   Esta función añade la ruta especificada en la primera posición libre de
 *   la tabla de rutas.
 *
 * PARÁMETROS:
 *   'table': Tabla donde añadir la ruta especificada.
 *   'route': Ruta a añadir en la tabla de rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el indice de la posición [0,IPv4_ROUTE_TABLE_SIZE-1]
 *   donde se ha añadido la ruta especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no ha sido posible añadir la ruta
 *   especificada.
 */
int ipv4_route_table_add ( ipv4_route_table_t * table, ipv4_route_t * route )
{
  int route_index = -1;

  if (table != NULL) {
    /* Find an empty place in the route table */
    int i;
    for (i=0; i<IPv4_ROUTE_TABLE_SIZE; i++) {
      if (table->routes[i] == NULL) {
        table->routes[i] = route;
        route_index = i;
        break;
      }
    }
  }

  return route_index;
}


/* ipv4_route_t * ipv4_route_table_remove ( ipv4_route_table_t * table,
 *                                          int index );
 *
 * DESCRIPCIÓN:
 *   Esta función borra la ruta almacenada en la posición de la tabla de rutas
 *   especificada.
 *
 *   Esta función NO libera la memoria reservada para la ruta borrada. Para
 *   ello es necesario utilizar la función 'ipv4_route_free()' con la ruta
 *   devuelta.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea borrar una ruta.
 *   'index': Índice de la ruta a borrar. Debe tener un valor comprendido
 *            entre [0, IPv4_ROUTE_TABLE_SIZE-1].
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta que estaba almacenada en la posición
 *   indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si la ruta no ha podido ser borrada, o no
 *   existía ninguna ruta en dicha posición.
 */
ipv4_route_t * ipv4_route_table_remove ( ipv4_route_table_t * table, int index )
{
  ipv4_route_t * removed_route = NULL;

  if ((table != NULL) && (index >= 0) && (index < IPv4_ROUTE_TABLE_SIZE)) {
    removed_route = table->routes[index];
    table->routes[index] = NULL;
  }

  return removed_route;
}


/* ipv4_route_t * ipv4_route_table_lookup ( ipv4_route_table_t * table,
 *                                          ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve la mejor ruta almacenada en la tabla de rutas para
 *   alcanzar la dirección IPv4 destino especificada.
 *
 *   Esta función recorre toda la tabla de rutas buscando rutas que contengan
 *   a la dirección IPv4 indicada. Para ello emplea la función
 *   'ipv4_route_lookup()'. De todas las rutas posibles se devuelve aquella
 *   con el prefijo más específico, esto es, aquella con la máscara de subred
 *   mayor.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas en la que buscar la dirección IPv4 destino.
 *    'addr': Dirección IPv4 destino a buscar.
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta más específica para llegar a la dirección
 *   IPv4 indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no no existe ninguna ruta para alcanzar
 *   la dirección indicada, o si no ha sido posible realizar la búsqueda.
 */
ipv4_route_t * ipv4_route_table_lookup ( ipv4_route_table_t * table,
                                         ipv4_addr_t addr )
{
  ipv4_route_t * best_route = NULL;
  int best_route_prefix = -1;

  if (table != NULL) {
    int i;
    for (i=0; i<IPv4_ROUTE_TABLE_SIZE; i++) {
      ipv4_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        int route_i_lookup = ipv4_route_lookup(route_i, addr);
        if (route_i_lookup > best_route_prefix) {
          best_route = route_i;
          best_route_prefix = route_i_lookup;
        }
      }
    }
  }

  return best_route;
}


/* ipv4_route_t * ipv4_route_table_get ( ipv4_route_table_t * table, int index );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas especificada.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea obtener una ruta.
 *   'index': Índice de la ruta consultada. Debe tener un valor comprendido
 *            entre [0, IPv4_ROUTE_TABLE_SIZE-1].
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no ha sido posible consultar la tabla de
 *   rutas, o no existe ninguna ruta en dicha posición.
 */
ipv4_route_t * ipv4_route_table_get ( ipv4_route_table_t * table, int index )
{
  ipv4_route_t * route = NULL;

  if ((table != NULL) && (index >= 0) && (index < IPv4_ROUTE_TABLE_SIZE)) {
    route = table->routes[index];
  }

  return route;
}


/* int ipv4_route_table_find ( ipv4_route_table_t * table, ipv4_addr_t subnet,
 *                                                         ipv4_addr_t mask );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el índice de la ruta para llegar a la subred
 *   especificada.
 *
 * PARÁMETROS:
 *    'table': Tabla de rutas en la que buscar la subred.
 *   'subnet': Dirección de la subred a buscar.
 *     'mask': Máscara de la subred a buscar.
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la posición de la tabla de rutas donde se encuentra
 *   la ruta que apunta a la subred especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no se ha encontrado la ruta especificada o
 *   '-2' si no ha sido posible realizar la búsqueda.
 */
int ipv4_route_table_find
( ipv4_route_table_t * table, ipv4_addr_t subnet, ipv4_addr_t mask )
{
  int route_index = -2;

  if (table != NULL) {
    route_index = -1;
    int i;
    for (i=0; i<IPv4_ROUTE_TABLE_SIZE; i++) {
      ipv4_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        int same_subnet =
          (memcmp(route_i->subnet_addr, subnet, IPv4_ADDR_SIZE) == 0);
        int same_mask =
          (memcmp(route_i->subnet_mask, mask, IPv4_ADDR_SIZE) == 0);

        if (same_subnet && same_mask) {
          route_index = i;
          break;
        }
      }
    }
  }

  return route_index;
}


/* void ipv4_route_table_free ( ipv4_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la tabla de rutas
 *   especificada, incluyendo todas las rutas almacenadas en la misma,
 *   mediante la función 'ipv4_route_free()'.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas a borrar.
 */
void ipv4_route_table_free ( ipv4_route_table_t * table )
{
  if (table != NULL) {
    int i;
    for (i=0; i<IPv4_ROUTE_TABLE_SIZE; i++) {
      ipv4_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        table->routes[i] = NULL;
        ipv4_route_free(route_i);
      }
    }
    free(table);
  }
}


/* int ipv4_route_table_read ( char * filename, ipv4_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función lee el fichero especificado y añade las rutas IPv4
 *   estáticas leídas en la tabla de rutas indicada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero con rutas IPv4 que se desea leer.
 *      'table': Tabla de rutas donde añadir las rutas leidas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas leidas y añadidas en la tabla, o
 *   '0' si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al leer el
 *   fichero de rutas.
 */
int ipv4_route_table_read ( char * filename, ipv4_route_table_t * table )
{
  int read_routes = 0;

  FILE * routes_file = fopen(filename, "r");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening input IPv4 Routes file \"%s\": %s.\n",
            filename, strerror(errno));
    return -1;
  }

  int linenum = 0;
  char line_buf[1024];
  int err = 0;

  while ((! feof(routes_file)) && (err==0)) {

    linenum++;

    /* Read next line of file */
    char* line = fgets(line_buf, 1024, routes_file);
    if (line == NULL) {
      break;
    }

    /* If this line is empty or a comment, just ignore it */
    if ((line_buf[0] == '\n') || (line_buf[0] == '#')) {
      err = 0;
      continue;
    }

    /* Parse route from line */
    ipv4_route_t* new_route = ipv4_route_read(filename, linenum, line);
    if (new_route == NULL) {
      err = -1;
      break;
    }

    /* Add new route to Route Table */
    if (table != NULL) {
      err = ipv4_route_table_add(table, new_route);
      if (err >= 0) {
	err = 0;
	read_routes++;
      }
    }
  } /* while() */

  if (err == -1) {
    read_routes = -1;
  }

  /* Close IP Route Table file */
  fclose(routes_file);

  return read_routes;
}


/* void ipv4_route_table_output ( ipv4_route_table_t * table, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la tabla de rutas IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 *        'out': Salida por la que imprimir la tabla de rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas impresas por la salida indicada, o
 *   '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 */
int ipv4_route_table_output ( ipv4_route_table_t * table, FILE * out )
{
  int err;

  int i;
  for (i=0; i<IPv4_ROUTE_TABLE_SIZE; i++) {
    ipv4_route_t * route_i = ipv4_route_table_get(table, i);
    if (route_i != NULL) {
      err = ipv4_route_output(route_i, i, out);
      if (err == -1) {
	return -1;
      }
    }
  }

  return 0;
}


/* int ipv4_route_table_write ( ipv4_route_table_t * table, char * filename );
 *
 * DESCRIPCIÓN:
 *   Esta función almacena en el fichero especificado la tabla de rutas IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a almacenar.
 *   'filename': Nombre del fichero donde se desea almacenar la tabla de
 *               rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas almacenadas en el fichero de
 *   rutas, o '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir el
 *   fichero de rutas.
 */
int ipv4_route_table_write ( ipv4_route_table_t * table, char * filename )
{
  int num_routes = 0;

  FILE * routes_file = fopen(filename, "w");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening output IPv4 Routes file \"%s\": %s.\n",
            filename, strerror(errno));
    return -1;
  }

  fprintf(routes_file, "# %s\n", filename);
  fprintf(routes_file, "#\n");

  if (table != NULL) {
    num_routes = ipv4_route_table_output (table, routes_file);
    if (num_routes == -1) {
      fprintf(stderr, "Error writing IPv4 Routes file \"%s\": %s.\n",
              filename, strerror(errno));
      return -1;
    }
  }

  fclose(routes_file);

  return num_routes;
}


/* void ipv4_route_table_print ( ipv4_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida estándar la tabla de rutas IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 */
void ipv4_route_table_print ( ipv4_route_table_t * table )
{
  if (table != NULL) {
    ipv4_route_table_output (table, stdout);
  }
}








/* Funciones open(), close(), send(), recive() */

ipv4_layer_t *ipv4_open(char* file_conf, char* file_conf_route){
  /*1. Crear layer->routing_table*/
  ipv4_layer_t *layer = malloc(sizeof(ipv4_layer_t));
  layer->routing_table = malloc(sizeof(ipv4_route_table_t));

  /*2. Leer direcciones y subred de file_conf*/
  //ipv4_config_read(nom del archivo, var donde guardar iface, var donde guardar la addr, var donde guardar la netmask)
  char nom_iface[IFACE_NAME_MAX_LENGTH];

  int read_config = ipv4_config_read(file_conf, nom_iface, layer->addr, layer->netmask );
  if (read_config != 0) {//en caso de que nose lea bien el archivo devolvera 0 y si hay fallo devolvera -1
      ipv4_route_table_free (layer->routing_table);
      free(layer);
    return NULL;
  }

  /*3. Leer tabla de rutas de file_conf_route*/
  //ipv4_route_table_read(nom de archivo a leer, la tabla de rutas que rellenar)
  int read_route_config =  ipv4_route_table_read(file_conf_route, layer->routing_table);
  if (read_route_config == -1){//en el que caso de que haya un error devolvera -1
    ipv4_route_table_free (layer->routing_table);//si hay fallo , se liberaran todas las memorias dinamicas y despues se devolvera un Null
    free(layer);
    return NULL;
  }

  /*4. Abrir interfaz eth*/
  printf("Abriendo interfaz Ethernet %s\n", nom_iface);
  eth_iface_t *new_eth =  eth_open(nom_iface);//lo que se relena es la interfaz por la que abrir el ethernet
  layer->iface = new_eth;
  if(new_eth==NULL){ //si hay algun fallo abriendo el eth , este devolvera null, y se activara el if
    ipv4_route_table_free (layer->routing_table);//en caso de que haya algun fallo iniciando se liberara la memoria dinámica
    free(layer);
    return NULL;
  }

  return layer;
}


int ipv4_close(ipv4_layer_t* layer){
  int err = -1;
  if(layer->routing_table != NULL){
    /*1. Liberar tabla de rutas layer->routing_table*/
    ipv4_route_table_free (layer->routing_table);
    free(layer);
    /*2. Cerrar la interfaz ethernet layer->iface*/
    printf("Cerrando la interfaz Ethernet\n");
     eth_close(layer->iface);
  }
  return err;
}


int ipv4_send(ipv4_layer_t *layer, ipv4_addr_t dst, uint8_t protocol, unsigned char * payload , int payload_length){
  /*ipv4_layer_t *layer: info capa ip,
    ipv4_addr_t dst: ip destino del mensaje ip,
    uint8_t protocol: protocolo,
    unsigned char * payload : datos a enviar,
    int payload_length: longitud de los datos que enviamos*/

   /*1. Hacer ipv4 lookup para encontrar ruta */
   mac_addr_t mac_dst;
   ipv4_route_t * ruta_ip = ipv4_route_table_lookup ( layer->routing_table, dst);
   char str[IPv4_STR_MAX_LENGTH] ;
   ipv4_addr_str ( ruta_ip->gateway_addr,str );

   /*1.1 ruta.geteway = 0.0.0.0 => arp_resolve(ip_dest)*/
   if (memcmp(ruta_ip->gateway_addr, IPv4_ZERO_ADDR, IPv4_ADDR_SIZE )==0){ //if (strcmp(str, "0.0.0.0")== 0){
     printf("\n\nDirectamente contectado: %s\n",str );
     arp_resolve(layer->iface, dst, mac_dst, layer->addr);
   }else{
     /*1.2 ruta.geteway != 0.0.0.0=> arp_resolve(ip_getway)*/
      printf("\n\nIP Gateway: %s\n",str );
      arp_resolve(layer->iface, ruta_ip->gateway_addr, mac_dst, layer->addr);
   }
   uint16_t type = 0x0800;
   /*2. Rellenar la cabecera IPv4(sin OPTION)*/
   struct ipv4_frame ipv4_message;
   ipv4_message.version_IHL = 0x45;
   ipv4_message.ip_type= 0x00;
   ipv4_message.total_length = htons(IPv4_HEADER_LENGTH + payload_length);
   ipv4_message.id= 0x0000;
   ipv4_message.flags_offset = 0x0000;
   ipv4_message.ttl = 0x40;
   ipv4_message.prot = protocol;
   ipv4_message.checksum = 0;
   memcpy(ipv4_message.src_addr, layer->addr, IPv4_ADDR_SIZE);
   memcpy(ipv4_message.dst_addr, dst, IPv4_ADDR_SIZE);


   memcpy(ipv4_message.payload, payload, payload_length);//En este punto lo que se hace es copiar
                                                        //los valores de la payload en el campo de payload de
                                                        //ip para luego poder enivarlo por ethernet
   ipv4_message.checksum = htons(ipv4_checksum((unsigned char *) &ipv4_message, IPv4_HEADER_LENGTH));
   printf("Enviamos mensaje:\n" );
   /*3. Enviar cabecera + payload con eth_send()*/
   int datagram_length = IPv4_HEADER_LENGTH + payload_length;
   int r = eth_send(layer->iface, mac_dst, type, (unsigned char *) &ipv4_message, datagram_length);
   //int r = eth_send(layer->iface, mac_dst, type, (unsigned char *) &ipv4_message, datagram_length);
   if (r == -1) {
     fprintf(stderr, "ERROR en eth_send)\n");
     return r;
   } else if (r == 0) {
     fprintf(stderr, "ERROR: No hay envio de bytes\n");
     return r;
  }
   /*4. Devolver longitud IP_payload enviado*/
   return (r - IPv4_HEADER_LENGTH) ;//en caso de que todo
}


int ipv4_recv(ipv4_layer_t* layer,uint8_t protocol, unsigned char buffer[], ipv4_addr_t sender, int buf_len, long int timeout){
  /*ipv4_layer_t* layer: info capa ip,
    uint8_t protocol: protocolo,
    unsigned char buffer[]; donde almacenarán los datos recibidos,
    ipv4_addr_t sender: se guarda la ip de quien nos ha mandado ,
    int buf_len: longitud de los datos recibidos,
    long int timeout*/
  /* Comprobar parámetros */
  if (layer->iface == NULL) {
    fprintf(stderr, "ipv4_recv(): ERROR: iface == NULL\n");
    return -1;
  }

  uint16_t type = 0x0800;
  struct ipv4_frame ipv4_message;
  bool is_my_response;

  /*2. Crear temporizador*/
  timerms_t timer;
  long int time_left = timerms_reset(&timer, timeout);
  int r;
  mac_addr_t mac_dst;

  /*2. Escuchar paquetes IPv4 con eth_recv*/
  do {
    time_left = timerms_left(&timer);

    /* Recibir trama del interfaz Ethernet */
    r = eth_recv (layer->iface, mac_dst, type, (unsigned char *) &ipv4_message, buf_len, time_left);
    if (r == -1) {
      fprintf(stderr, "ERROR en eth_recv()\n");
      return r;
    } else if (r == 0) {
      fprintf(stderr, "ERROR: No hay respuesta del Servidor Ethernet, ha expirado el temporizador\n");
      return r;
    }

    /*3. Ver si coincide el protocolo e IP*/
    is_my_response =  (memcmp(layer->addr, ipv4_message.dst_addr,IPv4_ADDR_SIZE)==0) && (protocol == ipv4_message.prot);

    if (is_my_response) {
      memcpy(buffer, (unsigned char *) &ipv4_message, buf_len);
      memcpy(sender,ipv4_message.src_addr, IPv4_ADDR_SIZE);
      printf("Es la respuesta que espero:\n" );
    }

  }while(!is_my_response );

  return (r - IPv4_HEADER_LENGTH);
}
