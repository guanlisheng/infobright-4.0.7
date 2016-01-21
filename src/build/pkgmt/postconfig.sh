#!/bin/sh

#####################################
# user registration script is included only in ice.
#####################################
ibregis=`dirname $0`/support-files/ibregis
test -f "$ibregis" && . "$ibregis"
#####################################

my_conf="/etc/my-ib.cnf"
echo "Infobright post configuration"
echo "--------------------------------------"

NEW_DATADIR=
NEW_CACHEDIR=
NEW_PORT=
NEW_SOCKET=

OLD_DATADIR=
OLD_CACHEDIR=

status_datadir_changed=
status_cachedir_changed=

BH_INI="brighthouse.ini"
pid_file=

parse_arguments() 
{
  # We only need to pass arguments through to the server if we don't
  # handle them here.  So, we collect unrecognized options (passed on
  # the command line) into the args variable.
  pick_args=
  if test "$1" = PICK-ARGS-FROM-ARGV
  then
    pick_args=1
    shift
  fi

  for arg do
    case "$arg" in
      # these get passed explicitly to mysqld
      --basedir=*) MY_BASEDIR_VERSION=`echo "$arg" | sed -e "s;--basedir=;;"` ;;
      --datadir=*) DATADIR=`echo "$arg" | sed -e "s;--datadir=;;"` ;;
      --pid-file=*) pid_file=`echo "$arg" | sed -e "s;--pid-file=;;"` ;;
      --socket=*)  SOCKET=`echo "$arg" | sed -e "s;--socket=;;"` ;;
      --port=*)    PORT=`echo "$arg" | sed -e "s;--port=;;"` ;;
      --help)
        usage
        ;;
    esac
  done
}

error_withexit()
{
  echo "This script encountered an error: "$1
  if [ ! -z "$status_datadir_changed" ]; then
    echo "data directory($OLD_DATADIR) was copied to a new location ($NEW_DATADIR) but it will not be used due to the current failure."
  fi
  if [ ! -z "$status_cachedir_changed" ]; then
    echo "New cache directory($NEW_CACHEDIR) was created but it will not be used due to the current failure."
  fi
  if [ -f "$DATADIR/${BH_INI}.tmp" ]; then
    rm -f "$DATADIR/${BH_INI}.tmp"
  fi
  if [ -f "${my_conf}.tmp" ]; then
    rm -f "${my_conf}.tmp"
  fi
  exit 1; 
}

PREFIX=`pwd`
if test ! -f $PREFIX/bin/mysqld
then
  echo "Please run this script from inside the installed folder."
  echo "--------------------------------------"
  exit 1
fi

if test ! -w "/etc/passwd"
then
  echo "Error: You do not have sufficient access. Please try using root access."
  echo "--------------------------------------"
  exit 1
fi

# check if any job for installation are left. Complete here.
# Usually ICE is disabled and user must have to run this script.
# We enable it here.
# ========================================
if test -f /etc/my-ib.cnf.inactive
then
  mv -f /etc/my-ib.cnf.inactive /etc/my-ib.cnf
  echo "Infobright server activated."
  echo "--------------------------------------"
  if test -f "$ibregis"
  then
    UserRegistration
    echo
  fi
  exit 0
fi

echo "Using postconfig you can: "
echo "--------------------------------------"
echo "(1) Move existing data directory to other location,"
echo "(2) Move existing cache directory to other location,"
echo "(3) Configure server socket,"
echo "(4) Configure server port,"
echo "(5) Relocate datadir path to an existing data directory."
echo ""
echo "Please type 'y' for option that you want or press ctrl+c for exit."
echo ""

# Get first arguments from the my-ib.cnf file, groups [mysqld]
if test -x $PREFIX/bin/my_print_defaults
then
  print_defaults="$PREFIX/bin/my_print_defaults"
else
  echo "Error: conf file parser tool $PREFIX/bin/my_print_defaults was not found."
  exit 1
fi

parse_arguments `$print_defaults --defaults-file=$my_conf mysqld`
CACHEDIR=`grep "\(\s*CacheFolder\)" $DATADIR/$BH_INI|grep -v "\(^\s*#\)"|awk -F"=" '{print $2}'`
CACHEDIR=`echo $CACHEDIR | sed 's/^ *//;s/ *$//'`

