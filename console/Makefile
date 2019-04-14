#PLATFORMS += armhf
# CC for miss compilation
# add 'PLATFORMS += mips' as well
CXX = 
CXXFLAGS = -Wall -W -funroll-all-loops -mpreferred-stack-boundary=4 -mfpmath=sse -malign-double -fno-rtti -momit-leaf-frame-pointer -fomit-frame-pointer -fschedule-insns -fforce-mem -fforce-addr -fno-exceptions -falign-functions -falign-labels -falign-loops -falign-jumps -frename-registers -fno-cprop-registers -ffast-math -fno-exceptions -ftracer -O3
BINDIR = .
LDLIBS = 

SERVER_OBJS = $(BINDIR)/xapinit.o $(BINDIR)/xaptx.o $(BINDIR)/xapsend.o $(BINDIR)/simple-cache.o
CLIENT_OBJS = $(BINDIR)/xapinit.o $(BINDIR)/xaprx.o $(BINDIR)/xaptx.o $(BINDIR)/xaprecv.o

SERVER_EXEC = lumeterd
CLIENT_EXEC = lumeterc

PLATFORMS += x86
TARGETS = $(SERVER_EXEC) $(CLIENT_EXEC)

all:	$(PLATFORMS) $(GUI)

x86:
	CC=gcc
	mkdir -p ./build/x86
	$(MAKE) BINDIR=./build/x86 CC=gcc build

mips:
	CC=/opt/brcm/hndtools-mipsel-uclibc/bin/mipsel-uclibc-gcc
	mkdir -p ./build/mips
	$(MAKE) BINDIR=./build/mips CC=/opt/brcm/hndtools-mipsel-uclibc/bin/mipsel-uclibc-gcc build

armhf:
	CC=/opt/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-gcc
	mkdir -p ./build/armhf
	$(MAKE) BINDIR=./build/armhf CC=/opt/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-gcc build

build:	$(TARGETS)

$(BINDIR)/%.o: ../src/%.c
	$(CC) $(CFLAGS) -c -o $(BINDIR)/$*.o $< 

$(SERVER_EXEC):	$(SERVER_OBJS)
	$(CC) -o $(BINDIR)/$(SERVER_EXEC) $(SERVER_OBJS) $(LDLIBS)
$(CLIENT_EXEC):	$(CLIENT_OBJS)
	$(CC) -o $(BINDIR)/$(CLIENT_EXEC) $(CLIENT_OBJS) $(LDLIBS)

pack:
	echo Making archive
	mkdir -p versions
	cd .. && tar cjf ./xap/versions/xap-netstat-`date +%Y-%m-%d_%H-%M`.tar.bz2 ./xap/*.c ./xap/*.h ./xap/Makefile 

clean:		
	rm -rf ./build

