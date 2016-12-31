$(warning Loading Linux Rules)

DISTRO := $(shell lsb_release -is)
RELEASE := $(shell lsb_release -rs)
CODENAME := $(shell lsb_release -cs)

LINKNAME=$(LIBRARY_NAME).so
SONAME=$(LINKNAME).$(MAJOR_VERSION)
REALNAME=$(SONAME).$(MINOR_VERSION).$(REVISION)

LIBRARY_TARGET=$(SONAME)

SHARED_LIB_FLAGS += -shared -Wl,-soname,$(SONAME)

include $(wildcard Makefiles/$(DISTRO).mk Makefiles/$(DISTRO)$(RELEASE).mk Makefiles/$(CODENAME).mk)
