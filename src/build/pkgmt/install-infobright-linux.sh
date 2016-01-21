#!/bin/bash
# This script installs Infobright server

#####################################
# user registration script is included only in ice.
#####################################
ibregis=$(dirname $0)/support-files/ibregis
[ -f "$ibregis" ] && source "$ibregis"
#####################################

# Default values
PREFIX=`pwd`
bh_datadir=$PREFIX/data
bh_cachedir=$PREFIX/cache
my_port="5029"

ibuseradd="/sbin/useradd"
ibusermod="/sbin/usermod"
ibuserdel="/sbin/userdel"
ibgroupadd="/sbin/groupadd"

[ ! -f "$ibuseradd" ] && ibuseradd="/usr/sbin/useradd"
[ ! -f "$ibusermod" ] && ibusermod="/usr/sbin/usermod"
[ ! -f "$ibuserdel" ] && ibuserdel="/usr/sbin/userdel"
[ ! -f "$ibgroupadd" ] && ibgroupadd="/usr/sbin/groupadd"

# We keep two additional defaults files (master and slave)
my_conf="/etc/my-ib.cnf"
my_conf_in="support-files/my-ib.cnf.in"
my_master_conf="/etc/my-ib-master.cnf"
my_master_conf_in="support-files/my-ib-master.cnf.in"
my_slave_conf="/etc/my-ib-slave.cnf"
my_slave_conf_in="support-files/my-ib-slave.cnf.in"
my_sock="/tmp/mysql-ib.sock"

user=mysql
group=""

daemon_path=/etc/init.d/
daemon_prefix=mysqld-ib

client_script=$PREFIX/mysql-ib
[ -d /usr/bin ] && client_script=/usr/bin/mysql-ib

LIBPATH=/usr/lib
MACHINE_ARC=`uname -m`
if [ "$MACHINE_ARC"="x86_64" ]; then
  test -d /usr/lib64 && LIBPATH=/usr/lib64
fi
libclient=libmysqlibclient.so.16

have_cachedir="false"
have_datadir="false"
have_symlink="false"
have_conf="false"
have_master_conf="false"
have_slave_conf="false"
have_user_created="false"

upgrade=""
cur_basedir=""

is_static_build()
{
  mysqld_path=$1
  ldd $mysqld_path | grep -v -q ".so" 2>/dev/null
  return $?
}

exit_with_error()
{
   echo $1
   exit 1
}

get_glibc_minor()
{
  glibc_version_minor="0"
  GLIBCPATH=/lib
  [ "`uname -m`"="x86_64" ] && GLIBCPATH=/lib64
  glibc_version=`strings $GLIBCPATH/libc.so.6 | grep GLIBC_ | grep -v PRIVATE | awk -F"GLIBC_" '{print $2}' | tail -1 2> /dev/null`
  if [ ! -z "glibc_version" ] ; then
    glibc_version_minor=`echo "$glibc_version" | awk -F"." '{print $2}'`
  fi
  minor=`expr $glibc_version_minor`
  return $minor
}

check_write_permission()
{
  on_path=$1
  [ ! -w "$on_path" ] && return 1;
  return 0;
}

check_glibc_compatibility()
{
  mysqld_path=$1
  get_glibc_minor
  glibc_minor=$?
  is_static_build $mysqld_path
  if [ $? -eq 0 ] ;
  then
    # We compile and build our static build package using glibc 2.9
    ib_recommended_glibc_minor=9
    if [ $glibc_minor -gt $ib_recommended_glibc_minor ] ;
    then
      is_nscd_running=`ps aux | grep nscd | grep -v "grep" 2>/dev/null`
      [ -z "$is_nscd_running" ] && return 1;
    fi
  else    
    ib_min_glibc_minor=5
    if [ $glibc_minor -lt $ib_min_glibc_minor ] ;
    then
      echo "Warning: current glibc is older than the recommended glibc version. Current installation may fail or product may not function properly."
    fi
  fi  
  return 0;
} 

