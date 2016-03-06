DIRS = src/sockrpip src/php_librpip src/socktest
IDIRS = src/php_librpip

INSTALL = /usr/local

REL_VERSION = 0.1.0

all :
	@set -e; for d in $(DIRS); do $(MAKE) -C $$d -j REL_VERSION=${REL_VERSION}; done


install :
	killall sockrpip
	mkdir -p ${INSTALL}/etc/sockrpip.d
	mkdir -p ${INSTALL}/bin/sockrpip
	mkdir -p /var/lib/sockrpip
	if id -u sockrpip >/dev/null 2>&1; then touch /var/lib/sockrpip; else groupadd -rf sockrpip; useradd -r -d /var/lib/sockrpip -s /usr/sbin/nologin -g sockrpip sockrpip; fi
	chown -R sockrpip:sockrpip /var/lib/sockrpip
	cp bin/sockrpip ${INSTALL}/bin/sockrpip
	cp etc/sockrpip.conf ${INSTALL}/etc/sockrpip.d
	runuser -u sockrpip /usr/local/bin/sockrpip/sockrpip
	# add systemd config, start service
	@set -e; for d in $(IDIRS); do $(MAKE) -C $$d install ; done
	cp config/librpip.ini /etc/php5/mods-available
	if [ -d /etc/php5/cli ]; then ln -fsr /etc/php5/mods-available/librpip.ini /etc/php5/cli/conf.d/49-librpip.ini; fi
	if [ -d /etc/php5/apache2 ]; then ln -fsr /etc/php5/mods-available/librpip.ini /etc/php5/apache2/conf.d/49-librpip.ini; service apache2 restart; fi	


clean :
	find . -type f -name '*DS_Store' -exec rm -f '{}' ';'
	find . -type f -name '._*' -exec rm -f '{}' ';'
	@set -e; for d in $(DIRS); do $(MAKE) -C $$d clean ; done

