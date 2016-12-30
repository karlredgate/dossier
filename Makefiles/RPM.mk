
PWD := $(shell pwd)

distro_dependencies:
	@echo Check distro dependencies
	rpm --quiet --query git
	rpm --quiet --query rpm-build

distro_package:
	rm -rf rpm
	mkdir -p rpm/BUILD rpm/RPMS rpm/BUILDROOT
	rpmbuild --quiet -bb --buildroot=$(PWD)/rpm/BUILDROOT $(PACKAGE).spec

distro_build:
	@echo distro specific build

distro_test:
	@echo distro specific test

distro_clean:
	@echo distro specific clean
	$(RM) -rf rpm exports
