#
#  Makefile for CSC501 Xinu.
#

INCLUDE = -I../h
CFLAGS	= -march=i586 -fno-builtin -c -Wall -O ${DEFS} ${INCLUDE}
SDEFS	= -I../h

CC      =       /usr/bin/gcc
cPP     =       /usr/bin/gcc -E
AS      =       /usr/bin/as
LD      =       /usr/bin/ld
MAKETD  =	/usr/X11R6/bin/makedepend
AWK	=	awk
LIB     =       ../lib
XINU    =       ./xinu

# Amount to move loaded image down in memory
BRELOC  =	0x100000
BOOTPLOC=	0x150000

#------------------------------------------------------------------------
# source files
#------------------------------------------------------------------------
COM =	comcntl.c	comgetc.c	comiin.c	cominit.c	\
	cominput.c	comoutput.c	comread.c	comintr.c

MON =   monitor.c       monarp.c        monbootp.c      monip.c         \
	monnet.c        monudp.c        mongpq.c        ethintr.c       \
	ethwrite.c      ethinit.c       ethdemux.c      ethwstrt.c      \
	ethrom.c        monboot.c       montimer.c      ethcmd.c	\
	monpci.c	mon3com.c

SYS =	blkcmp.c	blkequ.c	main.c		stacktrace.c	\
	chprio.c	clkinit.c	close.c		conf.c		\
	control.c	create.c	evec.c		freebuf.c	\
	freemem.c	getbuf.c	getc.c		getitem.c	\
	getmem.c	getpid.c	getprio.c	getstk.c	\
	gettime.c	gpq.c		i386.c		init.c		\
	insert.c	insertd.c	ioerr.c		ionull.c	\
	kill.c		kprintf.c	kputc.c		mark.c		\
	mkpool.c	newqueue.c	open.c		panic.c		\
	poolinit.c	putc.c		queue.c		read.c		\
	ready.c		receive.c	recvclr.c	recvtim.c	\
	resched.c	resume.c	scount.c	screate.c	\
	sdelete.c	send.c		setdev.c	setnok.c	\
	signal.c	signaln.c	sleep.c		sleep10.c	\
	sleep100.c	sleep1000.c	sreset.c	suspend.c	\
	unsleep.c	userret.c	wait.c		wakeup.c	\
	write.c		xdone.c		pci.c

TTY =	ttyalloc.c	ttycntl.c	ttygetc.c	ttyiin.c	\
	ttyinit.c	ttynew.c	ttyopen.c	ttyputc.c	\
	ttyread.c	ttywrite.c	ttyoin.c

SRC = ${COM} ${TTY} ${MON} ${SYS}

#------------------------------------------------------------------------
# object files
#------------------------------------------------------------------------
COMOBJ = ${COM:%.c=%.o}

MONOBJ = ${MON:%.c=%.o}

SYSOBJ = ${SYS:%.c=%.o}

TTYOBJ = ${TTY:%.c=%.o}

XOBJ = startup.o initialize.o intr.o clkint.o ctxsw.o

OBJ =	${COMOBJ} ${MONOBJ} ${SYSOBJ} ${TTYOBJ}		\
	moncksum.o monclkint.o comint.o ethint.o montftp.o

#------------------------------------------------------------------------
# make targets
#------------------------------------------------------------------------
all: xinu.elf

xinu.elf: Makefile Configuration ../h/conf.h ${XOBJ} ${OBJ} ${LIB}/libxc.a
	cp ../o/* .
	$(LD) -dn -Ttext 0x0 -e start ${XOBJ} ${OBJ} ${LIB}/libxc.a \
	-o ${XINU}.elf

clean: FRC
	rm -rf .d* *.o *.errs *.bak *nm* core ${XINU} ${XINU}.elf tags version
	rm -rf ../h/conf.h
	echo '0' > vn
	(cd ${LIB}/libxc; ${MAKE} clean)

depend: makedep
	sed -e '1,/^# DO NOT DELETE THIS LINE/!d' Makefile > Makefile.base
	cat Makefile.base makedep > Makefile
	rm Makefile.base makedep makedep1 makedeperrs
	@rm -f ../h/conf.h

undepend:
	sed -e '1,/^# DO NOT DELETE THIS LINE/!d' Makefile > Makefile.base
	mv Makefile.base Makefile

CFILES = ../*/*.c ../*/*/*.c

makedep: FRC
	@cat /dev/null >makedep
	@touch ../h/conf.h
	-(for i in ${CFILES} ; do \
		${CC} -M -w ${DEFS} -I../h ./$$i >> makedep; done) \
		2>makedeperrs
	sed -e 's,^\(.*\)\.o:,\1.o:,' makedep > makedep1
	${AWK} ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$3) > 78) { print rec; rec = $$0; } \
			else rec = rec " " $$3 } } \
		END { print rec } ' makedep1 > makedep
	@cat makedeperrs
	@(if [ -s makedeperrs ]; then false; fi)

xdepend: FRC
	${MAKETD} ${DEFS} -I../h ${CFILES}

#------------------------------------------------------------------------
#
#------------------------------------------------------------------------
tags: FRC
	ctags -R ..

../config/config:
	make -C ../config/ -f ../config/Makefile clean all

../h/conf.h ../sys/conf.c: ../config/config Configuration
	../config/config; mv conf.c ../sys 

${LIB}/libxc.a: FRC
	(cd ${LIB}/libxc; make install)

intr.o: ../sys/intr.S
	${CPP} ${SDEFS} ../sys/intr.S | ${AS} -o intr.o

clkint.o: ../sys/clkint.S
	${CPP} ${SDEFS} ../sys/clkint.S | ${AS} -o clkint.o

ctxsw.o: ../sys/ctxsw.S
	${CPP} ${SDEFS} ../sys/ctxsw.S | ${AS} -o ctxsw.o

ethint.o: ../mon/ethint.S
	${CPP} ${SDEFS} ../mon/ethint.S | ${AS} -o ethint.o

startup.o: ../sys/startup.S Makefile
	${CPP} ${SDEFS} -DBRELOC=${BRELOC} -DBOOTPLOC=${BOOTPLOC} \
	../sys/startup.S | ${AS} -o startup.o 

moncksum.o: ../mon/moncksum.S
	${CPP} ${SDEFS} ../mon/moncksum.S | ${AS} -o moncksum.o

monclkint.o: ../mon/monclkint.S
	${CPP} ${SDEFS} ../mon/monclkint.S | ${AS} -o monclkint.o

montftp.o: ../mon/montftp.c
	${CC} ${CFLAGS} -DBOOTPLOC=${BOOTPLOC} ../mon/montftp.c -o montftp.o

comint.o: ../com/comint.S
	${CPP} ${SDEFS} ../com/comint.S | ${AS} -o comint.o

initialize.o: $(OBJ) startup.o 
	sh mkvers.sh
	${CC} -c ${CFLAGS} -DVERSION=\""`cat version`"\" ../sys/initialize.c

${MONOBJ}:
	 ${CC} ${CFLAGS} ../mon/`basename $@ .o`.[c]
${COMOBJ}:
	 ${CC} ${CFLAGS} ../com/`basename $@ .o`.[c]
${SYSOBJ}:
	 ${CC} ${CFLAGS} ../sys/`basename $@ .o`.[c]
${TTYOBJ}:
	 ${CC} ${CFLAGS} ../tty/`basename $@ .o`.[c]	 
FRC:
#------------------------------------------------------------------------

# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT

