$(warning Included Fedora rules)

CXXFLAGS += $(shell pkg-config --cflags tcl )
CFLAGS += $(shell pkg-config --cflags tcl )
LDFLAGS += $(shell pkg-config --libs tcl )

include Makefiles/RPM.mk