if test -z "$DATADIR"
then
  echo "Error: datadir has empty value in the $my_conf file. It must be set properly."
  exit 1
fi

if test -z "$SOCKET"
then
  echo "Error: socket has empty value in the $my_conf file. It must be set properly."
  exit 1
fi

if test -z "$PORT"
then
  echo "Error: port has empty value in the $my_conf file. It must be set properly."
  exit 1
fi

if test -z "$CACHEDIR"
then
  echo "Error: CacheFolder has empty value in $DATADIR/$BH_INI file. It must be set properly."
  exit 1
fi

if test -z "$pid_file"
then
  pid_file=$DATADIR/`/bin/hostname`.pid
else
  case "$pid_file" in
    /* ) ;;
    * )  pid_file="$DATADIR/$pid_file" ;;
  esac
fi

# If there exists an old pid file, check if the daemon is already running
# Note: The switches to 'ps' may depend on your operating system
if test -f "$pid_file"
then
  PID=`cat $pid_file`
  echo "Error: A infobright mysqld instance (pid $PID) is running. Please shutdown the server and then run this postconfig."
  exit 1
fi

echo "Current configuration: "
echo
echo "--------------------------------------"
echo "Current config file: [$my_conf]"
echo "Current brighthouse.ini file: [$DATADIR/$BH_INI]"
echo "Current datadir: [$DATADIR]"
echo "Current CacheFolder in brighthouse.ini file: [$CACHEDIR]"
echo "Current socket: [$SOCKET] "
echo "Current port: [$PORT]"
echo "--------------------------------------"
echo

OLD_DATADIR=$DATADIR
OLD_CACHEDIR=$CACHEDIR
TR=tr
ECHO="echo -n"
AWK=awk

# solaris 10 default echo does not have -n option.
if [ ! -z "`uname -s | grep SunOS`" ]; then
  ECHO="echo"
  [ -x "/usr/xpg4/bin/tr" ] && TR=/usr/xpg4/bin/tr
  [ -x "/usr/ucb/echo"    ] && ECHO="/usr/ucb/echo -n"
  AWK=/usr/bin/nawk
fi

mod_DATADIR=
mod_CACHEDIR=
mod_SOCKET=
mod_PORT=
mod_RELOCATEDATADIR=

###########################################################
$ECHO "(1) Do you want to copy current datadir [$DATADIR] to a new location? [y/n]:"
read likey_input
likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
if [ "$likey_input" = "y" ]; then
  $ECHO "Give new datadir path (e.g. /opt/datadirnewpath/data):"
  read NEW_DATADIR
  [ -z "$NEW_DATADIR" ] && error_withexit "Error: empty path is given! Please give a valid path.";
  [ -d "$NEW_DATADIR" ] && error_withexit "Error: $NEW_DATADIR already exist! It must not exist.\nNew path will be created and data from current datadir will be moved to new path.";
  [ "$NEW_DATADIR" != "$DATADIR" ] && mod_DATADIR="1"
fi

###########################################################
likey_input="n"
if [ ! -z "$mod_DATADIR" ]; then
  echo "(2) Option to change CacheFolder is disabled when option 1 is chosen!";
else
  $ECHO "(2) Do you want to move current CacheFolder [$CACHEDIR] to a new location? [y/n]:"
  read likey_input
  likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
fi
if [ "$likey_input" = "y" ]; then
  $ECHO "Give new CacheFolder path:"
  read NEW_CACHEDIR

  [ -z "$NEW_CACHEDIR" ] && error_withexit "Error: empty path is given! Please give a valid path.";
  [ -d "$NEW_CACHEDIR" ] && error_withexit "Error: $NEW_CACHEDIR already exist!  It must not exist. This script will create the folder.";
  grep_silent="grep -q"
  test "x`uname -s | grep SunOS`" != "x" && grep_silent="grep"

  isparent=
  ischild=

  cdir=`dirname $NEW_CACHEDIR`
  cdir=`dirname $cdir`
  test "$cdir" != "/" && cdir=$cdir/
  cdir=$cdir`basename $NEW_CACHEDIR`

  # verify that cacedir is not a child or parent of data dir.
  active_datadir=$NEW_DATADIR
  test -z "$active_datadir" && active_datadir=$OLD_DATADIR
  test -z "$active_datadir" && echo "Error: invalid path found for either old data folder or new data folder." && exit 1
  test "$NEW_CACHEDIR" = "$active_datadir" && echo "Error: cache folder and data folder are same." && exit 1

  ddir=$active_datadir
  ddir=`dirname $ddir`
  test "$ddir" != "/" && ddir=$ddir/
  ddir=$ddir`basename $active_datadir`

  echo $cdir|$grep_silent -i "$ddir/" 2>/dev/null && isparent="yes"
  echo $ddir|$grep_silent -i "$cdir/" 2>/dev/null && ischild="yes"

  if test ! -z "$isparent" -o ! -z "$ischild"  -o \
    "$cdir" = "$ddir" -o "$cdir" = "/tmp"
  then
    echo "Error: Invalid cache folder input. It can not be datadir, /tmp folder or child/parent folder of data dir."
    exit 1
  fi
  [ "$NEW_CACHEDIR" != "$CACHEDIR" ] && mod_CACHEDIR="1"
fi

###########################################################
$ECHO "(3) Do you want to change current socket [$SOCKET]? [y/n]:"
read likey_input
likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
if [ "$likey_input" = "y" ]; then
  $ECHO "Give new socket:"
  read NEW_SOCKET
  [ -z "$NEW_SOCKET" ] && error_withexit "Error: new socket is empty. Please give a valid socket.";
  [ "$NEW_SOCKET" != "$SOCKET" ] && mod_SOCKET="1"
fi

###########################################################
$ECHO "(4) Do you want to change current port [$PORT]? [y/n]:"
read likey_input
likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
if [ "$likey_input" = "y" ]; then
  $ECHO "Give new port:"
  read NEW_PORT
  [ -z "$NEW_PORT"   ] && error_withexit "Error: new port is empty. Please give a valid port.";
  isnumber=`echo $NEW_PORT | sed 's/^[0-9]*$//'`
  [ ! -z "$isnumber" ] && error_withexit "Error: new port $NEW_PORT is not a number. Please give a valid port.";
  [ "$NEW_PORT" != "$PORT" ] && mod_PORT="1"
fi

###########################################################
if [ -z "$mod_DATADIR" -a \
  -z "$mod_CACHEDIR" -a \
  -z "$mod_PORT" -a -z "$mod_SOCKET" ];
then
  EXISTING_DATADIR=""
  $ECHO "(5) Do you want to relocate to an existing datadir? Current datadir is [$DATADIR]. [y/n]:"
  read likey_input
  likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
  if [ "$likey_input" = "y" ]; then
    $ECHO "Give an existing datadir path:"
    read EXISTING_DATADIR
    EXISTING_DATADIR=`echo $EXISTING_DATADIR | sed 's/^ *//;s/ *$//'`
    [  -z "$EXISTING_DATADIR"   ]        && error_withexit "Empty path given! Please give a valid path.";
    [ ! -d "$EXISTING_DATADIR" ]         && error_withexit "$EXISTING_DATADIR does not exist! Invalid path.";
    [ ! -d "$EXISTING_DATADIR/mysql" ]   && error_withexit "Invalid data directory.";
    [ "$EXISTING_DATADIR" = "$DATADIR" ] && error_withexit "Current data dir and given data are same. No relocation required.";

    olddatadiruser=`ls -ld $DATADIR |$AWK '{print $3}'`
    olddatadirgroup=`ls -ld $DATADIR|$AWK '{print $4}'`

    newdatadiruser=`ls -ld $EXISTING_DATADIR | $AWK '{print $3}'`
    newdatadirgroup=`ls -ld $EXISTING_DATADIR| $AWK '{print $4}'`

    [ "$olddatadiruser" != "$newdatadiruser"   ] && echo "!!!Warning new datdir is not owned by user $olddatadiruser."
    [ "$olddatadirgroup" != "$newdatadirgroup" ] && echo "!!!Warning new datdir is not owned by group $olddatadirgroup."

    mod_RELOCATEDATADIR="1"
  fi
