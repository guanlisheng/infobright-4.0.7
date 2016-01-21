#!/bin/sh

my_conf="/etc/my-ib.cnf"
adv_conf=".infobright"
show_config="no"
autoconfigure="no"

echo "Infobright Advanced Configuration Manager."
echo "------------------------------------------"
DATADIR=
BH_INI="brighthouse.ini"
pid_file=
[ ! -z "`uname -a|egrep -i -e SunOS`" -o ! -z "`uname -a|egrep -i -e solaris`" ] && solaris="yes"

SED=sed
if [ ! -z "$solaris" ]; then
  if test -f "/usr/local/bin/sed"
  then
    SED="/usr/local/bin/sed"
  fi
fi

exit_with_error()
{
   echo $1
   exit 1
}

getcpucount()
{
  [ ! -z "$solaris" ] && processors=`/usr/sbin/prtconf -v|grep cpu-model|wc -l`
  [ ! -z "`uname -a|egrep -i -e Linux`" ] && processors=`cat /proc/cpuinfo |grep vendor_id|wc -l`
  [ $processors -gt 8 ] && processors=8
  [ -z "$processors"  ] && processors=1
  echo $processors
}

version=1.0.0
Threads=`getcpucount`
QueueLength=32
Depth=2
HugefileDir=
ClusterSize=2000
CachingLevel=1
BufferingLevel=2

LoaderSaveThreadNumber=16
# Disable LoaderSaveThreadNumber for 32 bit
[ -z "`uname -m | egrep -i -e x86_64`" ] && LoaderSaveThreadNumber=0
[ ! -z "$solaris" ] && LoaderSaveThreadNumber=16 

ParallelScanMaxThreads=256
ThrottleLimit=0

verify_numeric_param()
{
  param=$1
  val=$2
  isnumber=`echo $val | grep "^[0-9]*$"`
  [ -z "$isnumber" ] && exit_with_error "Error: $param should be a valid number!"
}

verify_param_values()
{
  [ ! -z "$pthreads"     ] && verify_numeric_param "--prefetch-threads" $pthreads
  [ ! -z "$pqueuelength" ] && verify_numeric_param "--prefetch-queuelength" $pqueuelength
  [ ! -z "$pdepth" ] && verify_numeric_param "--prefetch-depth" $pdepth
  [ ! -z "$ploadersavethreadnumber" ] && verify_numeric_param "--loadersavethreadnumber" $ploadersavethreadnumber
  [ ! -z "$pparallelscanmaxthreads" ] && verify_numeric_param "--parallelscan-maxthreads" $pparallelscanmaxthreads
  [ ! -z "$pthrottlelimit" ] && verify_numeric_param "--throttle-limit" $pthrottlelimit
}

# read my-ib.cnf file
##########################
parse_my_conf()
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
      --basedir=*) MY_BASEDIR=`echo "$arg" | sed -e "s;--basedir=;;"` ;;
      --datadir=*) DATADIR=`echo "$arg" | sed -e "s;--datadir=;;"` ;;
      --pid-file=*) pid_file=`echo "$arg" | sed -e "s;--pid-file=;;"` ;;
    esac
  done
}

usage()
{
  echo "Usage:"
  echo "./confman.sh options" 
  echo "[--defaults-file=path-of-mysql-config-file]" 
  echo "[--show-config]"
  echo "[--autoconfigure]"
  echo "(The following parameters value can be passed along with autoconfigure to set them by user's choice)"  
  echo "[--prefetch-threads=number of prefetch threads to be set, e.g. 4]" 
  echo "[--prefetch-queuelength=prefetch queue length to be set, e.g. 32]" 
  echo "[--prefetch-depth=prefetch depth to be set, e.g. 2]" 
  echo "[--loadersavethreadnumber=loader save thread number to be set, e.g. 16]" 
  echo "[--parallelscan-maxthreads=parallel scan max threads to be set, e.g. 256]" 
  echo "[--throttle-limit=query throttle limit to be set, e.g. 4]" 
  echo "[--help]"
  exit 1
}

