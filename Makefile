
#cross compiler
ARMCC=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/bin/gcc
BASE=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/bin 
DIR_ACTUAL=`pwd`

monnod: ADC.o monitorNodo.o utilidades.o IPC.o monitordef.o snmpdef.o modBD.o email.o mediciones.o DIO.o
	$(ARMCC) -Wall -Wno-trigraphs -mcpu=arm9 -lpthread -lnetsnmp -o $($DIR_ACTUAL)/bin/monnod -g $(DIR_ACTUAL)/lib/tsadclib1624.o $(DIR_ACTUAL)/ADC.o $(DIR_ACTUAL)/monitorNodo.o $(DIR_ACTUAL)/utilidades.o $(DIR_ACTUAL)/IPC.o $(DIR_ACTUAL)/monitordef.o $(DIR_ACTUAL)/snmpdef.o $(DIR_ACTUAL)/modBD.o $(DIR_ACTUAL)/email.o $(DIR_ACTUAL)/mediciones.o $(DIR_ACTUAL)/DIO.o $(mysql_config --libs) $(net-snmp-config --libs) 

ADC.o: source/ADC.c include/ADC.h
	$(ARMCC) -c -mcpu=arm9 -Wall -Wno-trigraphs -B $(BASE) -I $(DIR_ACTUAL)/include source/ADC.c 

monitorNodo.o: source/monitorNodo.c
	$(ARMCC) -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/monitorNodo.c

utilidades.o: source/utilidades.c include/utilidades.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/utilidades.c

IPC.o: source/IPC.c include/IPC.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/IPC.c

monitordef.o: source/monitordef.c include/monitordef.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/monitordef.c

snmpdef.o: source/snmpdef.c include/snmpdef.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/snmpdef.c

modBD.o: source/modBD.c include/modBD.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/modBD.c

email.o: source/email.c include/email.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/email.c

mediciones.o: source/mediciones.c include/mediciones.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/mediciones.c

DIO.o: source/DIO.c include/DIO.h
	$(ARMCC) -g -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/DIO.c

archive:
	tar -cf Backup/warmup2-`date +%Y.%m.%d`_`date +%H.%M.%S`.tar *.c *.h README Makefile

clean:
	rm -f $(DIR_ACTUAL)/*.o $(DIR_ACTUAL)/bin/monnod


