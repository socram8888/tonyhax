
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

.PHONY: clean

all: $(BOOT_CD_FILES)

clean:
	$(RM) $(BOOT_CD_FILES)

$(BOOT_CD_FILES): ../loader/tonyhax.exe cd.xml licensee.dat system.cnf
	mkpsxiso -y cd.xml
