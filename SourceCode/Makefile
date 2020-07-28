
#
#
SHELL = /bin/sh
INSTALL = /usr/bin/install

DIRS=PlayVideo ShutDown

BUILDDIRS=$(DIRS:%=build-%)
INSTALLDIRS=$(DIRS:%=install-%)
CLEANDIRS=$(DIRS:%=clean-%)

all: $(BUILDDIRS)

$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

install: all $(INSTALLDIRS)
$(INSTALLDIRS):
	$(MAKE) -C $(@:install-%=%) install

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: subdirs $(INSTALLDIRS)
.PHONY: all install clean
