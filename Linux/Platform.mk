
DISTRO := $(shell lsb_release --short --id)
RELEASE := $(shell lsb_release -rs)
CODENAME := $(shell lsb_release -cs)

include $(wildcard Makefiles/$(DISTRO).mk Makefiles/$(DISTRO)$(RELEASE).mk Makefiles/$(CODENAME).mk)

LDSOFLAGS += -shared -Wl,-soname,$(SONAME)