else
  echo "(5) Relocation is disabled when options 1-4 are chosen!";
fi

echo
echo "--------------------------------------"

###########################################################
#  Some messages about user choice.
###########################################################
[ ! -z "$mod_DATADIR"  ] && echo "Datadir($DATADIR) is going to be copied to "$NEW_DATADIR
[ ! -z "$mod_CACHEDIR" ] && echo "New CacheFolder is going to be "$NEW_CACHEDIR
[ ! -z "$mod_SOCKET"   ] && echo "New socket is going to be "$NEW_SOCKET
[ ! -z "$mod_PORT"     ] && echo "New port is going to be "$NEW_PORT
[ ! -z "$mod_RELOCATEDATADIR" ] && echo "datadir is going to be relocated to "$EXISTING_DATADIR

#keep the configuration, if we do not want to change it
[ -z "$mod_DATADIR"  ] && NEW_DATADIR=$DATADIR
[ -z "$mod_SOCKET"   ] && NEW_SOCKET=$SOCKET
[ -z "$mod_PORT"     ] && NEW_PORT=$PORT

###########################################################

echo "--------------------------------------"
if test -z "$mod_DATADIR" -a \
  -z "$mod_CACHEDIR" -a \
  -z "$mod_PORT" -a -z "$mod_SOCKET" -a \
  -z "$mod_RELOCATEDATADIR"
