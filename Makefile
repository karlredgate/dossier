
PACKAGE = dossier
PWD := $(shell pwd)
DEPENDENCIES = tcl

MAJOR_VERSION=1
MINOR_VERSION=0
REVISION=0


# migrate this to platform
CFLAGS += -fpic -g
CXXFLAGS += -fpic -g
# LDFLAGS += -Wl,-dylib
LDFLAGS += -g

LIBRARY_NAME = libdossier

default: build install

OS := $(shell uname -s)
include Makefiles/$(OS).mk
CXXFLAGS += -I$(OS)

kit: build test install
build: $(UTILTARGET) all test

CLEANS += dossier dumpdmi
all: dossier dumpdmi
	# $(MAKE) -C src

CLEANS += dossier.o
dossier: dossier.o $(LIBRARY_TARGET)
	$(CXX) -o $@ $^ -lstdc++ $(LDFLAGS) -ltcl

CLEANS += dumpdmi.o
dumpdmi: dumpdmi.o $(LIBRARY_TARGET)
	$(CXX) -o $@ $^ -lstdc++ $(LDFLAGS) -ltcl

OBJS = \
	smbios.o \
	uuid.o

CLEANS += $(LIBRARY_TARGET) $(LINKNAME)
$(LIBRARY_TARGET): $(OBJS)
	$(CXX) $(SHARED_LIB_FLAGS) -o $@ $^ -lc $(LDFLAGS) -ltcl
	: rm -f $(LINKNAME)
	: ln -s $(LIBRARY_TARGET) $(LINKNAME)

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
