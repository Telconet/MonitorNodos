
#cross compiler
ARMCC=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/bin/gcc
BASE=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/bin 
DIR_ACTUAL=`pwd`
LIBRERIASOS=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/bin/lib
LIBRERIASSNMP=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/usr/local/lib/libnetsnmp.a

monnod: ADC.o monitorNodo.o utilidades.o IPC.o monitordef.o snmpdef.o modBD.o email.o mediciones.o DIO.o
	$(ARMCC) -Wall -fPIC -Wno-trigraphs -mcpu=arm9 -lpthread -lm -o bin/monnod -g lib/tsadclib1624.o obj/ADC.o obj/monitorNodo.o obj/utilidades.o obj/IPC.o obj/monitordef.o obj/snmpdef.o obj/modBD.o obj/email.o obj/mediciones.o obj/DIO.o /usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/arm-linux/usr/local/lib/libnetsnmp.a

ADC.o: source/ADC.c include/ADC.h
	$(ARMCC) -c -fPIC -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/ADC.c -o obj/ADC.o

monitorNodo.o: source/monitorNodo.c
	$(ARMCC) -c -fPIC -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/monitorNodo.c -o obj/monitorNodo.o

utilidades.o: source/utilidades.c include/utilidades.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/utilidades.c -o obj/utilidades.o

IPC.o: source/IPC.c include/IPC.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/IPC.c -o obj/IPC.o

monitordef.o: source/monitordef.c include/monitordef.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/monitordef.c -o obj/monitordef.o

snmpdef.o: source/snmpdef.c include/snmpdef.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/snmpdef.c -o obj/snmpdef.o

modBD.o: source/modBD.c include/modBD.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/modBD.c -o obj/modBD.o

email.o: source/email.c include/email.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/email.c -o obj/email.o

mediciones.o: source/mediciones.c include/mediciones.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/mediciones.c -o obj/mediciones.o

DIO.o: source/DIO.c include/DIO.h
	$(ARMCC) -g -fPIC -c -mcpu=arm9 -Wall -Wno-trigraphs -I $(DIR_ACTUAL)/include source/DIO.c -o obj/DIO.o

archive:
	tar -cf Backup/monitornodos-`date +%Y.%m.%d`_`date +%H.%M.%S`.tar *.c *.h README Makefile

clean:
	rm -f obj/*.o bin/monnod


