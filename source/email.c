#include "email.h"


/**
 *Esta rutina se encarga de enviar e-mails de notificacion
 */
int enviarEMail(char *destinatario, char *asunto, char* de, char *mensaje){
    
    char *contenidoArchivo = malloc(sizeof(char)*TAMANO_MAX_EMAIL);
    
    //vemos que no sobrepase la longitud del buffer
    int longitud = strlen(destinatario) + strlen(asunto) + strlen(mensaje) + strlen(mensaje);
    
    if(contenidoArchivo == NULL || longitud > (TAMANO_MAX_EMAIL - 200)){
        printf("ERROR: No se pudo crear el e-mail\n");
        return -1;
    }
    
    //Creamos el archivo con los datos
    int total = 0;
    memset(contenidoArchivo, 0, TAMANO_MAX_EMAIL);
    
    snprintf(contenidoArchivo, TAMANO_MAX_EMAIL, "To: %s\nFrom: %s\nSubject: %s\n\n%s\0", destinatario, de, asunto, mensaje);
    
    char *nombreArchivo = malloc(sizeof(char)*200);
    memset(nombreArchivo, 0, 200);
    
    sleep(3);
    struct timeval tv;
    gettimeofday(&tv,NULL);
    srand(tv.tv_usec);
    int r1 = rand();
    int r2 = rand();
    
#ifdef DEBUG
    printf("Numero aleatorio: %d, %d\n",r1,r2);
#endif

    snprintf(nombreArchivo, 200, "%s_%d_%d", ARCHIVO_EMAIL_TEMPORAL,r1, r2);
    
    FILE *archivoEmail = fopen(nombreArchivo, "w");
    //Escribimos el archivo
    fwrite(contenidoArchivo, 1, strlen(contenidoArchivo),archivoEmail);
    fflush(archivoEmail);
    free(contenidoArchivo);
    
    //Mensaje y asunto son son liberados por quienes usan esta rutina
    
    //Creamos el comando
    char *comando = malloc(sizeof(char)*TAMANO_MAX_EMAIL);
    memset(comando, 0, TAMANO_MAX_EMAIL);
    
    if(comando == NULL){
        printf("ERROR: No se pudo crear el comando de envio de e-mail\n");
        return -1;
    }

#ifdef DEBUG
    snprintf(comando, TAMANO_MAX_EMAIL, "/usr/sbin/ssmtp -v %s < %s &\0", destinatario,nombreArchivo);
#else
    snprintf(comando, TAMANO_MAX_EMAIL, "/usr/sbin/ssmtp %s < %s &\0", destinatario,nombreArchivo);
#endif

    fclose(archivoEmail);
    free(nombreArchivo);
    
    pthread_t thread_email;
    int iret1;
    
    iret1 = pthread_create( &thread_email, NULL, funcionEnvioEmail, (void*) comando);
  
}

/**
 *funcion que maneja el thread de envio de emails.
 */
void *funcionEnvioEmail(void *ptr)
{
     char *comando;
     comando = (char *) ptr;
     system(comando);
     free(comando);
}

/**
 *Envio de multiples emails.
 */
void enviarMultiplesEmails(char **destinatarios, int numeroDestinatarios, char *asunto, char *de, char *mensaje){
    
    int i = 0;
    
    for(i = 0; i< numeroDestinatarios; i++){
        //printf("destinatario %s\n", destinatarios[i]);
        enviarEMail(destinatarios[i],asunto, de, mensaje);
    }
    
    return;
}
