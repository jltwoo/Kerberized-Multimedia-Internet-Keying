#SSERVER_OBJS = MikeyTestConfig.o sserver.o
GCKS_1_OBJS = MikeyTestConfig.o gcks1.o
SCLIENT_OBJS = MikeyTestConfig.o sclient.o
SCLIENT2_OBJS = MikeyTestConfig.o sclient2.o

INC = ../krb5-1.8.3/src/include/
INCDIRS   = -I${INC}
LIBS = -lmcrypto -lmsip -lmikey -lrt 
CXX = g++
DEBUG = -g
CFLAGS = -Wall $(INCDIRS) -c $(DEBUG)
LFLAGS = -Wall $(DEBUG) -L/usr/local/lib

LIBMIKEY_WRAPPER_SO = libmikey_wrapper.so*

#all: sserver sclient sclient2 gcks libmikey_wrapper
all: gcks1 sclient sclient2 gcks2 libmikey_wrapper

test: main.c
	gcc -DEBUG main.c -o testmain -lmikey_wrapper

#sserver: $(SSERVER_OBJS)
#	$(CXX) $(LFLAGS) $(SSERVER_OBJS) -o sserver -lkrb5 $(LIBS)

gcks1: $(GCKS_1_OBJS)
	$(CXX) $(LFLAGS) $(GCKS_1_OBJS) -o gcks1 -lkrb5 $(LIBS)

sclient: $(SCLIENT_OBJS)
	$(CXX) $(LFLAGS) $(SCLIENT_OBJS) -o sclient -lkrb5 $(LIBS)

sclient2: $(SCLIENT2_OBJS)
	$(CXX) $(LFLAGS) $(SCLIENT2_OBJS) -o sclient2 -lkrb5 $(LIBS)

#gcks: gcks.o MikeyTestConfig.o
#	$(CXX) gcks.o MikeyTestConfig.o -o gcks $(LIBS) -lkrb5
gcks2: gcks2.o MikeyTestConfig.o
	$(CXX) gcks2.o MikeyTestConfig.o -o gcks2 $(LIBS) -lkrb5

libmikey_wrapper: MikeyTestConfig.o libmikey_wrapper.h libmikey_wrapper.cxx
	$(CXX) -shared -Wl,-soname,libmikey_wrapper.so.1 \
	-o libmikey_wrapper.so.1.0.0 MikeyTestConfig.o libmikey_wrapper.cxx -lmikey

#sserver.o : MikeyTestConfig.h sample.h sserver.cxx
#	$(CXX) $(CFLAGS) sserver.cxx
gcks1.o : MikeyTestConfig.h sample.h gcks1.cxx
	$(CXX) $(CFLAGS) gcks1.cxx

sclient.o : sample.h sclient.cxx
	$(CXX) $(CFLAGS) sclient.cxx

sclient2.o : sample.h sclient2.cxx
	$(CXX) $(CFLAGS) sclient2.cxx

gcks2.o : gcks2.cxx
	${CXX} $(CFLAGS) gcks2.cxx

MikeyTestConfig.o: MikeyTestConfig.h MikeyTestConfig.cxx
	$(CXX) $(CFLAGS) MikeyTestConfig.cxx $(LIBS)

libmikey_wrapper.o: MikeyTestConfig.h libmikey_wrapper.h libmikey_wrapper.cxx
	$(CXX) libmikey_wrapper

install-includes: libmikey_wrapper.h
	cp libmikey_wrapper.h /usr/local/include

install-libraries: $(LIBMIKEY_WRAPPER_SO)
	cp libmikey_wrapper.so.1.0.0 /usr/local/lib
	cd /usr/local/lib
	ln -sf libmikey_wrapper.so.1.0.0 libmikey_wrapper.so
	ldconfig

install: install-includes install-libraries

clean:
#	rm -f *.o sclient sclient2 sserver testmain gcks
	rm -f *.o sclient sclient2 testmain gcks1 gcks2
	rm -f $(LIBMIKEY_WRAPPER_SO)

