ifndef DESTDIR
$(error DESTDIR is not set)
endif

# Cross-platform version of $(shell readlink -m "$(DESTDIR)")
ABSOLUTE_DESTDIR := $(shell mkdir -p $$(dirname "$(DESTDIR)") && cd $$(dirname "$(DESTDIR)") && pwd -P)/$$(basename "$(DESTDIR)")

.PHONY: all

all:
	mkdir -p "$(DESTDIR)"
	cp -r syntax themes contentDetection.json "$(DESTDIR)"/