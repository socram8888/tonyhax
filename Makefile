
# Thanks to whoever made https://devhints.io/makefile!

include variables.mk

PACKAGE_FILE = tonyhax-$(TONYHAX_VERSION).zip
PACKAGE_CONTENTS = $(ENTRY_FILES:%=entrypoints/%) $(LOADER_FILES:%=loader/%) README.md LICENSE

.PHONY: modules clean

all: modules $(PACKAGE_FILE)

$(PACKAGE_FILE): $(PACKAGE_CONTENTS)
	$(RM) $(PACKAGE_FILE)
	zip -9 $(PACKAGE_FILE) $(PACKAGE_CONTENTS)

$(PACKAGE_CONTENTS): modules

modules:
	$(MAKE) -C entrypoints all
	$(MAKE) -C loader all

clean:
	$(MAKE) -C entrypoints clean
	$(MAKE) -C loader clean
