semantic_version := $(shell cat VERSION)
git_rev := $(shell git rev-parse --short HEAD)
version_suffix := $(shell echo $${BUILD_NUMBER:-git.$(git_rev)})
#version_suffix := $(shell if [[ -n "$${BUILD_NUMBER}" ]]; then echo "r.$${BUILD_NUMBER}"; else echo "git.$(git_rev)"; fi)
version := $(semantic_version).$(version_suffix)
workdir = deb-dist-$(1)

distro_dependencies:
	@echo " ===== Check for packages required to build ==== "
	dpkg-query --showformat='$${PackageSpec}-$${Version}\n' --show equivs
	dpkg-query --showformat='$${PackageSpec}-$${Version}\n' --show git

define packdeb
	rm -rf $(workdir)
	mkdir -p $(workdir)
	tar cf - $(PACKAGE).control COPYRIGHT changelog $(shell ./debfiles) | tar -C $(workdir) -xf -
	sed -ri 's/__version__/$(version)/' $(workdir)/changelog
	sed -ri 's/__dist__/$(1)/' $(workdir)/changelog
	cd $(workdir) && equivs-build --full $(PACKAGE).control
endef

package-deb: default
	@echo "This rule was called for backward compatability"
	@echo "The Jenkins job should be changed"

distro_package:
	$(call packdeb,precise)

distro_build:
	@echo "No Debian specific build rules"

validate-build:
	: ssh 192.168.16.104 rm -rf diagnostics
	: ssh 192.168.16.104 mkdir diagnostics
	: scp -r . 192.168.16.104:diagnostics
	: ssh 192.168.16.104 '( cd diagnostics ; make )'
	: ssh 192.168.16.104 '( sudo diagnostics/examples/install-deb testing )'
	: ssh 192.168.16.104 sudo http_proxy=http://192.168.16.100:3128/ call-home

distro_test:
	@echo "No tests for Debian distros"

distro_clean:
	: do not use make var - you might get 'rm -rf *'
	rm -rf deb-dist-*
