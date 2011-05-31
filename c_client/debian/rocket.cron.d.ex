#
# Regular cron jobs for the rocket package
#
0 4	* * *	root	[ -x /usr/bin/rocket_maintenance ] && /usr/bin/rocket_maintenance
