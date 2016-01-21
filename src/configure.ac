dnl -*- ksh -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)dnl		Minimum Autoconf version required.

AC_INIT([brighthouse],[3.2])

AC_CANONICAL_SYSTEM
AC_CONFIG_SRCDIR(storage/brighthouse/util/empty.cpp)
AC_CONFIG_HEADERS([config.h])

sinclude(config/ac-macros/ssl.m4)

# Canonicalize the configuration name.

# Check whether --with-system-type or --without-system-type was given.
AC_ARG_WITH(system-type,
    [  --with-system-type      Set the system type, like "sun-solaris10"],
    [SYSTEM_TYPE="$withval"],
    [SYSTEM_TYPE="$host_vendor-$host_os"])
AC_ARG_WITH(machine-type,
    [  --with-machine-type     Set the machine type, like "powerpc"],
    [MACHINE_TYPE="$withval"],
    [MACHINE_TYPE="$host_cpu"])
AC_SUBST(SYSTEM_TYPE)
AC_DEFINE_UNQUOTED([SYSTEM_TYPE], ["$SYSTEM_TYPE"],
                   [Name of system, eg sun-solaris])
AC_SUBST(MACHINE_TYPE)
AC_DEFINE_UNQUOTED([MACHINE_TYPE], ["$MACHINE_TYPE"],
                   [Machine type name, eg sparc])

BASE_MACHINE_TYPE=$MACHINE_TYPE
ARCHFLAGS=

# Detect processor type and architecture
gccinfo=`gcc --version | grep GCC`
cpuhalinfo=`lshal -u acpi_CPU0 | grep info.product`
if test "x$cpuhalinfo" = "x" ; then
  cpuhalinfo=`lshal -u acpi_CPU1 | grep info.product`
fi
isintel=`echo $cpuhalinfo | grep Intel`
isintelcore2=`echo $cpuhalinfo | grep "Core(TM)2"`
isinteldualcore=`echo $cpuhalinfo | grep Intel | grep "Dual-Core"`
isintelextreme=`echo $cpuhalinfo | grep Intel | grep Extreme`
isamd=`echo $cpuhalinfo | grep AMD`
isamd64=`echo $cpuhalinfo | grep AMD | grep 64`
isamdathlon=`echo $cpuhalinfo | grep Athlon`
isamdathlon64=`echo $cpuhalinfo | grep Athlon | grep 64`
isamdfam10=`echo $cpuhalinfo | grep AMD | grep 10th`
isamdsse3=`echo $cpuhalinfo | grep AMD | grep sse3`
isamdopteron=`echo $cpuhalinfo | grep Opteron`
isamdphenom=`echo $cpuhalinfo | grep Phenom`
issparc=`echo $cpuhalinfo | grep SPARC`

MACHINE_SPECIFIC=""

