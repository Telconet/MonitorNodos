#ifndef SNMPDEF_H
#define SNMPDEF_H

#include "definiciones.h"

netsnmp_session *inicializarSistemaSnmp(char *ip_servidor, char *nombre, char *comunidad, int versionSNMP);

netsnmp_session *abrirSesion(netsnmp_session *sesion);

netsnmp_session **abrirMultiplesSesiones(char **ip_servidor, int numero_servidores, char *nombre, char *comunidad, int versionSNMP);

int enviarTrap(netsnmp_session *sesion, char *ip_agente, int trapGenerico, int trapEspecifico, char *OID, float valor);

void cerrarSistemaSnmp(netsnmp_session *sesion);


#endif

