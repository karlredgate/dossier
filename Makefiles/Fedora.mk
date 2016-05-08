
$(warning Loading rules for Fedora)

CXXFLAGS += $(shell pkg-config --cflags tcl )
CFLAGS += $(shell pkg-config --cflags tcl )
LDFLAGS += $(shell pkg-config --libs tcl )