check_system_conf()
{
  matched="true"
  exp_processor="Intel/AMD"
  exp_machine="x86_64/i*86/86"
  exp_OS="GNU/Linux"
  exp_distri="Red Hat/Fedora/CentOS/Ubuntu/Debian/SuSe"

  if test -f /etc/redhat-release ; then
    OS_conf="/etc/redhat-release"
  elif test -f /etc/lsb-release ; then
    OS_conf="/etc/lsb-release"
  elif test -f /etc/SuSe-release ; then
    OS_conf="/etc/SuSe-release"
  elif test -f /etc/issue ; then
    OS_conf="/etc/issue"
  fi

  # processor and architecture
  cpuinfo=/proc/cpuinfo
  cur_processor=`cat $cpuinfo | grep 'model name' | cut -d ':' -f 2 | head -1`
  if test -z "$cur_processor" ; then
    cur_processor=`cat $cpuinfo | grep 'cpu model' | cut -d ':' -f 2 | head -1`
  fi
  # fallback: get CPU model from uname output
  if test -z "$cur_processor" ; then
    cur_processor=`uname -m`
  fi
  cur_machine=`uname -m`
  case "$cur_machine" in
    x86_64*)
      ;;
    i686*)
      ;;
    x86*)
      ;;
   # else unknown
    *)
     cur_machine=""
      ;;
  esac

  # prevent trying of 64 bit binaries on a 32 but machine
  if [ "$cur_machine" != "x86_64" ]
  then
    [ -f $PREFIX/bin/mysqld ]     && mysqld_path=$PREFIX/bin/mysqld
    [ -f $PREFIX/libexec/mysqld ] && mysqld_path=$PREFIX/libexec/mysqld

    if [ -f /usr/bin/file  -a ! -z "$mysqld_path" ]
    then
      /usr/bin/file $mysqld_path | grep -q "x86-64" && ARCH_MISMATCHED="yes"
      if [ "$ARCH_MISMATCHED" = "yes" ]; then
        echo "Error: Machine architecture did not match. Expecting 64 bit cpu.";
        return 1;
      fi
    fi
  fi
  
  # OS and distro
  cur_OS=`uname -o|grep $exp_OS`
  if [ -f $OS_conf ]; then
    cur_distri=`cat $OS_conf|grep "Red Hat"`
    cur_distri=${cur_distri}`cat $OS_conf|grep "CentOS"`
    cur_distri=${cur_distri}`cat $OS_conf|grep "Fedora"`
    cur_distri=${cur_distri}`cat $OS_conf|grep "Ubuntu"`
    cur_distri=${cur_distri}`cat $OS_conf|grep "Debian"`
    cur_distri=${cur_distri}`cat $OS_conf|grep "SuSe"`
  else
    matched="false"
  fi

  if [ -z "$cur_processor" -o -z "$cur_machine" -o \
     -z "$cur_OS" -o -z "$cur_distri" ]
  then
    matched="false"
  fi

  if [ ! -z "$cur_processor" -a ! -z "$cur_machine" -a \
     ! -z "$cur_OS" -a -z "$cur_distri" ]
  then
    # all matched except distro, so let it go and give a warning
    echo "Warning: Expected Linux distribution did not match."
    return 0;
  fi

  [ -z "$cur_processor" ] && cur_processor="---"
  [ -z "$cur_machine"   ] && cur_machine="---"
  [ -z "$cur_OS"        ] && cur_OS="---"
  [ -z "$cur_distri"    ] && cur_distri="---"

  if [ "$matched" = "false" ]; then
    echo "Warning: Expected system configuration did not match!"
    echo "Expected : "$exp_processor:$exp_machine:$exp_OS:$exp_distri"."
    echo "Found   :"$cur_processor:$cur_machine:$cur_OS:$cur_distri"."
    return 1
  fi
  
  ### check static build compatibility
  #############################################
  mysqld_path=$PREFIX/bin/mysqld
  [ ! -f $mysqld_path ] && mysqld_path=$PREFIX/libexec/mysqld
  
  check_glibc_compatibility $mysqld_path
  if [ $? -ne 0 ];
  then
    echo "Warning: Infobright current package may require nscd service running prior to the installation."
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