for arg in $@ ; do
  case "$arg" in
    --defaults-file=*) my_conf=`echo "$arg" | $SED -e "s;--defaults-file=;;"` ;;
    --showconfig*) show_config="yes" ;;
    --show-config*) show_config="yes" ;;
    --show*) show_config="yes" ;;
    --autoconfigure*) autoconfigure="yes" ;;
    --auto-configure*) autoconfigure="yes" ;;
    --auto*) autoconfigure="yes" ;;
    --basedir=*) PREFIX=`echo "$arg" | $SED -e "s;--basedir=;;"` ;;
    --configure*) configure=`echo "$arg" | $SED -e "s;--configure=;;"` echo "'--configure' is an unused parameter." ;;
    --prefetch-threads=*) pthreads=`echo "$arg" | $SED -e "s;--prefetch-threads=;;"` ;;
    --prefetch-queuelength=*) pqueuelength=`echo "$arg" | $SED -e "s;--prefetch-queuelength=;;"` ;;
    --prefetch-depth=*) pdepth=`echo "$arg" | $SED -e "s;--prefetch-depth=;;"` ;;
    --loadersavethreadnumber=*) ploadersavethreadnumber=`echo "$arg" | $SED -e "s;--loadersavethreadnumber=;;"` ;;
    --parallelscan-maxthreads=*) pparallelscanmaxthreads=`echo "$arg" | $SED -e "s;--parallelscan-maxthreads=;;"` ;;
    --throttle-limit=*) pthrottlelimit=`echo "$arg" | $SED -e "s;--throttle-limit=;;"` ;;
    --help) usage ;;
  esac
done

if [ "$show_config" = "no" -a "$autoconfigure" = "no" ];
then
  echo "No required parameters were passed. Run this script with --help.";
  echo "You need to pass either --show-config or --autoconfigure." 
  exit 1;
fi
[ "$autoconfigure" = "yes" ] && verify_param_values

[ -z "$PREFIX" ] && PREFIX=`pwd`
if test ! -f $PREFIX/bin/mysqld -a ! -f $PREFIX/libexec/mysqld
then
  echo "Please run this script from inside the installed folder or pass a valid basedir."
  exit 1
fi

echo "Using defaults-file $my_conf."
if test ! -f "$my_conf"
then
  echo "Infobright server defaults-file $my_conf was not found!"
  exit 1
fi

# Get first arguments from the my.cnf file, groups [mysqld]
if test -x $PREFIX/bin/my_print_defaults
then
  print_defaults="$PREFIX/bin/my_print_defaults"
else
  echo "Error: conf file parser tool $PREFIX/bin/my_print_defaults was not found."
  exit 1
fi

parse_my_conf `$print_defaults --defaults-file=$my_conf mysqld`

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
  echo "Error: A infobright mysqld instance (pid $PID) is running. Please shutdown the server and then run this script."
  echo "If mysqld is not running then remove unused pid file: $pid_file to overcome this issue."
  exit 1
fi

test -z "$MY_BASEDIR" && MY_BASEDIR=$PREFIX
if [ ! -d "$MY_BASEDIR" ];
then
  echo "Error: $MY_BASEDIR does not exist."
  exit 1
fi
echo "Using basedir $MY_BASEDIR"

read_advancedconfig_paramval()
{
   advconf=$1
   param=$2
   value=`grep -wi $param $advconf | awk -F">" '{print $2}' | awk -F"<" '{print $1}'`
   value=`echo $value | $SED -e 's/ *$//'`
   echo $value
}

load_advancedconfig_file()
{
  advconf=$1
  if [ -f "$advconf" ];
  then
    version=`read_advancedconfig_paramval $advconf "version"`
    Threads=`read_advancedconfig_paramval $advconf "Threads"`
    QueueLength=`read_advancedconfig_paramval $advconf "QueueLength"`
    Depth=`read_advancedconfig_paramval $advconf "Depth"`
    HugefileDir=`read_advancedconfig_paramval $advconf "HugefileDir"`
    ClusterSize=`read_advancedconfig_paramval $advconf "ClusterSize"`
    CachingLevel=`read_advancedconfig_paramval $advconf "CachingLevel"`
    BufferingLevel=`read_advancedconfig_paramval $advconf "BufferingLevel"`
    LoaderSaveThreadNumber=`read_advancedconfig_paramval $advconf "LoaderSaveThreadNumber"`
    ParallelScanMaxThreads=`read_advancedconfig_paramval $advconf "maxthreads"`
    ThrottleLimit=`read_advancedconfig_paramval $advconf "limit"`
  fi
}

display_advancedconfig_file()
{
  advconf=$1
  load_advancedconfig_file $advconf
  
  test ! -z "$version" && echo "Option: brighthouse.version, value: "$version
  test ! -z "$Threads" && echo "Option: brighthouse.prefetch.threads, value: "$Threads
  test ! -z "$QueueLength" && echo "Option: brighthouse.prefetch.queuelength, value: "$QueueLength
  test ! -z "$Depth" && echo "Option: brighthouse.prefetch.depth, value: "$Depth
  test ! -z "$LoaderSaveThreadNumber" && echo "Option: brighthouse.loadersavethreadnumber, value: "$LoaderSaveThreadNumber
  echo "Option: brighthouse.HugefileDir, value: "$HugefileDir
  test ! -z "$ClusterSize" && echo "Option: brighthouse.clustersize, value: "$ClusterSize
  test ! -z "$CachingLevel" && echo "Option: brighthouse.cachinglevel, value: "$CachingLevel
  test ! -z "$BufferingLevel" && echo "Option: brighthouse.bufferinglevel, value: "$BufferingLevel
  test ! -z "$ParallelScanMaxThreads" && echo "Option: brighthouse.parallelscan.maxthreads, value: "$ParallelScanMaxThreads
  test ! -z "$ThrottleLimit" && echo "Option: brighthouse.throttle.limit, value: "$ThrottleLimit
}

create_advancedconfig_file()
{
  destdir=$1
  advconfigname=".infobright"
  advconfpath=$destdir/$advconfigname

  # set values from command line
  [ ! -z "$pthreads" ] && Threads=$pthreads
  [ ! -z "$pqueuelength" ] && QueueLength=$pqueuelength
  [ ! -z "$pdepth" ] && Depth=$pdepth
  [ ! -z "$ploadersavethreadnumber" ] && LoaderSaveThreadNumber=$ploadersavethreadnumber
  [ ! -z "$pparallelscanmaxthreads" ] && ParallelScanMaxThreads=$pparallelscanmaxthreads
  [ ! -z "$pthrottlelimit" ] && ThrottleLimit=$pthrottlelimit
  
  # create xml config file
  ###############################
  echo "<!--"> $advconfpath
  [ $? != 0 ] && return 1; 
  echo "IMPORTANT: This file contains configuration parameters that are automatically">> $advconfpath
  echo "set by the Infobright configuration manager. Manually changing the settings in">> $advconfpath
  echo "this file may adversely affect the behavior and performance of your Infobright">> $advconfpath
  echo "installation and is not supported by Infobright.">> $advconfpath
  echo "-->">> $advconfpath
  echo "">> $advconfpath
  echo "<Brighthouse >"                >> $advconfpath
  echo "  <Version> $version </Version>" >> $advconfpath
  echo "  <Prefetch>" >> $advconfpath
  echo "	    <Threads > $Threads </Threads>" >> $advconfpath
  echo "	    <QueueLength > $QueueLength </QueueLength>" >> $advconfpath
  echo "	    <Depth > $Depth </Depth>" >> $advconfpath
  echo "  </Prefetch>" >> $advconfpath
  echo ""                >> $advconfpath
  echo "  <HugefileDir> $HugefileDir </HugefileDir>"      >> $advconfpath
  echo "  <ClusterSize> $ClusterSize </ClusterSize>" >> $advconfpath 
  echo "  <LoaderSaveThreadNumber> $LoaderSaveThreadNumber </LoaderSaveThreadNumber>" >> $advconfpath
  echo "  <CachingLevel> $CachingLevel </CachingLevel>"          >> $advconfpath
  echo "  <BufferingLevel> $BufferingLevel </BufferingLevel>"      >> $advconfpath
  echo "  <ParallelScan>" >> $advconfpath
  echo "	    <MaxThreads > $ParallelScanMaxThreads </MaxThreads>" >> $advconfpath
  echo "  </ParallelScan>" >> $advconfpath
  echo "  <throttle>" >> $advconfpath
  echo "	    <limit > $ThrottleLimit </limit>" >> $advconfpath
  echo "  </throttle>" >> $advconfpath
  echo "</Brighthouse>"                       >> $advconfpath
  ###############################
  return 0;
}

if [ "$show_config" = "yes" ] ;
then
  [ ! -f "$MY_BASEDIR/$adv_conf" ] && exit_with_error "$MY_BASEDIR/$adv_conf is not found!"
  display_advancedconfig_file $MY_BASEDIR/$adv_conf
fi

if [ "$autoconfigure" = "yes" ];
then
  echo "Checking system configuration..."
  create_advancedconfig_file $MY_BASEDIR
  if [ -f "$MY_BASEDIR/$adv_conf" ];
  then 
    echo "Infobright advanced configuration has been reset."
    echo "Please restart the infobright server to apply the changes."
  fi
  exit 0
fi

echo "Bye!"
