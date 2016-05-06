
MAJOR_VERSION=1
MINOR_VERSION=0
REVISION=0

# to handle travis-ci.org Ubuntu environment
INCLUDES += -I/usr/include/tcl8.5
CFLAGS += -fpic -g $(INCLUDES)
CXXFLAGS += -fpic -g $(INCLUDES) 
# LDFLAGS += -Wl,-dylib
LDFLAGS += $( pkg-config --libs tcl )

LINKNAME=libdossier.so
SONAME=$(LINKNAME).$(MAJOR_VERSION)
REALNAME=$(SONAME).$(MINOR_VERSION).$(REVISION)

include $(shell uname)/Platform.mk

default: build install
kit: build test install
build: $(UTILTARGET) all test

CLEANS += dossier dumpdmi
all: dossier dumpdmi
	# $(MAKE) -C src

CLEANS += dossier.o
dossier: dossier.o $(SONAME)
	$(CC) -o $@ $^ -lstdc++ $(LDFLAGS)

CLEANS += dumpdmi.o
dumpdmi: dumpdmi.o $(SONAME)
	$(CC) -o $@ $^ -lstdc++ $(LDFLAGS)

OBJS = \
	smbios.o \
	uuid.o

CLEANS += $(SONAME) $(LINKNAME)
$(SONAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lc
	rm -f $(LINKNAME)
	ln -s $(SONAME) $(LINKNAME)

smbios.o :: smbios.h
uuid.o :: uuid.h

test: dmi.xml
	@echo "make tests.log when ready"

dmi.xml: dumpdmi
	sudo ./runtest ./dumpdmi | xmllint --format --output dmi.xml -

CLEANS += tests.log
tests.log: dossier
	rm -f tests.log
	sudo ./runtest ./dossier tests/probe >> tests.log

install:

clean:
	rm -f $(OBJS) $(CLEANS)
