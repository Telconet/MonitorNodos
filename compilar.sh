#directorio actual
DIR_ACTUAL=`pwd`

#echo $DIR_ACTUAL
#$(mysql_config --cflags)
#compilamos los programas
#$(net-snmp-config --libs)

#echo $DIR_ACTUAL/lib/tsadclib1624.o

#rm -rf bin/*
#-Q
echo 'Compilando el monitor de nodos...'
gcc -W -mcpu=arm9 -lpthread -lnetsnmp -I/usr/local/mysql/include -I$DIR_ACTUAL/include $DIR_ACTUAL/source/monitorNodo.c $DIR_ACTUAL/source/utilidades.c $DIR_ACTUAL/source/IPC.c $DIR_ACTUAL/source/monitordef.c $DIR_ACTUAL/source/snmpdef.c $DIR_ACTUAL/source/modBD.c $DIR_ACTUAL/source/email.c $DIR_ACTUAL/source/mediciones.c $DIR_ACTUAL/source/ADC.c $DIR_ACTUAL/source/DIO.c -o $DIR_ACTUAL/bin/monnod $DIR_ACTUAL/lib/tsadclib1624.o $(mysql_config --libs) $(net-snmp-config --libs) 
echo 'Monitor de nodos compilado.'

#-mcpu=arm9
#$(net-snmp-config --cflags)

echo 'Compilando el controlador del monitor de nodos...'
gcc -W -I$DIR_ACTUAL/include $DIR_ACTUAL/source/controladorMonitor.c $DIR_ACTUAL/source/utilidades.c $DIR_ACTUAL/source/IPC.c $DIR_ACTUAL/source/controladordef.c -o $DIR_ACTUAL/bin/conmon $(net-snmp-config --libs)
echo 'Controlador de monitor de nodos compilado.'