case $gccinfo in
  *4.3.*)
      if test "x" != "x$isintelcore2" -o "x" != "x$isinteldualcore" ; then
         ARCHFLAGS="$ARCHFLAGS -march=core2 "
      elif test "x" != "x$isintelextreme" -o "x" != "x$isintel" ; then
         ARCHFLAGS="$ARCHFLAGS "
         MACHINE_SPECIFIC=" -march=nocona "
      elif test "x" != "x$isamdfam10" -o "x" != "x$isamdphenom" ; then
         ARCHFLAGS="$ARCHFLAGS -march=amdfam10 "
      elif test "x" != "x$isamdsse3" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8-sse3 "
      elif test "x" != "x$isamdopteron" ; then
         ARCHFLAGS="$ARCHFLAGS -march=opteron "
      elif test "x" != "x$isamd64" -o "x" != "x$isamdathlon64" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamd" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$issparc" ; then
         ARCHFLAGS="$ARCHFLAGS -mcpu=v9 -m64 -pthreads "
      fi
    ;;
  *4.2.*)
      if test "x" != "x$isintelcore2" -o "x" != "x$isinteldualcore" ; then
         ARCHFLAGS="$ARCHFLAGS "
         MACHINE_SPECIFIC=" -march=nocona "
      elif test "x" != "x$isintelextreme" -o "x" != "x$isintel" ; then
         ARCHFLAGS="$ARCHFLAGS "
         MACHINE_SPECIFIC=" -march=nocona "
      elif test "x" != "x$isamdfam10" -o "x" != "x$isamdphenom" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamdsse3" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamdopteron" ; then
         ARCHFLAGS="$ARCHFLAGS -march=opteron "
      elif test "x" != "x$isamd64" -o "x" != "x$isamdathlon64" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamd" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$issparc" ; then
         ARCHFLAGS="$ARCHFLAGS -mcpu=v9 -m64 -pthreads "
      fi
    ;;
  *4.1.*)
      if test "x" != "x$isintelcore2" -o "x" != "x$isinteldualcore" ; then
         ARCHFLAGS="$ARCHFLAGS "
         MACHINE_SPECIFIC=" -march=nocona "
      elif test "x" != "x$isintelextreme" -o "x" != "x$isintel" ; then
         ARCHFLAGS="$ARCHFLAGS "
         MACHINE_SPECIFIC=" -march=nocona "
      elif test "x" != "x$isamdfam10" -o "x" != "x$isamdphenom" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamdsse3" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamdopteron" ; then
         ARCHFLAGS="$ARCHFLAGS -march=opteron "
      elif test "x" != "x$isamd64" -o "x" != "x$isamdathlon64" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$isamd" ; then
         ARCHFLAGS="$ARCHFLAGS -march=k8 "
      elif test "x" != "x$issparc" ; then
         ARCHFLAGS="$ARCHFLAGS -mcpu=v9 -m64 -pthreads "
      fi
    ;;
esac


case $MACHINE_TYPE in
  *i?86*|*I?86*) 
		BASE_MACHINE_TYPE=i386
        ARCHFLAGS="$ARCHFLAGS -m32 "
	;;
  *?86?64*) 
		BASE_MACHINE_TYPE=x86_64
        ARCHFLAGS="$ARCHFLAGS -m64 -fprefetch-loop-arrays ${MACHINE_SPECIFIC} "
	;;
  *amd64*|*Amd64*|*AMD64*) 
		BASE_MACHINE_TYPE=x86_64
        ARCHFLAGS="$ARCHFLAGS -m64 -fprefetch-loop-arrays ${MACHINE_SPECIFIC} "
	;;
  *EM64?*|*em64?*) 
		BASE_MACHINE_TYPE=x86_64
        ARCHFLAGS="$ARCHFLAGS -mtune=nocona -fprefetch-loop-arrays ${MACHINE_SPECIFIC} "
	;;
esac

DETECT_MATCH_TYPE=`uname -m`
case $DETECT_MATCH_TYPE in
  *64*) 
		BH_MACHTYPE="x86_64"
	;;
  *i86pc*) 
		BH_MACHTYPE="x86_64"
	;;
  *) 
		BH_MACHTYPE="i686"
	;;
esac

issunos=`uname -s | grep SunOS`
if  test "x$issunos" != "x" ; then
     ARCHFLAGS=" -m64 "
     # This is a hack for amd64 and only valid for amd64. Right now we do not support x86.
     CXXLDFLAGS="$CXXLDFLAGS -L/usr/sfw/lib/amd64 -lkstat -lgcc_s "
fi

AC_SUBST(ARCHFLAGS)

NATIVE_COMPILER_PREFIX=`uname -m`
AC_SUBST(NATIVE_COMPILER_PREFIX)
nativegcc=`cd /usr/bin; ls ${NATIVE_COMPILER_PREFIX}*gcc`
if test "x" = "x$nativegcc" ; then
  nativegcc=gcc
fi
nativegxx=`cd /usr/bin; ls ${NATIVE_COMPILER_PREFIX}*g++`
if test "x" = "x$nativegxx" ; then
  nativegxx=g++
fi
AC_SUBST(nativegcc)
AC_SUBST(nativegxx)
CC="$nativegcc"
CXX="$nativegxx"
AC_SUBST(CC)
AC_SUBST(CXX)

#ACX_PTHREAD

# Save some variables and the command line options for mysqlbug
SAVE_CC="$CC"
SAVE_CXX="$CXX"
SAVE_ASFLAGS="$ASFLAGS"
SAVE_CFLAGS="$CFLAGS"
SAVE_CXXFLAGS="$CXXFLAGS"
SAVE_LDFLAGS="$LDFLAGS"
SAVE_CXXLDFLAGS="$CXXLDFLAGS"
CONF_COMMAND="$0 $ac_configure_args"
AC_SUBST(CONF_COMMAND)
AC_SUBST(SAVE_CC)
AC_SUBST(SAVE_CXX)
AC_SUBST(SAVE_ASFLAGS)
AC_SUBST(SAVE_CFLAGS)
AC_SUBST(SAVE_CXXFLAGS)
AC_SUBST(SAVE_LDFLAGS)
AC_SUBST(SAVE_CXXLDFLAGS)
AC_SUBST(CXXLDFLAGS)

# This is needed is SUBDIRS is set
AC_PROG_MAKE_SET
AC_PROG_INSTALL
dnl AC_PROG_MKDIR_P dnl This macro is supported from 2.60 version of autoconf.


##############################################################################
# The below section needs to be done before AC_PROG_CC
##############################################################################

# Hack for OS X/Darwin and Metrowerks CodeWarrior
AC_ARG_WITH(darwin-mwcc,
[  --with-darwin-mwcc      Use Metrowerks CodeWarrior wrappers on OS X/Darwin],[
 if [ "with_darwin_mwcc" = yes ] ; then
  builddir=`pwd`
  ccwrapper="$builddir/support-files/MacOSX/mwcc-wrapper"
  arwrapper="$builddir/support-files/MacOSX/mwar-wrapper"
  CC="$ccwrapper"
  CXX="$ccwrapper"
  LD="$ccwrapper"
  AR="$arwrapper"
  RANLIB=:
  export CC CXX LD AR RANLIB
  AC_SUBST(AR)
  AC_SUBST(RANLIB)
 fi
])

if test "x${CFLAGS-}" = x ; then
  cflags_is_set=no
else
  cflags_is_set=yes
fi

if test "x${CPPFLAGS-}" = x ; then
  cppflags_is_set=no
else
  cppflags_is_set=yes
fi

if test "x${LDFLAGS-}" = x ; then
  ldflags_is_set=no
else
  ldflags_is_set=yes
fi

################ End of section to be done before AC_PROG_CC #################

# The following hack should ensure that configure doesn't add optimizing
# or debugging flags to CFLAGS or CXXFLAGS
# C_EXTRA_FLAGS are flags that are automaticly added to both
# CFLAGS and CXXFLAGS
PTHREAD_CFLAGS="-D_THREAD_SAFE -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS "
PTHREAD_LIBS="-lboost_thread -lpthread "
###C_EXTRA_FLAGS="$C_EXTRA_FLAGS $PTHREAD_CFLAGS $ARCHFLAGS -fPIC -fexceptions -ftls-model=global-dynamic -fno-strict-aliasing -ffloat-store "
C_EXTRA_FLAGS="$C_EXTRA_FLAGS $PTHREAD_CFLAGS $ARCHFLAGS -fno-strict-aliasing -fno-merge-constants -fno-gcse "
CFLAGS="$CFLAGS $C_EXTRA_FLAGS "
CXXFLAGS="$CXXFLAGS $C_EXTRA_FLAGS -Wno-deprecated "
if  test "x$issunos" != "x" ; then
     CXXFLAGS="$CXXFLAGS -DSOLARIS "
fi

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC(${nativegcc})
AC_PROG_CXX(${nativegxx})
AC_PROG_CPP(${nativegxx})
#AC_LANG(C++)

#
# Some system specific hacks
#

#---------------------------------------------------------------
#---------------------------------------------------------------
#---------------------------------------------------------------
#---------------------------------------------------------------