usage()
{
  echo -e "Parameters required for installation -\n\
    --datadir=infobright data folder       [--datadir=$bh_datadir]\n\
    --cachedir=infobright cache folder     [--cachedir=$bh_cachedir]\n\
    --config=mysql conf file to be created [--config=$my_conf]\n\
    --port=infobright server port          [--port=$my_port]\n\
    --socket=socket file to be used by this server [--socket=$my_sock]\n\
    --user=user to be created if not exist         [--user=$user]\n\
    --group=user group be created if not exist     [--group=`test -z "$group" && echo $user || echo $group`]"
  echo -e "\nParameters required for upgrade -\n\
    --upgrade\n\
    --config=Existing mysql conf file to be used [--config=$my_conf]"
  echo -e "Example command - \n\
    ./install-infobright.sh --help\n\
    ./install-infobright.sh          (To install using defaults).\n\
    ./install-infobright.sh --datadir=$bh_datadir \
--cachedir=$bh_cachedir --port=$my_port \
--config=$my_conf --socket=$my_sock --user=$user --group=`test -z "$group" && echo $user || echo $group`\n\
    ./install-infobright.sh --upgrade --config=$my_conf  (To upgrade existing server, conf file $my_conf).\n"
  exit 1
}

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
      --basedir=*) cur_basedir=`echo "$arg" | sed -e "s;--basedir=;;"` ;;
      --datadir=*) cur_datadir=`echo "$arg" | sed -e "s;--datadir=;;"` ;;
      #--socket=*)  SOCKET=`echo "$arg" | sed -e "s;--socket=;;"` ;;
      #--port=*)    PORT=`echo "$arg" | sed -e "s;--port=;;"` ;;
      --help)
        usage
        ;;
    esac
  done
}

upgrade_brighthouseini_file()
{
  datadir=$1
  # These parameters have been removed from brighthouse.ini since 3.4.0
  # So make sure that they get removed otherwise server would not start.
  sed \
  -e 's/^\s*ServerCompressedHeapSize.*//gI' \
  -e 's/^\s*HugefileDir.*//gI' \
  -e 's/^\s*BufferingLevel.*//gI' \
  -e 's/^\s*CachingLevel.*//gI' \
  -e 's/^\s*LoaderSaveThreadNumber.*//gI' \
  -e 's/^\s*PrefetchThreads.*//gI' \
  -e 's/^\s*PrefetchQueueLength.*//gI' \
  -e 's/^\s*PrefetchDepth.*//gI' \
  -e 's/^\s*ClusterSize.*//gI' \
  -e 's/^\s*CachingLevel.*//gI' \
  -e 's/^\s*BufferingLevel.*//gI' \
  < $datadir/brighthouse.ini > $datadir/brighthouse.ini.tmp
  mv -f $datadir/brighthouse.ini.tmp $datadir/brighthouse.ini
}

upgrade_my_conf()
{
  issolaris=
  [ ! -z "`uname -a|egrep -i -e SunOS`" -o ! -z "`uname -a|egrep -i -e solaris`" ] && issolaris="yes"
  
  conf=$1
  
  if [ ! -z "$issolaris" ]; then
    sed -e 's/^ *query_cache_size *= *.*//g' -e 's/^ *query_cache_type *= *.*//g' < ${conf} > ${conf}.tmp
  else
    sed -e 's/^ *query_cache_size *= *.*//g' -e 's/^ *query_cache_type *= *.*//gI' < ${conf} > ${conf}.tmp
  fi
  [ -f "${conf}.tmp" ] && mv -f ${conf}.tmp ${conf}
  
  if [ ! -z "$issolaris" ]; then
    sed -e 's/^ *\[ *mysqld *\] */[mysqld] \
# query cache must be turned off for 3.5.2 release \
query_cache_type = 0 \
/g' ${conf}  > ${conf}.tmp 
  else
    sed -e 's/^ *\[ *mysqld *\] */[mysqld] \
# query cache must be turned off for Infobright 3.5.2 release \
query_cache_type = 0 \
/gI' ${conf}  > ${conf}.tmp
  fi
  [ -f "${conf}.tmp" ] && mv -f ${conf}.tmp ${conf}
}

upgrade_server()
{
  echo "Upgrading infobright server..."

  AWK=awk
  test ! -z "`uname -a|egrep -i -e SunOS`" -o ! -z "`uname -a|egrep -i -e solaris`" && AWK=/usr/bin/nawk

  # parse conf file
  #########################################

  if test -x $PREFIX/bin/my_print_defaults
  then
    print_defaults="$PREFIX/bin/my_print_defaults"
  else
    exit_with_error "Error: conf file parser tool $PREFIX/bin/my_print_defaults was not found.\
Upgrade terminated."
  fi
  parse_arguments `$print_defaults --defaults-file=$my_conf mysqld`

  # checking basedir, mysqld versions old, new etc
  #########################################

  test ! -d $cur_basedir && exit_with_error "Error: either $cur_basedir does not exist or $my_conf \
does not have correct basedir path. Upgrade terminated."

  cur_mysql_path=$cur_basedir/bin/mysqld
  test ! -f $cur_mysql_path && cur_mysql_path=$cur_basedir/libexec/mysqld
  test ! -f $cur_mysql_path && exit_with_error "Error: mysqld does not exist inside $cur_basedir. \
Upgrade terminated."

  cur_mysql_version=`$cur_mysql_path --no-defaults --version | $AWK -F"Ver " '{print $2}' | $AWK -F" " '{print $1}' | $AWK -F"-" '{print $1}'`
  cur_mysql_version_short=`echo $cur_mysql_version| $AWK -F"." '{print $3}'`

  test -z "$cur_mysql_version" -o -z "$cur_mysql_version_short" && exit_with_error "Error: Could not detect version \
of existing installation. Upgrade terminated."

  new_mysql_path=$PREFIX/bin/mysqld
  test ! -f $new_mysql_path && new_mysql_path=$PREFIX/libexec/mysqld
  test ! -f $new_mysql_path && exit_with_error "Error: mysqld does not exist inside $PREFIX. \
Upgrade terminated."

  new_mysql_version=`$new_mysql_path --no-defaults --version | $AWK -F"Ver " '{print $2}' | $AWK -F" " '{print $1}' | $AWK -F"-" '{print $1}'`
  new_mysql_version_short=`echo $new_mysql_version | $AWK -F"." '{print $3}'`

  test -z "$new_mysql_version" -o -z "$new_mysql_version_short" && exit_with_error "Error: Could not detect version of current package.
Upgrade terminated."

  if test $new_mysql_version_short -lt $cur_mysql_version_short;
  then
    exit_with_error "Error: Downgrade ($new_mysql_version_short -> $cur_mysql_version) is not possible."
  fi

  echo "Updating folder $cur_basedir..."
  echo "Copying files from $PREFIX to $cur_basedir..."
  cp -rf $PREFIX/bin \
    $PREFIX/scripts \
    $PREFIX/include \
    $PREFIX/mysql-test \
    $PREFIX/share \
    $PREFIX/support-files \
    $PREFIX/README \
    $PREFIX/install-infobright.sh \
    $PREFIX/lib \
    $cur_basedir/
  test $? != 0 && exit_with_error "Copy failed. Upgrade incompleted."
  test -d $PREFIX/libexec && cp -rf $PREFIX/libexec $cur_basedir/
  test -d $PREFIX/tools && cp -rf $PREFIX/tools $cur_basedir/

  create_symlink_libclient $cur_basedir

  if test "$cur_mysql_version" = "5.1.14";
  then
    echo "Updating $my_conf..."
    sed -e 's/\(^\s*log\s*=\)/general_log=1\ngeneral_log_file=/' \
-e 's/\(^\s*log_slow_queries\s*=\)/slow_query_log=1\nslow_query_log_file=/' \
 < $my_conf > $my_conf.tmp
    mv -f $my_conf.tmp $my_conf
  fi

  upgrade_brighthouseini_file $cur_datadir

  if test ! -d $cur_datadir/sys_infobright;
  then
    cp -rf $PREFIX/support-files/data/sys_infobright $cur_datadir/
    chown -R mysql:mysql $cur_datadir/sys_infobright
  fi

  # Create advanced configuration.
  version_comment=`$cur_basedir/bin/mysqld --no-defaults --version 2>/dev/null`
  ice=`echo $version_comment |grep ice 2>/dev/null`
  test -z "$ice" -a -f $PREFIX/confman.sh && cp -f $PREFIX/confman.sh $cur_basedir/
 (test -z "$ice" -a -f $cur_basedir/confman.sh && $cur_basedir/confman.sh --defaults-file=$my_conf --basedir=$cur_basedir --autoconfigure) > /dev/null 

  upgrade_my_conf $my_conf

  echo "INFOBRIGHT UPGRADE COMPLETED."
  if test $new_mysql_version_short -gt $cur_mysql_version_short;
  then
    echo "Please remember to run mysql_upgrade. See User guide for instructions."
  fi

  exit 0;
}

