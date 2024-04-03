
all:
	@.tools/mdt -b

install:
	@.tools/mdt -i

uninstall:
	@.tools/mdt -u

.PHONY: build install
