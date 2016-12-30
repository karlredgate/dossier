
include Makefiles/Debian.mk

$(warning Loading rules for Ubuntu)

CXXFLAGS += -I/usr/include/tcl
LDFLAGS += -ltcl
