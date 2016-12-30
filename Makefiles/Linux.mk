$(warning Loading Linux Rules)

DISTRO := $(shell lsb_release -is)
RELEASE := $(shell lsb_release -rs)
CODENAME := $(shell lsb_release -cs)

include $(wildcard Makefiles/$(DISTRO).mk Makefiles/$(DISTRO)$(RELEASE).mk Makefiles/$(CODENAME).mk)
