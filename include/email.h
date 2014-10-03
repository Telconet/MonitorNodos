#ifndef EMAIL_H
#define EMAIL_H

#include "definiciones.h"

#define TAMANO_MAX_EMAIL 8192
#define ARCHIVO_EMAIL_TEMPORAL "/tmp/tmpEmail"


int enviarEMail(char *destinatario, char *asunto, char* de, char *mensaje);

void enviarMultiplesEmails(char **destinatarios, int numeroDestinatarios, char *asunto, char *de, char *mensaje);

void *funcionEnvioEmail(void *ptr);

#endif