verify_args()
{
  if test ! -z "$upgrade"; then
    test   -z "$my_conf" && exit_with_error "Error: missing parameter --config! To upgrade, \
conf file path of existing installation must be given. Upgrade terminated."
    test ! -f "$my_conf" && exit_with_error "Error: $my_conf does not exist! To upgrade, \
a valid conf file path of existing installation must be given. Upgrade terminated."
    return 0;
  fi

  # check if there is any escape characters. We do not support escape character
  # as it may break the installation in some other places.
  bh_datadir=`echo $bh_datadir|sed -e 's/^\s*//' -e 's/\s*$//'`
  bh_cachedir=`echo $bh_cachedir|sed -e 's/^\s*//' -e 's/\s*$//'`
  my_conf=`echo $my_conf|sed -e 's/^\s*//' -e 's/\s*$//'`
  my_sock=`echo $my_sock|sed -e 's/^\s*//' -e 's/\s*$//'`
  escape_in_1=`echo $bh_datadir | grep  "[\\\'\"\ \*\(\)&^#$%,?;|=@]"`;
  escape_in_2=`echo $bh_cachedir | grep  "[\\\'\"\ \*\(\)&^#$%,?;|=@]"`;
  escape_in_3=`echo $my_conf | grep  "[\\\'\"\ \*\(\)&^#$%,?;|=@]"`;
  escape_in_4=`echo $my_sock | grep  "[\\\'\"\ \*\(\)&^#$%,?;|=@]"`;
  escape_in_5=`echo $user | grep  "[\\\'\"\ \*\(\)&^#$%,?;|=@]"`;
  if test ! -z "$escape_in_1" -o ! -z "$escape_in_2" -o ! -z "$escape_in_3" -o ! -z "$escape_in_4" -o ! -z "$escape_in_5" ;
  then 
    exit_with_error "Error: One or more parameters contain escape character(space,@,*&% etc.). Please provide input parameters without escape character!";
  fi

  if test "$bh_datadir" = "";  then echo "Error: datadir can not be empty!";   return 1; fi
  if test "$bh_cachedir" = ""; then echo "Error: cachedir can not be empty!";  return 1; fi
  if test "$my_conf" = "";     then echo "Error: mysql conf path can not be empty!"; return 1; fi
  if test "$my_port" = "";     then echo "Error: port is missing!";            return 1; fi
  if test "$my_sock" = "";     then echo "Error: socket file path is missing!";return 1; fi
  if test "$user" = "" ;       then echo "Error: user can not be empty!";      return 1; fi  

  grep_silent="grep -q"
  test "x`uname -s | grep SunOS`" != "x" && grep_silent="grep"

  isparent=
  ischild=

  cdir=`dirname $bh_cachedir`
  cdir=`dirname $cdir`
  test "$cdir" != "/" && cdir=$cdir/
  cdir=$cdir`basename $bh_cachedir`

  ddir=$bh_datadir
  ddir=`dirname $ddir`
  test "$ddir" != "/" && ddir=$ddir/
  ddir=$ddir`basename $bh_datadir`

  echo $cdir|$grep_silent -i "$ddir/" 2>/dev/null && isparent="yes"
  echo $ddir|$grep_silent -i "$cdir/" 2>/dev/null && ischild="yes"

  if test ! -z "$isparent" -o ! -z "$ischild"  -o \
    "$cdir" = "$ddir" -o "$cdir" = "/tmp"
  then
    echo "Error: Invalid cache folder input. It can not be datadir, /tmp folder or child/parent folder of data dir."
    return 1;
  fi
  return 0
}

