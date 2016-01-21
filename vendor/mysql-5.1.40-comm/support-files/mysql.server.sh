#!/bin/sh
# Copyright Abandoned 1996 TCX DataKonsult AB & Monty Program KB & Detron HB
# This file is public domain and comes with NO WARRANTY of any kind

# MySQL daemon start/stop script.

# Usually this is put in /etc/init.d (at least on machines SYSV R4 based
# systems) and linked to /etc/rc3.d/S99mysql and /etc/rc0.d/K01mysql.
# When this is done the mysql server will be started when the machine is
# started and shut down when the systems goes down.

# Comments to support chkconfig on RedHat Linux
# chkconfig: 2345 64 36
# description: A very fast and reliable SQL database engine.

# Comments to support LSB init script conventions
### BEGIN INIT INFO
# Provides: mysql
# Required-Start: $local_fs $network $remote_fs
# Should-Start: ypbind nscd ldap ntpd xntpd
# Required-Stop: $local_fs $network $remote_fs
# Default-Start:  2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: start and stop MySQL
# Description: MySQL is a very fast and reliable SQL database engine.
### END INIT INFO
 
# If you install MySQL on some other places than @prefix@, then you
# have to do one of the following things for this script to work:
#
# - Run this script from within the MySQL installation directory
# - Create a /etc/my.cnf file with the following information:
#   [mysqld]
#   basedir=<path-to-mysql-installation-directory>
# - Add the above to any other configuration file (for example ~/.my.ini)
#   and copy my_print_defaults to /usr/bin
# - Add the path to the mysql-installation-directory to the basedir variable
#   below.
#
# If you want to affect other MySQL variables, you should make your changes
# in the /etc/my.cnf, ~/.my.cnf or other MySQL configuration files.

# If you change base dir, you must also change datadir. These may get
# overwritten by settings in the MySQL configuration files.

conf=@BH_CONF@
user=@BH_USER@

basedir=
datadir=

MYSQLD=mysqld

# Default value, in seconds, afterwhich the script should timeout waiting
# for server start. 
# Value here is overriden by value in my.cnf. 
# 0 means don't wait at all
# Negative numbers mean to wait indefinitely
service_startup_timeout=8

# The following variables are only set for letting mysql.server find things.

# Set some defaults
pid_file=
server_pid_file=
use_mysqld_safe=1
#user=@MYSQLD_USER@
if test -z "$basedir"
then
  basedir=@prefix@
  bindir=@bindir@
  if test -z "$datadir"
  then
    datadir=@localstatedir@
  fi
  sbindir=@sbindir@
  libexecdir=@libexecdir@
else
  bindir="$basedir/bin"
  if test -z "$datadir"
  then
    datadir="$basedir/data"
  fi
  sbindir="$basedir/sbin"
  libexecdir="$basedir/libexec"
fi

# datadir_set is used to determine if datadir was set (and so should be
# *not* set inside of the --basedir= handler.)
datadir_set=

#
# Use LSB init script functions for printing messages, if possible
#
lsb_functions="/lib/lsb/init-functions"
if test -f $lsb_functions ; then
  . $lsb_functions
else
  log_success_msg()
  {
    echo " SUCCESS! $@"
  }
  log_failure_msg()
  {
    echo " ERROR! $@"
  }
fi

PATH=/sbin:/usr/sbin:/bin:/usr/bin:$basedir/bin
export PATH

mode=$1    # start or stop
shift
other_args="$*"   # uncommon, but needed when called from an RPM upgrade action
           # Expected: "--skip-networking --skip-grant-tables"
           # They are not checked here, intentionally, as it is the resposibility
           # of the "spec" file author to give correct arguments only.

case `echo "testing\c"`,`echo -n testing` in
    *c*,-n*) echo_n=   echo_c=     ;;
    *c*,*)   echo_n=-n echo_c=     ;;
    *)       echo_n=   echo_c='\c' ;;
esac

parse_server_arguments() {
  for arg do
    case "$arg" in
      --basedir=*)  basedir=`echo "$arg" | sed -e 's/^[^=]*=//'`
                    bindir="$basedir/bin"
		    if test -z "$datadir_set"; then
		      datadir="$basedir/data"
		    fi
		    sbindir="$basedir/sbin"
		    libexecdir="$basedir/libexec"
        ;;
      --datadir=*)  datadir=`echo "$arg" | sed -e 's/^[^=]*=//'`
		    datadir_set=1
	;;
      --user=*)  user=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --pid-file=*) server_pid_file=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --service-startup-timeout=*) service_startup_timeout=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --use-mysqld_safe) use_mysqld_safe=1;;
      --use-manager)     use_mysqld_safe=0;;
    esac
  done
}

parse_manager_arguments() {
  for arg do
    case "$arg" in
      --pid-file=*) pid_file=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --user=*)  user=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    esac
  done
}

