#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

#excute upgrade application
/usr/bin/wrt-upgrade

#remove unuse databases
rm /opt/dbspace/.wrt.db
rm /opt/dbspace/.wrt.db-journal

rm -r /opt/share/widget

rm /opt/usr/dbspace/.wrt_custom_handler.db
rm /opt/usr/dbspace/.wrt_custom_handler.db-journal
rm /opt/usr/dbspace/.wrt_i18n.db
rm /opt/usr/dbspace/.wrt_i18n.db-journal