paths_exist()
{
  # check if datafolder, cache folder and my-ib.cnf file exist
  if [ "$bh_datadir" != "$PREFIX/data" ]; then
    if test -d $bh_datadir;
    then 
      echo "Error: datadir $bh_datadir already exists!"; 
      echo "Please give a different data directory path. It will be created by this installer.";
      return 1; 
    fi
  fi
  if [ "$bh_cachedir" != "$PREFIX/cache" ]; then
    if test -d $bh_cachedir;
    then 
      echo "Error: cachedir path $bh_cachedir already exists!"; 
      echo "Please give a different cache folder path. It will be created by this installer.";
      return 1; 
    fi
  fi
  if test -f $my_conf; 
  then 
    echo "Error: Infobright conf file $my_conf already exists!"; 
    echo "Please give a different conf file name or path. It will be created and configured by this installer.";
    return 1; 
  fi
  return 0
}

undo_with_exit()
{
  echo "Rolling back the installation due to unexpected failure!"
  
  if [ "$bh_datadir" != "$PREFIX/data" ]; then
    [ "$have_datadir" = "true" ] && rm -rf $bh_datadir
  fi
  test "$have_cachedir" = "true" && rm -rf $bh_cachedir
  test "$have_symlink" = "true" && rm -f $LIBPATH/$libclient
  test "$have_conf" = "true" && rm -f $my_conf
  test "$have_master_conf" = "true" && rm -f $my_master_conf
  test "$have_slave_conf" = "true" && rm -f $my_slave_conf
  test "$have_user_created" = "true" && $ibuserdel -f $user

  echo -e "Installation failed! Please investigate the error messages."
  exit 1
}

create_symlink_libclient()
{
  cur_prefix=$1
  srclibpath=$cur_prefix/lib/mysql
  test ! -f $srclibpath/$libclient && srclibpath=$cur_prefix/lib
  if [ ! -f $LIBPATH/$libclient -a -f $srclibpath/$libclient ]; then
    check_write_permission $LIBPATH/
    if [ $? = 0 ]; 
    then
      ln -sf $srclibpath/$libclient $LIBPATH/$libclient
      if [ $? != 0 ]; then
        echo "Warning: The installer failed creating symlink $LIBPATH/$libclient to $srclibpath/$libclient."
        echo "You may need to set LD_LIBRARY_PATH to folder $srclibpath to run infobright mysql client."
        return 1;
      fi
      have_symlink="true"
    else
      echo "<<Message>> You do not have permission to create symlink $LIBPATH/$libclient to $srclibpath/$libclient."
      echo "You may need to set LD_LIBRARY_PATH to folder $srclibpath to run infobright mysql client."
      return 1;
    fi
  fi
  return 0
}

# Display infobright license
license_agreement()
{
  bh_license=$1
  likey_input='s'

  if [ "$1" = "" ] || [ ! -e $1 ]; then
    echo "Infobright internal error. Invalid license file!"
    echo "License file: '$1' does not exist."
    return 1
  fi
  if [ ! -e /usr/bin/less ]; then
     echo "Error: required system editor tool 'Less' does not exits in /usr/bin."
     echo "Installation terminated!"
     exit 1
  fi

  echo -e "Infobright license agreement...\n\
System tool 'Less' - a text file viewer will be used to display license agreement.\n\
Please only use up/down arrow keys for scrolling license text and press Q when finished reading."
  
  while [ "$likey_input" = "r" ] || [ "$likey_input" = "s" ]
  do
    if [ "$likey_input" = "s" ]; then
      echo -n "Press R -Read license agreement, N -Exit the installation [R/N]:"
      read -n1 likey_input
      likey_input=`echo $likey_input | tr '[:upper:]' '[:lower:]'`
    fi
    if [ "$likey_input" = "r" ]; then
      less ${bh_license}
      echo ''
      echo -n "Press Y -I agree, Any other key -I do not agree [Y/*]:"
      read -n1 likey_input
      likey_input=`echo $likey_input | tr '[:upper:]' '[:lower:]'`
      if [ "$likey_input" = "y" ]; then
        return 0
      fi
      return 1
    elif [ "$likey_input" = "n" ]; then
      return 1
    fi
    likey_input='s'
    echo ''
  done

 return 0
}