is_mysqld_running() {
  # If pid file is provided then get the PID from pid file
  # otherwise use the PID passed to this function.
  pidfile=$1
  PID=$2
  [ -f "$pidfile" ] && PID=`cat "$pidfile"` 
  if @CHECK_PID@
  then
    if @FIND_PROC@
    then
      return 1;
    fi
  fi
  return 0  
}

is_redhat()
{
  lsb_os=`lsb_release -i`
  if [ $? = 0 ];
  then
    isredhat=`echo $lsb_os | grep RedHat 2>/dev/null`
    test ! -z "$isredhat" && return 1;
  fi
  return 0;
}

wait_for_pid () {
  verb="$1"
  manager_pid="$2"  # process ID of the program operating on the pid-file
  i=0
  avoid_race_condition="by checking again"
  while test $i -ne $service_startup_timeout ; do

    case "$verb" in
      'created')
        # wait for a PID-file to pop into existence.
        test -s $pid_file && i='' && break
        ;;
      'removed')
        # wait for this PID-file to disappear
        #test ! -s $pid_file && i='' && break
        if [ ! -s $pid_file ]; then
          # Do extra checking since sometimes it takes time to shutdown the compete process
          # even though the pid file is removed by the process.
          is_mysqld_running "" $manager_pid 
          if [ $? = 0 ]; then
            i=''
            break
          fi
        fi
        ;;
      *)
        echo "wait_for_pid () usage: wait_for_pid created|removed manager_pid"
        exit 1
        ;;
    esac

    # if manager isn't running, then pid-file will never be updated
    if test "$use_mysqld_safe" = "0" -a -n "$manager_pid" ; then
      if kill -0 "$manager_pid" 2>/dev/null; then
        :  # the manager still runs
      else
        # The manager may have exited between the last pid-file check and now.  
        if test -n "$avoid_race_condition"; then
          avoid_race_condition=""
          continue  # Check again.
        fi

        # there's nothing that will affect the file.
        log_failure_msg "Manager of pid-file quit without updating file."
        return 1  # not waiting any more.
      fi
    fi

    echo $echo_n ".$echo_c"
    i=`expr $i + 1`
    sleep 1
  done

  if test -z "$i" ; then
    log_success_msg
    return 0
  else
    log_failure_msg
    return 1
  fi
}

# Get arguments from the my.cnf file,
# the only group, which is read from now on is [mysqld]
if test -x ./bin/my_print_defaults
then
  print_defaults="./bin/my_print_defaults"
elif test -x $bindir/my_print_defaults
then
  print_defaults="$bindir/my_print_defaults"
elif test -x $bindir/mysql_print_defaults
then
  print_defaults="$bindir/mysql_print_defaults"
else
  # Try to find basedir in /etc/my-ib.cnf
  print_defaults=
  if test -r $conf
  then
    subpat='^[^=]*basedir[^=]*=\(.*\)$'
    dirs=`sed -e "/$subpat/!d" -e 's//\1/' $conf`
    for d in $dirs
    do
      d=`echo $d | sed -e 's/[ 	]//g'`
      if test -x "$d/bin/my_print_defaults"
      then
        print_defaults="$d/bin/my_print_defaults"
        break
      fi
      if test -x "$d/bin/mysql_print_defaults"
      then
        print_defaults="$d/bin/mysql_print_defaults"
        break
      fi
    done
  else
    echo "Error - $conf was not found! Please make sure Infobright installation completed and activated successfully."
    exit 1
  fi

  # Hope it's in the PATH ... but I doubt it
  test -z "$print_defaults" && print_defaults="my_print_defaults"
fi

#
# Read defaults file from 'basedir'.   If there is no defaults file there
# check if it's in the old (depricated) place (datadir) and read it from there
#

extra_args=""
if test -r "$conf"
then
  extra_args="-c $conf"
fi

parse_server_arguments `$print_defaults $extra_args mysqld server mysql_server mysql.server`

# Look for the pidfile 
parse_manager_arguments `$print_defaults $extra_args manager`

#
# Set pid file if not given
#
if test -z "$pid_file"
then
  pid_file=$datadir/mysqlmanager-`@HOSTNAME@`.pid
