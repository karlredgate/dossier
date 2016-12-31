
DISTRO := OSX
RELEASE := $(shell sw_vers -productVersion)

ifeq ($(RELEASE),10.11.6)
CODENAME := ElCapitan
endif

LINKNAME=$(LIBRARY_NAME).dylib
REAL_NAME=$(LIBRARY_NAME).$(MAJOR_VERSION).$(MINOR_VERSION).dylib

LIBRARY_TARGET=$(LINKNAME)

SHARED_LIB_FLAGS= -dynamiclib
# Mac OSX package

distro_dependencies: release_dependencies
	@echo Check distro dependencies
	[ -x /usr/bin/xsltproc ]
	: do not use fpm anymore
	: [ -x /usr/bin/fpm ]

distro_package: release_package
	@echo Generic Darwin packaging
	: fpm -s dir -t osxpkg -n $(PACKAGE) dist

distro_build: release_build
	@echo Generic Darwin build

distro_test: release_test
	@echo Generic Darwin test

distro_clean: release_clean
	@echo Generic Darwin clean

# VERSION := $(shell sw_vers -productVersion)
# include Makefiles/Darwin$(VERSION).mk

# For now do not include codename makefile, since it is included by release file
include $(wildcard Makefiles/$(DISTRO).mk Makefiles/Darwin$(RELEASE).mk )

# NOTES:
# Add "-H" to compile to check include paths