then
  echo "No changes has been made."
  echo "--------------------------------------"
  exit 1
else
  echo
  $ECHO "Please confirm to proceed? [y/n]:"
  read likey_input
  likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
  if [ "$likey_input" != "y" ]; then
    echo "No changes has been made."
    echo "--------------------------------------"
    exit 1
  fi
fi 

###########################################################
#  APPLY CHANGES                                          #
###########################################################
if test ! -z "$mod_DATADIR" -o \
  ! -z "$mod_PORT" -o ! -z "$mod_SOCKET"
then
  NEW_DATADIR_temp=`echo $NEW_DATADIR | sed -e 's/\//\\\\\//g'`
  DATADIR_temp=`echo $DATADIR | sed -e 's/\//\\\\\//g'`
  NEW_SOCKET_temp=`echo $NEW_SOCKET | sed -e 's/\//\\\\\//g'`
  SOCKET_temp=`echo $SOCKET | sed -e 's/\//\\\\\//g'`

  conf_change="sed \
-e 's/$DATADIR_temp/$NEW_DATADIR_temp/' \
-e 's/$PORT/$NEW_PORT/' \
-e 's/$SOCKET_temp/$NEW_SOCKET_temp/' \
	< $my_conf > $my_conf.tmp"
  eval $conf_change
  if test $? != 0
  then
    echo "Error during updating $my_conf file.";  
    exit 1
  fi
fi

if test ! -z "$mod_DATADIR"
then
  $ECHO "Copying $DATADIR to $NEW_DATADIR ..."
  mkdir -p "$NEW_DATADIR"
  if [ $? != 0 ]; then
    error_withexit "Error creating new data folder $NEW_DATADIR.";
  fi
  cp -prf $DATADIR/* $NEW_DATADIR/
  if [ $? != 0 ]; then
    [ -f $my_conf.tmp ] && rm -f $my_conf.tmp
    error_withexit "Error during copying $DATADIR to $NEW_DATADIR.";
  fi
  user=`ls -ld $DATADIR |$AWK '{print $3}'`
  group=`ls -ld $DATADIR|$AWK '{print $4}'`
  DATADIR=$NEW_DATADIR
  chown -R $user:$group $NEW_DATADIR
  echo "is done."
  status_datadir_changed="true"
fi

if test ! -z "$mod_CACHEDIR"
then
  # create the new cache folder.
  $ECHO "Creating new cachedir $NEW_CACHEDIR ..."
  mkdir -p $NEW_CACHEDIR 2>/dev/null
  if [ $? != 0 ]; then
    error_withexit "Error creating new cachedir $NEW_CACHEDIR.";
  fi
  # updating brightbous.ini file with new cachefolder
  CACHEDIR_temp=`echo $CACHEDIR | sed -e 's/\//\\\\\//g'`
  NEW_CACHEDIR_temp=`echo $NEW_CACHEDIR | sed -e 's/\//\\\\\//g'`
  conf_change="sed \
-e 's/^[ ]*CacheFolder[ ]*=.*/CacheFolder = $NEW_CACHEDIR_temp/g' 
< $DATADIR/$BH_INI > $DATADIR/$BH_INI.tmp"
  eval $conf_change
  [ $? != 0 ] && error_withexit "Error during updating $DATADIR/$BH_INI file.";
  user=`ls -ld $CACHEDIR |$AWK '{print $3}'`
  group=`ls -ld $CACHEDIR|$AWK '{print $4}'`
  CACHEDIR=$NEW_CACHEDIR
  chown -R $user:$group $NEW_CACHEDIR
  echo "is done."
  status_cachedir_changed="true"
