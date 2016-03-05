DIRS = src/sockrpip src/php_librpip src/socktest
IDIRS = src/php_librpip

INSTALL = /usr/local

REL_VERSION = 0.1.0

all :
	@set -e; for d in $(DIRS); do $(MAKE) -C $$d -j REL_VERSION=${REL_VERSION}; done


install :
	mkdir -p ${INSTALL}/etc/php-librpip
	mkdir -p ${INSTALL}/bin/php-librpip
	cp bin/sockrpip ${INSTALL}/bin/php-librpip
	cp etc/sockrpip.conf ${INSTALL}/etc/php-librpip
	# add systemd config, start service
	@set -e; for d in $(IDIRS); do $(MAKE) -C $$d install ; done
	cp config/php_librpip.ini /etc/php5/mods-available
	# link to ini, restart apache

clean :
	find . -type f -name '*DS_Store' -exec rm -f '{}' ';'
	find . -type f -name '._*' -exec rm -f '{}' ';'
	@set -e; for d in $(DIRS); do $(MAKE) -C $$d clean ; done