else
  case "$pid_file" in
    /* ) ;;
    * )  pid_file="$datadir/$pid_file" ;;
  esac
fi
if test -z "$server_pid_file"
then
  server_pid_file=$datadir/`@HOSTNAME@`.pid
else
  case "$server_pid_file" in
    /* ) ;;
    * )  server_pid_file="$datadir/$server_pid_file" ;;
  esac
fi

case "$mode" in
  'start')
    # Start daemon

    # Safeguard (relative paths, core dumps..)
    cd $basedir

    manager=$bindir/mysqlmanager
    if test -x $libexecdir/mysqlmanager
    then
      manager=$libexecdir/mysqlmanager
    elif test -x $sbindir/mysqlmanager
    then
      manager=$sbindir/mysqlmanager
    fi

    echo $echo_n "Starting MySQL"
    if test -x $manager -a "$use_mysqld_safe" = "0"
    then
      if test -n "$other_args"
      then
        log_failure_msg "MySQL manager does not support options '$other_args'"
        exit 1
      fi
      # Give extra arguments to mysqld with the my.cnf file. This script may
      # be overwritten at next upgrade.
      "$manager" \
        --mysqld-safe-compatible \
        --user="$user" \
        --pid-file="$pid_file" >/dev/null 2>&1 &
      wait_for_pid created $!; return_value=$?

      # Make lock for RedHat / SuSE
      if test -w /var/lock/subsys
      then
        touch /var/lock/subsys/mysqlmanager
      fi
      exit $return_value
    elif test -x $bindir/mysqld_safe
    then
      # Give extra arguments to mysqld with the my.cnf file. This script
      # may be overwritten at next upgrade.
      pid_file=$server_pid_file
      is_mysqld_running $pid_file "" 
      [ $? != 0 ] && log_failure_msg "A mysqld process is already running." && exit 0;
      [ -f $pid_file ] && rm -f $pid_file
      with_sudo=""
      is_redhat
      [ $? -eq 1 ] && with_sudo="sudo -u $user"
      $with_sudo $bindir/mysqld_safe --defaults-file=$conf --log-queries-not-using-indexes --user=$user --pid-file=$server_pid_file >/dev/null 2>&1 &
      wait_for_pid created $!; return_value=$?

      # Make lock for RedHat / SuSE
      if test -w /var/lock/subsys
      then
        touch /var/lock/subsys/mysqld-ib
      fi
      exit $return_value
    else
      log_failure_msg "Couldn't find MySQL manager ($manager) or server ($bindir/mysqld_safe)"
    fi
    ;;

  'stop')
    # Stop daemon. We use a signal here to avoid having to know the
    # root password.

    # The RedHat / SuSE lock directory to remove
    lock_dir=/var/lock/subsys/mysqlmanager

    # If the manager pid_file doesn't exist, try the server's
    if test ! -s "$pid_file"
    then
      pid_file=$server_pid_file
      lock_dir=/var/lock/subsys/mysqld-ib
    fi

    if test -s "$pid_file"
    then
      is_mysqld_running $pid_file ""
      [ $? = 0 ] && exit 0;
      mysqlmanager_pid=`cat $pid_file`
      
      if (kill -0 $mysqlmanager_pid 2>/dev/null)
      then
        echo $echo_n "Shutting down MySQL"
        kill $mysqlmanager_pid
        # mysqlmanager should remove the pid_file when it exits, so wait for it.
        wait_for_pid removed "$mysqlmanager_pid"; return_value=$?
      else
        log_failure_msg "MySQL manager or server process #$mysqlmanager_pid is not running!"
        rm $pid_file
      fi
      
      # delete lock for RedHat / SuSE
      if test -f $lock_dir
      then
        rm -f $lock_dir
      fi
      exit $return_value
    else
      log_failure_msg "MySQL manager or server PID file could not be found!"
    fi
    ;;

  'restart')
    # Stop the service and regardless of whether it was
    # running or not, start it again.
    if $0 stop  $other_args; then
      $0 start $other_args
    else
      log_failure_msg "Failed to stop running server, so refusing to try to start."
      exit 1
    fi
    ;;

  'reload'|'force-reload')
    if test -s "$server_pid_file" ; then
      read mysqld_pid <  $server_pid_file
      kill -HUP $mysqld_pid && log_success_msg "Reloading service MySQL"
      touch $server_pid_file
    else
      log_failure_msg "MySQL PID file could not be found!"
      exit 1
    fi
    ;;
  'status')
    # First, check to see if pid file exists
    if test -s "$server_pid_file" ; then 
      read mysqld_pid < $server_pid_file
      if kill -0 $mysqld_pid 2>/dev/null ; then 
        log_success_msg "MySQL running ($mysqld_pid)"
        exit 0
      else
        log_failure_msg "MySQL is not running, but PID file exists"
        exit 1
      fi
    else
      # Try to find appropriate mysqld process
      mysqld_pid=`pidof $libexecdir/mysqld`
      if test -z $mysqld_pid ; then 
        if test "$use_mysqld_safe" = "0" ; then 
          lockfile=/var/lock/subsys/mysqlmanager
        else
          lockfile=/var/lock/subsys/mysqld-ib
        fi 
        if test -f $lockfile ; then 
          log_failure_msg "MySQL is not running, but lock exists"
          exit 2
        fi 
        log_failure_msg "MySQL is not running"
        exit 3
      else
        log_failure_msg "MySQL is running but PID file could not be found"
        exit 4
      fi
    fi
    ;;
    *)
      # usage
      echo "Usage: $0  {start|stop|restart|reload|force-reload|status}  [ MySQL server options ]"
      exit 1
    ;;
esac

exit 0