C_EXTRA_OPT_FLAGS=" -finline-limit=32000 "
MAX_C_OPTIMIZE=" -O3 $C_EXTRA_OPT_FLAGS "
MAX_CXX_OPTIMIZE=" -O3 $C_EXTRA_OPT_FLAGS "

#---------------------------------------------------------------
#---------------------------------------------------------------
#---------------------------------------------------------------
#---------------------------------------------------------------

# Print version of CC and CXX compiler (if they support --version)
case $SYSTEM_TYPE in
  *netware*)
CC_VERSION=`$CC -version | grep -i version`
    ;;
  *)
CC_VERSION=`$CC --version | sed 1q`
    ;;
esac
if test $? -eq "0"
then
  AC_MSG_CHECKING("C Compiler version")
  AC_MSG_RESULT("$CC $CC_VERSION")
else
CC_VERSION=""
fi
AC_SUBST(CC_VERSION)

# Fix for sgi gcc / sgiCC which tries to emulate gcc
if test "$CC" = "sgicc"
then
  ac_cv_prog_gcc="no"
fi
if test "$CXX" = "sgi++"
then
  GXX="no"
fi

if test "$ac_cv_prog_gcc" = "yes"
then
  AS="$CC -c"
  AC_SUBST(AS)
else
  AC_PATH_PROG(AS, as, as)
fi

# Still need ranlib for readline; local static use only so no libtool.
AC_PROG_RANLIB
# We use libtool
#AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])

# static linking
AC_ARG_ENABLE(static,
	AC_HELP_STRING(
    	[--enable-static],
		[Enable static linking of system libraries.]),
    [ 
		ENABLE_STATIC=yes
		AC_ENABLE_STATIC
	],
    [ 
		ENABLE_STATIC=no
		AC_DISABLE_STATIC
	]
)

# all-static linking
ALL_STATIC=
AC_ARG_WITH(all-static,
	AC_HELP_STRING(
    	[--with-all-static],
		[Require only static linking of system libraries.]),
    [ 
		WITH_ALL_STATIC=yes
		AC_DISABLE_SHARED
		ALL_STATIC="-all-static -static"
	],
    [ 
		WITH_ALL_STATIC=no
		AC_ENABLE_SHARED
	]
)
AC_SUBST(ALL_STATIC)

# Build optimized or debug version ?
# First check for gcc and g++
if test "$ac_cv_prog_gcc" = "yes"
then
  DEBUG_CFLAGS="-g3 -ggdb3"
  #DEBUG_OPTIMIZE_CC="-O"
  #Modified for brighthouse
  DEBUG_OPTIMIZE_CC="-O0 -fno-inline"
  OPTIMIZE_CFLAGS="$MAX_C_OPTIMIZE"
  OPTIMIZE_CXXFLAGS="$MAX_CXX_OPTIMIZE"
  DEBUG_CXXFLAGS="-g3 -ggdb3"
  DEBUG_OPTIMIZE_CXX="-O0 -fno-inline"
else
  DEBUG_CFLAGS="-g"
  DEBUG_OPTIMIZE_CC=""
  OPTIMIZE_CFLAGS="$MAX_C_OPTIMIZE"
  DEBUG_CXXFLAGS="-g"
  DEBUG_OPTIMIZE_CXX=""
  OPTIMIZE_CXXFLAGS="$MAX_CXX_OPTIMIZE"
fi
if test "$ac_cv_prog_cxx_g" = "yes"
then
  DEBUG_CXXFLAGS="-g3 -ggdb3"
  #DEBUG_OPTIMIZE_CXX="-O"
  #Modified for brighthouse
  DEBUG_OPTIMIZE_CXX="-O0 -fno-inline"
  OPTIMIZE_CXXFLAGS="$MAX_CXX_OPTIMIZE"
  DEBUG_CFLAGS="-g3 -ggdb3"
  DEBUG_OPTIMIZE_CC="-O0 -fno-inline"
  OPTIMIZE_CFLAGS="$MAX_C_OPTIMIZE"