create_daemon()
{
  check_write_permission "$daemon_path"
  if [ $? = 1 ]; 
  then
    echo "<<Message>> You do not have write permission on $daemon_path. Infobright daemon script will be created inside $PREFIX."
    daemon_path=$PREFIX/
    rm -f $daemon_path$daemon_prefix
  fi
  daemon=$daemon_path$daemon_prefix
  if test -f $daemon; then
    daemon=$daemon_path$daemon_prefix"_"`date +%F`
    echo "Note: $daemon_prefix script already exists inside $daemon_path. The installer is \
going to create a new script $daemon. Please rename this script as you want."
  fi
  cmd_sed="sed -e 's+@BH_CONF@+$my_conf+'\
    -e 's+@BH_USER@+$user+' < support-files/mysql.server> $daemon"
  eval $cmd_sed
  if [ $? != 0 ]; then
    echo "<<Warning>> Ignoring failed command: $cmd_sed \n\
The above command attempted to create a infobright start/stop script $daemon.\n\
You can manually edit @BH_CONF@ and @BH_USER@ variables inside support-files/mysql.server and copy \
it to ${daemon}."
  else
    chmod 755 $daemon
  fi
}

create_client_script()
{
  check_write_permission "`dirname $client_script`"
  if [ $? = 1 ]; 
  then
    echo "<<Message>> You do not have write permission on `dirname $client_script`. Infobright client script will be created inside $PREFIX."
    client_script=$PREFIX/mysql-ib
    rm -f $client_script   
  fi
  if test -f $client_script; then
    echo "Note: $client_script already exists. The installer is \
going to create a new link $client_script"_"`date +%F`. Please rename this link as you want."
    client_script=$client_script"_"`date +%F`
  fi
  rm -f $client_script
  ln -s $PREFIX/bin/mysql $client_script
}

for arg do
  case "$arg" in
    --datadir=*) bh_datadir=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --cachedir=*) bh_cachedir=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --port=*) my_port=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --config=*) my_conf=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --socket=*) my_sock=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --user=*) user=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --group=*) group=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
    --help) usage ;;
    --upgrade) upgrade="true" ;;
    *) usage ;;
  esac
done

echo "Infobright installation script is running..."

echo "Checking system configuration..."
check_system_conf || exit 1
verify_args || usage
test ! -z "$upgrade" && upgrade_server

paths_exist || exit 1

test -z "$group" && group=$user

echo "Creating/verifying user ${user}..."
id $user 1>/dev/null 2>/dev/null
if [ $? = 1 ]; then
  check_write_permission /etc/passwd
  if [ $? != 0 ]; then
    echo "Error you do not have permission to create user $user"
    undo_with_exit
  fi
  $ibuseradd $user 2>/dev/null
  if [ $? = 0 ]; then
    have_user_created="true"
    echo "User $user is created."
  else
    echo "Failed on $ibuseradd $user [ignored]"
  fi
fi

# If the my-ib config path is not same as default.
check_write_permission "`dirname $my_conf`"
if [ $? = 1 ]; 
then
  echo "<<Message>> You do not have write permission on `dirname $my_conf`. my-ib.cnf file will be created inside $PREFIX."   
  my_conf="$PREFIX/my-ib.cnf"
fi
if [ "$my_conf" != "/etc/my-ib.cnf" ];
then
  my_master_conf="`dirname $my_conf`/my-ib-master.cnf"
  my_slave_conf="`dirname $my_conf`/my-ib-slave.cnf"
fi

bh_license="./infobright.license"
if test -f $bh_license; then
  license_agreement $bh_license
  if [ $? != 0 ]; then
    echo -ne "\nYou did not agree on infobright license agreement."
    echo -e "\nYou have to agree to the license agreement to install infobright.\nThank you!"
    exit 1
  fi
fi

if [ "$bh_datadir" != "$PREFIX/data" ]; then
  if test ! -d $bh_datadir; then 
    mkdir -p --mode=700 $bh_datadir; 
    if [ $? != 0 ]; then
      echo "Error on mkdir $bh_datadir"
      undo_with_exit
    fi
    have_datadir="true"
  fi
