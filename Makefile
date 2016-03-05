DIRS = src/sockrpip src/phprpip src/socktest 

INSTALL	= /usr/local

REL_VERSION = 0.1.0

all :
	@set -e; for d in $(DIRS); do $(MAKE) -C $$d -j REL_VERSION=${REL_VERSION}; done


install :
	mkdir -p ${INSTALL}/etc/phprpip
	mkdir -p ${INSTALL}/bin/phprpip	

clean :
	find . -type f -name '*DS_Store' -exec rm -f '{}' ';'
	find . -type f -name '._*' -exec rm -f '{}' ';'
	@set -e; for d in $(DIRS); do $(MAKE) -C $$d clean ; done