else
  DEBUG_CXXFLAGS="-g"
  DEBUG_OPTIMIZE_CXX=""
  OPTIMIZE_CXXFLAGS="$MAX_CXX_OPTIMIZE"
  DEBUG_CFLAGS="-g"
  OPTIMIZE_CFLAGS="$MAX_C_OPTIMIZE"
  DEBUG_OPTIMIZE_CC="-O"
fi

if expr "$SYSTEM_TYPE" : ".*netware.*" > /dev/null; then
  DEBUG_CFLAGS="-g -DDEBUG -sym internal,codeview4"
  DEBUG_CXXFLAGS="-g -DDEBUG -sym internal,codeview4"
  DEBUG_OPTIMIZE_CC="-DDEBUG"
  DEBUG_OPTIMIZE_CXX="-DDEBUG"
  OPTIMIZE_CFLAGS="-O3 -DNDEBUG"
  OPTIMIZE_CXXFLAGS="-O3 -DNDEBUG"
fi

# If the user specified CFLAGS, we won't add any optimizations
if test -n "$SAVE_CFLAGS"
then
  OPTIMIZE_CFLAGS=""
  DEBUG_OPTIMIZE_CC=""
fi
# Ditto for CXXFLAGS
if test -n "$SAVE_CXXFLAGS"
then
  OPTIMIZE_CXXFLAGS=""
  DEBUG_OPTIMIZE_CXX=""
fi

AC_ARG_WITH(debug,
    [  --with-debug            Add debug code
  --with-debug=full       Add debug code (adds memory checker, very slow)],
    [with_debug=$withval],
    [with_debug=no])
if test "$with_debug" = "yes"
then
  # Medium debug.
  AC_DEFINE([DBUG_ON], [1], [Use libdbug])
  #CFLAGS="$DEBUG_CFLAGS $DEBUG_OPTIMIZE_CC -DSAFE_MUTEX $CFLAGS"
  #CXXFLAGS="$DEBUG_CXXFLAGS $DEBUG_OPTIMIZE_CXX -DSAFE_MUTEX $CXXFLAGS"
  #Modified for brighthouse
  CFLAGS="$DEBUG_CFLAGS $DEBUG_OPTIMIZE_CC $CFLAGS"
  CXXFLAGS="$DEBUG_CXXFLAGS $DEBUG_OPTIMIZE_CXX $CXXFLAGS $EXTRA_CXXFLAGS"
elif test "$with_debug" = "full"
then
  # Full debug. Very slow in some cases
  AC_DEFINE([DBUG_ON], [1], [Use libdbug])
  #CFLAGS="$DEBUG_CFLAGS -DSAFE_MUTEX -DSAFEMALLOC $CFLAGS"
  #CXXFLAGS="$DEBUG_CXXFLAGS -DSAFE_MUTEX -DSAFEMALLOC $CXXFLAGS"
  #Modified for brighthouse
  CFLAGS="$DEBUG_CFLAGS -DSAFEMALLOC $CFLAGS"
  CXXFLAGS="$DEBUG_CXXFLAGS -DSAFEMALLOC $CXXFLAGS $EXTRA_CXXFLAGS"