fi  
if test ! -d $bh_cachedir; then 
  mkdir -p --mode=700 $bh_cachedir;
  if [ $? != 0 ]; then
    echo "Error on mkdir $bh_datadir"
    undo_with_exit
  fi
  have_cachedir="true"
fi
  
create_symlink_libclient $PREFIX

cmd_sed="sed -e 's+@BH_PORT@+$my_port+'\
  -e 's+@BH_BASEDIR@+$PREFIX+'\
  -e 's+@BH_SOCK@+$my_sock+'\
  -e 's+@BH_DATADIR@+$bh_datadir+' < $my_conf_in > $my_conf"
eval $cmd_sed
if [ $? != 0 ]; then
  echo $cmd_sed
  undo_with_exit
fi
have_conf="true"
chmod 755 $my_conf
if test $? != 0; then echo "Failed on chmod 755 $my_conf"; undo_with_exit; fi

cmd_sed="sed -e 's+@BH_PORT@+$my_port+'\
  -e 's+@BH_BASEDIR@+$PREFIX+'\
  -e 's+@BH_SOCK@+$my_sock+'\
  -e 's+@BH_DATADIR@+$bh_datadir+' < $my_master_conf_in > $my_master_conf"
eval $cmd_sed
if [ $? != 0 ]; then
  echo $cmd_sed
  undo_with_exit
fi
have_master_conf="true"
chmod 755 $my_master_conf
if test $? != 0; then echo "Failed on chmod 755 $my_master_conf"; undo_with_exit; fi

cmd_sed="sed -e 's+@BH_PORT@+$my_port+'\
  -e 's+@BH_BASEDIR@+$PREFIX+'\
  -e 's+@BH_SOCK@+$my_sock+'\
  -e 's+@BH_DATADIR@+$bh_datadir+' < $my_slave_conf_in > $my_slave_conf"
eval $cmd_sed
if [ $? != 0 ]; then
  echo $cmd_sed
  undo_with_exit
fi
have_slave_conf="true"
chmod 755 $my_slave_conf
if test $? != 0; then echo "Failed on chmod 755 $my_slave_conf"; undo_with_exit; fi

chown -R $user . 2>/dev/null
[ $? != 0 ] && echo "<<Warning>> Failed on chown -R $user .";
chown -R $user $bh_datadir $bh_cachedir 2>/dev/null
[ $? != 0 ] && echo "<<Warning>> Failed on chown -R $user $bh_datadir $bh_cachedir";
chgrp -R $group . $bh_datadir $bh_cachedir 2>/dev/null
[ $? != 0 ] && echo "<<Warning>> Failed on chgrp -R $group .";

with_sudo=""
is_redhat
[ $? -eq 1 ] && with_sudo="sudo -u $user"
echo "Installing default databases..."
cmd_sed="$with_sudo $PREFIX/scripts/mysql_install_db\
  --defaults-file=$my_conf\
  --user=$user\
  --basedir=$PREFIX\
  --datadir="$bh_datadir"\
  --cachedir=$bh_cachedir --force "
eval $cmd_sed
if [ $? != 0 ]; then
  echo "Failed on: $cmd_sed"
  undo_with_exit
fi

create_daemon
create_client_script
if [ $? != 0 ]; then
  echo "Failed on: creating a client script. Please investigate the error message."
  undo_with_exit
fi

# Create advanced configuration.
version_comment=`$PREFIX/bin/mysqld --no-defaults --version 2>/dev/null`
ice=`echo $version_comment |grep ice 2>/dev/null`
(test -z "$ice" && $PREFIX/confman.sh --defaults-file=$my_conf --basedir=$PREFIX --autoconfigure) > /dev/null 

echo "============================="
echo "Please see README or User guide for instructions related to start/stop the Infobright server and connect to it."
echo "Please verify messages or warnings (if any)."
echo "============================="
echo "Infobright installed with:"
echo "base directory [$PREFIX]"
echo "config file    [$my_conf]"
echo "client/client script [$client_script]"
echo "daemon script  [$daemon_path$daemon_prefix]"
echo "For user/group [$user/$group]."
echo "============================="
[ -f "$ibregis" ] && UserRegistration
