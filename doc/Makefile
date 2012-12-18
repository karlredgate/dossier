
# SMBIOS=smbios.xml
SMBIOS=try1.xml
SRCS=smbios.h smbios.c

build: $(SRCS)
	@echo Source generated

smbios.h: $(SMBIOS)
	xsltproc ../style/smbios2header.xslt $^ > $@

smbios.c: $(SMBIOS)
	xsltproc ../style/smbios2functions.xslt $^ > $@

clean: 
	rm -f $(SRCS)