else
  # Optimized version. No debug
  AC_DEFINE([DBUG_OFF], [1], [Don't use libdbug]) dnl '
  CFLAGS="$OPTIMIZE_CFLAGS $CFLAGS"
  CXXFLAGS="$OPTIMIZE_CXXFLAGS $CXXFLAGS $EXTRA_CXXFLAGS"
fi

AC_ARG_WITH([coverage],[AC_HELP_STRING([--with-coverage],[Compile with coverage information included, default is no coverage data added.])],[COVERAGE=$withval],[COVERAGE="no"])
AC_ARG_WITH([profile],[AC_HELP_STRING([--with-profile],[Compile with profiling information included, default is no profiling included added.])],[PROFILE=$withval],[PROFILE="no"])
AC_ARG_WITH([embedded],[AC_HELP_STRING([--with-embedded],[Compile with embedded version of infobright library.])],[EMBEDDED=$withval],[EMBEDDED="no"])
AC_ARG_WITH([assert],[AC_HELP_STRING([--with-assert],[Compile with keeping assert in infobright code.])],[with_assert=$withval],[with_assert="no"])
AC_ARG_WITH([FET],[AC_HELP_STRING([--with-FET],[Compile with FET (function execution time enable.])],[with_FET=$withval],[with_FET="no"])
AC_ARG_WITH([NUMA],[AC_HELP_STRING([--with-NUMA],[Compile with NUMA (Non Uniform Memory Architecture.])],[with_NUMA=$withval],[with_NUMA="no"])

# Some usefull subst
AC_SUBST(CC)
AC_SUBST(GXX)

# Set configuration options for make_binary_distribution
AC_SUBST(MAKE_BINARY_DISTRIBUTION_OPTIONS)


AC_CHECK_SIZEOF(char*, 8)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
if test "$ac_cv_sizeof_int" -eq 0
then
  AC_MSG_ERROR("No size for int type.")
fi
AC_CHECK_SIZEOF(long, 8)
if test "$ac_cv_sizeof_long" -eq 0
then
  AC_MSG_ERROR("No size for long type.")
fi
AC_CHECK_SIZEOF(long long, 8)
if test "$ac_cv_sizeof_long_long" -eq 0
then
  AC_MSG_ERROR("MySQL needs a long long type.")
fi
# off_t is not a builtin type
AC_CHECK_SIZEOF(off_t, 8)
if test "$ac_cv_sizeof_off_t" -eq 0
then
  AC_MSG_ERROR("MySQL needs a off_t type.")
fi
AC_CHECK_TYPES([sigset_t, off_t], [], [], [#include <sys/types.h>])
AC_CHECK_TYPES([size_t], [], [], [#include <stdio.h>])
AC_CHECK_TYPES([u_int32_t])
AC_CHECK_TYPES([int32_t])
AC_CHECK_TYPES([uint32])
AC_CHECK_TYPES([int32])
AC_CHECK_TYPES([u_int64_t])
AC_CHECK_TYPES([int64_t])
AC_CHECK_TYPES([uint64])
AC_CHECK_TYPES([int64])
AC_CHECK_TYPES([long int])
AC_CHECK_TYPES([long long int])

#--------------------------------------------------------------------
# Check for system libraries. Adds the library to $LIBS
# and defines HAVE_LIBM etc
#--------------------------------------------------------------------

AC_CHECK_LIB(m, floor, [], AC_CHECK_LIB(m, __infinity))
AC_CHECK_LIB(nsl_r, gethostbyname_r, [], AC_CHECK_LIB(nsl, gethostbyname_r))
AC_CHECK_FUNC(gethostbyname_r)
AC_CHECK_FUNC(setsockopt, , AC_CHECK_LIB(socket, setsockopt))
AC_CHECK_FUNC(yp_get_default_domain, , AC_CHECK_LIB(nsl, yp_get_default_domain))
AC_CHECK_FUNC(p2open, , AC_CHECK_LIB(gen, p2open))
# This may get things to compile even if bind-8 is installed
AC_CHECK_FUNC(bind, , AC_CHECK_LIB(bind, bind))
# Check if crypt() exists in libc or libcrypt, sets LIBS if needed
AC_SEARCH_LIBS(crypt, crypt, AC_DEFINE(HAVE_CRYPT, 1, [crypt]))
# See if we need a library for address lookup.
AC_SEARCH_LIBS(inet_aton, [socket nsl resolv])
AC_SEARCH_LIBS([dlopen],[dl],,[AC_MSG_ERROR([Cannot continue without dynamic dynamic linker library.])])
# For the sched_yield() function on Solaris
AC_CHECK_FUNC(sched_yield, , AC_CHECK_LIB(posix4, sched_yield))
# For sem_xxx functions on Solaris
AC_CHECK_FUNC(sem_init, , AC_CHECK_LIB(posix4, sem_init))

######################################################################
#                                                                    #
# Brighthouse compilation and linking configuration parameters       #
# Note: We actually adjust mysql compile flags for our code          #
# - We are using CCASFLAGS_BH_PRIVATE instead of CCASFLAGS           #
# - We are using CFLAGS_BH_PRIVATE instead of CFLAGS                 #
# - We are using CXXFLAGS_BH_PRIVATE instead of CXXFLAGS             #
#                                                                    #
######################################################################

# -- CCASFLAGS_BH_PRIVATE --
CCASFLAGS_BH_PRIVATE=`echo $CCASFLAGS`

# -- CFLAGS_BH_PRIVATE --
CFLAGS_BH_PRIVATE=`echo $CFLAGS`

# -- CXXFLAGS_BH_PRIVATE --
# For future: 
# Remove -fno-exceptions for BH complie flags since BH always use exceptions .
# Remove -fno-rtti  for BH complie flags since BOOST does not compile with -fno-rtti.
#CXXFLAGS_BH_PRIVATE=`echo $CXXFLAGS | sed -e "s|-fno-exceptions||" | sed -e "s|-fno-rtti||" `
# For Current:
CXXFLAGS_BH_PRIVATE=`echo $CXXFLAGS`

with_release="yes"
TARGET="release"

if test "$with_debug" = "yes" -o "$with_debug" = "full" ; then
  TARGET="debug"
  with_release="no"
fi

if test "$with_release" = "yes" -a "$with_assert" = "no" ; then
  CXXFLAGS="$CXXFLAGS -DNDEBUG "	
  CFLAGS="$CFLAGS -DNDEBUG "	
  CFLAGS_BH_PRIVATE="$CFLAGS_BH_PRIVATE -DNDEBUG "
  CXXFLAGS_BH_PRIVATE="$CXXFLAGS_BH_PRIVATE -DNDEBUG "
fi

if test "$with_NUMA" = "yes" ; then 
  CXXFLAGS="$CXXFLAGS -DUSE_NUMA "
  CFLAGS="$CFLAGS -DUSE_NUMA "
  CFLAGS_BH_PRIVATE="$CFLAGS_BH_PRIVATE -DUSE_NUMA "
  CXXFLAGS_BH_PRIVATE="$CXXFLAGS_BH_PRIVATE -DUSE_NUMA "
  AC_CHECK_LIB(numa, numa_available)
fi

if test "$with_FET" = "yes" ; then 
  CXXFLAGS="$CXXFLAGS -DFUNCTIONS_EXECUTION_TIMES "	
  CFLAGS="$CFLAGS -DFUNCTIONS_EXECUTION_TIMES "	
  CFLAGS_BH_PRIVATE="$CFLAGS_BH_PRIVATE -DFUNCTIONS_EXECUTION_TIMES "
  CXXFLAGS_BH_PRIVATE="$CXXFLAGS_BH_PRIVATE -DFUNCTIONS_EXECUTION_TIMES "
fi

AC_ARG_VAR([EDITION],[Build given software edition, supported values are: enterprise, evaluation, community.])
if test "x${EDITION}" = "x" ; then
	EDITION="enterprise"
fi

if test "x${EDITION}" != "xenterprise" -a "x${EDITION}" != "xevaluation" -a "x${EDITION}" != "xcommunity" ; then
	AC_MSG_ERROR([EDITION must one of enterprise, evaluation or community.])
fi

DIR_EDITION="${EDITION}"

if test "x${DIR_EDITION}" = "xevaluation" ; then
	DIR_EDITION="enterprise"
fi

BH_EDITION_FLAGS="-D__BH_ENTERPRISE__"

if test "x${EDITION}" = "xevaluation" ; then
	BH_EDITION_FLAGS="-D__BH_EVALUATION__"
else
	if test "x${EDITION}" = "xcommunity" ; then
		BH_EDITION_FLAGS="-D__BH_COMMUNITY__"
	fi
fi

UT=""
if test "$with_debug" = "yes" -a "x${EDITION}" = "xenterprise" ; then
	UT="../test/Makefile"
fi

if test "x${COVERAGE}" = "xyes" ; then
	CXXFLAGS="${CXXFLAGS} --coverage"
	TARGET="coverage"
fi
if test "x${PROFILE}" = "xyes" ; then
	CXXFLAGS="${CXXFLAGS} -pg"
	TARGET="profile"
fi
if test "x${EMBEDDED}" = "xyes" ; then
	WITH_EMBEDDED="libbrighthouse-embedded.a mysqltest_embedded"
fi

AC_SUBST(DIR_EDITION)
AC_SUBST(BH_EDITION_FLAGS)
AC_SUBST(TARGET)
AC_SUBST(CCASFLAGS_BH_PRIVATE)
AC_SUBST(CFLAGS_BH_PRIVATE)
AC_SUBST(CXXFLAGS_BH_PRIVATE)
AC_SUBST(WITH_EMBEDDED)
######################################################################
# Take care of libraries that we might want to link statically       #
######################################################################

STATIC_LDFLAGS=
PATH_BOOST_INCLUDES=""
AC_SEARCH_LIBS(crypt, crypt, AC_DEFINE(HAVE_CRYPT, 1, [crypt]))
AC_SEARCH_LIBS(pthread_self,thr pthread)
m4_include([config/m4_ax_boost_base.m4])
m4_include([config/m4_ax_boost_program_options.m4])
m4_include([config/m4_ax_boost_regex.m4])
m4_include([config/m4_ax_boost_thread.m4])
m4_include([config/m4_ax_boost_system.m4])
m4_include([config/m4_ax_boost_filesystem.m4])
AX_BOOST_BASE([1.42.0])  
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM

# We added some wrapper functions. gcc 3.4.3 that we used on Solaris does not support --wrap option in LDFLAGS
IB_WRAPPER_LDFLAGS=""
CXX_WARN_MISSING_INCLUDE_DIRS=""
if test "x$issunos" = "x" ; then
	CXX_WARN_MISSING_INCLUDE_DIRS="-Wmissing-include-dirs"
fi
AC_SUBST(IB_WRAPPER_LDFLAGS)
AC_SUBST(PATH_BOOST_INCLUDES)
AC_SUBST(LIBBOOST_FLAGS)
AC_SUBST(STATIC_LDFLAGS)
AC_SUBST(CXX_WARN_MISSING_INCLUDE_DIRS)
AC_SUBST(BH_MACHTYPE)

######################################################################
# Get the name of the folder where the mysql libraries will go       #
# Brighthous project will use them to link                           #
######################################################################

if test "$with_debug" = "yes" ; then
  Release_folder="debug"
  vendor_libsdir="../../../../vendor/mysql/debug"
	vendor_releasedir="../../../../vendor/debug"
elif test "$with_debug" = "full" ; then
  Release_folder="debug"
  vendor_libsdir="../../../../vendor/mysql/debug"
	vendor_releasedir="../../../../vendor/debug"
else
  Release_folder="release"
  vendor_libsdir="../../../../vendor/mysql/release"
	vendor_releasedir="../../../../vendor/release"
fi

mkdir -p $Release_folder
AC_SUBST(Release_folder)
AC_SUBST(vendor_libsdir)
AC_SUBST(vendor_releasedir)

######################################################################
# Brighthouse Configuration Done!                                    #
######################################################################

# Output results

AC_CONFIG_FILES(Makefile ${UT})


AC_CONFIG_COMMANDS([default], , test -z "$CONFIG_HEADERS" || echo timestamp > stamp-h)

# Ensure that table handlers gets all modifications to CFLAGS/CXXFLAGS
# CXXFLAGS="${CXXFLAGS} -Werror" # wait for better times
# CXXFLAGS="${CXXFLAGS} -Wall -Wextra -Werror" # Wait for even better times.
AC_CONFIG_COMMANDS_POST(ac_configure_args="$ac_configure_args CFLAGS='$CFLAGS' CXXFLAGS='$CXXFLAGS'")

AC_OUTPUT

echo "The configure script will try to make clean the project."
# make clean > make_clean.log # reenable when build will work.
echo "Thank you for choosing Brighthouse!"
echo