fi

if [ ! -z "$mod_DATADIR" -o ! -z "$mod_PORT" -o ! -z "$mod_SOCKET" ];
then
  if [ -f $my_conf.tmp ]; then
    mv -f $my_conf.tmp $my_conf
    if [ $? != 0 ]; then
      error_withexit "Error moving $my_conf.tmp to $my_conf.";
    fi
  fi
fi
[ ! -z "$mod_CACHEDIR" -a -f $DATADIR/$BH_INI.tmp ] && mv -f $DATADIR/$BH_INI.tmp $DATADIR/$BH_INI
[ ! -z "$mod_DATADIR"  ] && $ECHO "You can now remove/backup your old $OLD_DATADIR ..."
[ ! -z "$mod_CACHEDIR" ] && rm -rf $OLD_CACHEDIR

if [ ! -z "$mod_RELOCATEDATADIR" ]; then
  $ECHO "Relocating Infobright datadir to $EXISTING_DATADIR ..."
  DATADIR_temp=`echo $DATADIR | sed -e 's/\//\\\\\//g'`
  EXISTING_DATADIR_TEMP=`echo $EXISTING_DATADIR | sed -e 's/\//\\\\\//g'`
  conf_change="sed \
-e 's/$DATADIR_temp/$EXISTING_DATADIR_TEMP/' \
< $my_conf > $my_conf.upgrade"
  eval $conf_change
  [ $? != 0 ] && error_withexit "Error, during updating $my_conf file."
  mv $my_conf.upgrade $my_conf
  echo "$my_conf has been configured to use the new data dir $EXISTING_DATADIR."
  echo "Please make sure $EXISTING_DATADIR has proper permission set for user 'mysql' or"
  echo "other user who runs infobright. You may verify $my_conf file before starting infobright."
  echo ""
  
	# Verify that CACHEDIR exists or else mysqld will fail to start
	if [ ! -d "$CACHEDIR" ]; then
	  echo "Cachedir $CACHEDIR does not exist, would you like to create it? [y/n]: "
	  read likey_input
	  likey_input=`echo $likey_input | $TR '[:upper:]' '[:lower:]'`
	  if [ "$likey_input" = "y" ]; then
	  [ -z "$CACHEDIR" ] && error_withexit "Error: empty path is given! Please give a valid path.";
	  [ -d "$CACHEDIR" ] && error_withexit "Error: $CACHEDIR already exist!  It must not exist.";
	  grep_silent="grep -q"
	  test "x`uname -s | grep SunOS`" != "x" && grep_silent="grep"

	  isparent=
	  ischild=

	  cdir=`dirname $CACHEDIR`
	  cdir=`dirname $cdir`
	  test "$cdir" != "/" && cdir=$cdir/
	  cdir=$cdir`basename $CACHEDIR`

	  # verify that cacedir is not a child or parent of data dir.
	  active_datadir=$DATADIR
	  test -z "$active_datadir" && active_datadir=$OLD_DATADIR
	  test -z "$active_datadir" && echo "Error: invalid path found for either old data folder or new data folder." && exit 1
	  test "$CACHEDIR" = "$active_datadir" && echo "Error: cache folder and data folder are same." && exit 1

	  ddir=$active_datadir
	  ddir=`dirname $ddir`
	  test "$ddir" != "/" && ddir=$ddir/
	  ddir=$ddir`basename $active_datadir`

	  echo $cdir|$grep_silent -i "$ddir/" 2>/dev/null && isparent="yes"
	  echo $ddir|$grep_silent -i "$cdir/" 2>/dev/null && ischild="yes"

	  if test ! -z "$isparent" -o ! -z "$ischild"  -o \
		"$cdir" = "$ddir" -o "$cdir" = "/tmp"
	  then
		echo "Error: Invalid cache folder input. It can not be datadir, /tmp folder or child/parent folder of data dir."
		exit 1
	  fi
	  mkdir -p $CACHEDIR 2>/dev/null
	  if [ $? != 0 ]; then
		error_withexit "Error creating new cachedir $CACHEDIR."
	  fi
	fi
  fi
fi

echo ""
echo "Done!"
