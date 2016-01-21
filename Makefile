SHELL=/bin/sh
COMA=,

-include .my_make

BUILD_DIR=build
REL_DIR=../../../..

VENDOR_SRC_DIR=vendor/mysql
DRIZZLE_SRC_DIR=vendor/libdrizzle
VENDOR_MAKEFILE_AM=$(VENDOR_SRC_DIR)/Makefile.am
VENDOR_CONFIGURE_IN=$(VENDOR_SRC_DIR)/configure.in

REL_VENDOR_SRC_DIR=$(REL_DIR)/$(VENDOR_SRC_DIR)
REL_VENDOR_CONFIGURE=$(REL_VENDOR_SRC_DIR)/configure
REL_VENDOR_CONFIG_H_IN=$(REL_VENDOR_SRC_DIR)/config.h.in
REL_VENDOR_MAKEFILE_IN=$(REL_VENDOR_SRC_DIR)/Makefile.in
REL_DRIZZLE_CONFIGURE=$(REL_DIR)/$(DRIZZLE_SRC_DIR)/configure

BH_SRC_DIR=src
BH_CONFIGURE_IN=$(BH_SRC_DIR)/configure.ac
BH_MAKEFILE_IN=$(BH_SRC_DIR)/Makefile.in

REL_BH_SRC_DIR=$(REL_DIR)/$(BH_SRC_DIR)
REL_BH_CONFIGURE=$(REL_BH_SRC_DIR)/configure
REL_BH_MAKEFILE_IN=$(REL_BH_SRC_DIR)/Makefile.in
REL_BH_CONFIG_H_IN=$(REL_BH_SRC_DIR)/config.h.in
ROOT=$(shell pwd)

$(foreach S,$(subst $(COMA), ,$(SKIP)),$(eval SKIP_$(shell echo $(S)|tr a-z A-Z)?=yes))

ifneq ($(EDITION),community)
TEST_ARTIFACT=test/Makefile.in
endif

define INTERMEDIATE_SET

TARGET_DIR_VENDOR_$(2)_$(1)=$$(BUILD_DIR)/$(2)/$(1)/vendor
TARGET_DIR_DRIZZLE_$(2)_$(1)=$$(BUILD_DIR)/$(2)/$(1)/libdrizzle
BIN_TGT_LIBMYSQLD_$(2)_$(1)=$$(TARGET_DIR_VENDOR_$(2)_$(1))/libmysqld.a \
	$$(TARGET_DIR_VENDOR_$(2)_$(1))/libsql.a $$(TARGET_DIR_VENDOR_$(2)_$(1))/libmysys.a \
	$$(TARGET_DIR_VENDOR_$(2)_$(1))/libheap.a $$(TARGET_DIR_VENDOR_$(2)_$(1))/libcsv.a \
	$$(TARGET_DIR_VENDOR_$(2)_$(1))/libmyisam.a $$(TARGET_DIR_VENDOR_$(2)_$(1))/libmystrings.a \
	$$(TARGET_DIR_VENDOR_$(2)_$(1))/libvio.a $$(TARGET_DIR_VENDOR_$(2)_$(1))/libregex.a
MAKEFILE_$(2)_$(1)=$$(TARGET_DIR_VENDOR_$(2)_$(1))/Makefile
VENDOR_CONFIGURE_STAMP_$(2)_$(1)=$$(TARGET_DIR_VENDOR_$(2)_$(1))/configure.stamp
BIN_STAMP_$(2)_$(1)=$$(TARGET_DIR_VENDOR_$(2)_$(1))/bin.stamp
DRIZZLE_BIN_STAMP_$(2)_$(1)=$$(TARGET_DIR_DRIZZLE_$(2)_$(1))/bin.stamp
DRIZZLE_CONFIGURE_STAMP_$(2)_$(1)=$$(TARGET_DIR_DRIZZLE_$(2)_$(1))/configure.stamp

$$(BIN_STAMP_$(2)_$(1)): $$(MAKEFILE_$(2)_$(1)) $$(DRIZZLE_BIN_STAMP_$(2)_$(1))
	if [ "x$$(EDITION)" = "xenterprise" -o "x$$(EDITION)" = "xevaluation" ]; then \
	cd $$(TARGET_DIR_DRIZZLE_$(2)_$(1)) ; \
	touch bin.stamp configure.stamp ; \
	$$(MAKE) $$(MAKE_OPT) ; \
	cd "$$(ROOT)" > /dev/null 2>&1 ; \
	fi && \
	touch -cr $$(VENDOR_CONFIGURE_STAMP_$(2)_$(1)) $$(VENDOR_SRC_DIR)/configure $$(VENDOR_SRC_DIR)/aclocal.m4 && \
	cd $$(TARGET_DIR_VENDOR_$(2)_$(1)) && \
	touch -cr configure.stamp config.status config.h $$(REL_VENDOR_CONFIG_H_IN) stamp-h1 && \
	touch -cr Makefile $$(REL_VENDOR_MAKEFILE_IN) $$(REL_VENDOR_SRC_DIR)/*/Makefile.in $$(REL_VENDOR_SRC_DIR)/*/*/Makefile.in $$(REL_VENDOR_SRC_DIR)/*/*/*/Makefile.in $$(REL_VENDOR_SRC_DIR)/*/*/*/*/Makefile.in && \
	test -f ./include/mysqld_error.h && touch -cr ./include/mysqld_error.h $$(REL_VENDOR_SRC_DIR)/sql/share/errmsg.txt ; \
	$$(MAKE) $$(MAKE_OPT) && \
	cd "$$(ROOT)" > /dev/null 2>&1

$$(DRIZZLE_BIN_STAMP_$(2)_$(1)): $$(DRIZZLE_CONFIGURE_STAMP_$(2)_$(1))
	if [ "x$$(EDITION)" = "xenterprise" -o "x$$(EDITION)" = "xevaluation" ]; then \
	[ ! -d $$(TARGET_DIR_DRIZZLE_$(2)_$(1)) ] && mkdir -p $$(TARGET_DIR_DRIZZLE_$(2)_$(1)) ; \
	cd $$(TARGET_DIR_DRIZZLE_$(2)_$(1)) ; \
	$$(REL_DRIZZLE_CONFIGURE) --enable-static=yes --enable-shared=no  $(3) ; \
	cd "$$(ROOT)" > /dev/null 2>&1 ; \
	fi
	
$$(MAKEFILE_$(2)_$(1)): $$(VENDOR_CONFIGURE_STAMP_$(2)_$(1))
	cd $$(TARGET_DIR_VENDOR_$(2)_$(1)) && \
	$$(REL_VENDOR_CONFIGURE) --with-extra-charsets=complex --enable-static --with-gnu-ld --with-pthread --with-partition $(3) && \
	cd "$$(ROOT)" > /dev/null 2>&1

$$(DRIZZLE_CONFIGURE_STAMP_$(2)_$(1)):
	if [ "x$$(EDITION)" = "xenterprise" -o "x$$(EDITION)" = "xevaluation" ]; then \
	mkdir -p $$(TARGET_DIR_DRIZZLE_$(2)_$(1)) ; \
	cd $$(TARGET_DIR_DRIZZLE_$(2)_$(1)) ; \
	autoreconf --force --install $(REL_DIR)/$$(DRIZZLE_SRC_DIR) ; \
	cd "$$(ROOT)" > /dev/null 2>&1 ; \
	fi

$$(VENDOR_CONFIGURE_STAMP_$(2)_$(1)): $$(VENDOR_CONFIGURE_IN) $$(VENDOR_MAKEFILE_AM)
	mkdir -p $$(TARGET_DIR_VENDOR_$(2)_$(1)) && \
	cd $$(TARGET_DIR_VENDOR_$(2)_$(1)) && \
	autoreconf --force --install $$(REL_VENDOR_SRC_DIR) && \
	cd "$$(ROOT)" > /dev/null 2>&1 && \
	touch $$(VENDOR_CONFIGURE_STAMP_$(2)_$(1))

endef

define TARGET_SET

VENDOR_TARGET_DIR_$(2)_$(1)=$$(BUILD_DIR)/$(2)/$(1)/vendor
VENDOR_BIN_STAMP_$(2)_$(1)=$$(VENDOR_TARGET_DIR_$(2)_$(1))/bin.stamp

TARGET_DIR_$(2)_$(1)=$$(BUILD_DIR)/$(2)/$(1)/infobright
BIN_TGT_MYSQLD_$(2)_$(1)=$$(TARGET_DIR_$(2)_$(1))/storage/brighthouse/mysqld
MAKEFILE_$(2)_$(1)=$$(TARGET_DIR_$(2)_$(1))/Makefile
BH_CONFIGURE_STAMP_$(2)_$(1)=$$(TARGET_DIR_$(2)_$(1))/configure.stamp
BIN_STAMP_$(2)_$(1)=$$(TARGET_DIR_$(2)_$(1))/bin.stamp


$(1): $$(BIN_TGT_MYSQLD_$(2)_$(1))


$$(BIN_TGT_MYSQLD_$(2)_$(1)): $$(MAKEFILE_$(2)_$(1)) $$(BIN_STAMP_$(2)_$(1)) $$(VENDOR_BIN_STAMP_$(2)_$(1))
	BUILD_MODE=$$(BUILD_MODE) && EDITION=$$(EDITION) && export BUILD_MODE EDITION && \
	touch -cr $$(BH_CONFIGURE_STAMP_$(2)_$(1)) $$(BH_SRC_DIR)/configure $$(BH_SRC_DIR)/aclocal.m4 && \
	cd $$(TARGET_DIR_$(2)_$(1)) && \
	$$(MAKE) SKIP_ICM=$(SKIP_ICM) SKIP_DATAPROCESSOR=$(SKIP_DATAPROCESSOR) SKIP_CHMT=$(SKIP_CHMT) SKIP_EMBEDDED=$(SKIP_EMBEDDED) SKIP_UPDATER=$(SKIP_UPDATER) $$(MAKE_OPT) && \
	cd "$$(ROOT)" > /dev/null 1>&2 && \
	if [ "x$(1)" = "xdebug" -a "x$$(EDITION)" = "xenterprise" ] ; then $$(MAKE) utpp ; fi && \
	if [ "x$(1)" = "xcoverage" -a "x$$(EDITION)" = "xenterprise" ] ; then $$(MAKE) utpp WITH_COVERAGE="yes"; fi

$$(BIN_STAMP_$(2)_$(1)):

$$(MAKEFILE_$(2)_$(1)): $$(BH_CONFIGURE_STAMP_$(2)_$(1)) $$(TEST_ARTIFACT)
	cd $$(TARGET_DIR_$(2)_$(1)) && \
	$$(REL_BH_CONFIGURE) --with-extra-charsets=complex --with-gnu-ld --with-partition $(3) && \
	cd "$$(ROOT)" > /dev/null 1>&2 && \
	touch -cr $$(BH_CONFIGURE_IN) $$(BH_SRC_DIR)/configure

$$(BH_CONFIGURE_STAMP_$(2)_$(1)): $$(BH_CONFIGURE_IN) $$(BH_MAKEFILE_IN)
	mkdir -p $$(TARGET_DIR_$(2)_$(1)) && \
	cd $$(BH_SRC_DIR) && \
	libtoolize --force --install > /dev/null 2>&1 || libtoolize --force > /dev/null 2>&1 && automake --add-missing --force-missing > /dev/null 2>&1 ; \
	cd "$$(ROOT)/$$(TARGET_DIR_$(2)_$(1))" && \
	autoreconf $$(REL_BH_SRC_DIR) && \
	cd "$$(ROOT)" > /dev/null 1>&2 && \
	touch -cr $$(BH_CONFIGURE_IN) $$(BH_SRC_DIR)/configure && \
	touch $$(BH_CONFIGURE_STAMP_$(2)_$(1))

clean-$(1):
	/bin/rm -rf $$(TARGET_DIR_$(2)_$(1)) $$(VENDOR_TARGET_DIR_$(2)_$(1)) build/ut $$(TARGET_DIR_DRIZZLE_$(2)_$(1))

install-$(1): $$(BIN_TGT_MYSQLD_$(2)_$(1))
	cd $$(VENDOR_TARGET_DIR_$(2)_$(1)) && \
	$$(MAKE) install && \
	cd "$$(ROOT)" > /dev/null 1>&2 && \
	touch -r $$(BH_CONFIGURE_STAMP_$(2)_$(1)) $$(BH_SRC_DIR)/configure $$(BH_SRC_DIR)/aclocal.m4 && \
	cd $$(TARGET_DIR_$(2)_$(1)) && \
	touch -cr configure.stamp config.status config.h $$(REL_BH_CONFIG_H_IN) stamp-h1 && \
	touch -cr Makefile $$(REL_BH_MAKEFILE_IN) && \
	$$(MAKE) install && \
	cd "$$(ROOT)" > /dev/null 1>&2

endef

.PHONY: debug release coverage profile distclean utpp clean-debug clean-release clean-coverage clean-profile install-debug install-release .my_make

# defaults here

PREFIX?=/usr/local/infobright
LINK_MODE?=shared
FET?=no
ASSERT?=no
NUMA?=no

.DEFAULT all:
	@echo "Target \`$(@)' is not recognized!" ; \
	echo "Only following targets are supported (exemplary make invocations):" ; \
	echo "" ; \
	echo "make EDITION=enterprise debug - build debug version of enterprise product" ; \
	echo "make EDITION=enterprise release - build release version of enterprise product" ; \
	echo "make EDITION=enterprise coverage - build prepared for test coverage analysis" ; \
	echo "make EDITION=enterprise profile - build prepared for gathering profile statistics" ; \
	echo "make EDITION=enterprise clean-debug" ; \
	echo "make EDITION=enterprise clean-release" ; \
	echo "make EDITION=enterprise clean-coverage" ; \
	echo "make EDITION=enterprise clean-profile" ; \
	echo "make EDITION=evaluation debug - build debug version of enterprise evaluation product" ; \
	echo "make EDITION=evaluation release - build release version of enterprise evaluation product" ; \
	echo "make EDITION=evaluation clean-debug" ; \
	echo "make EDITION=evaluation clean-release" ; \
	echo "make EDITION=community debug - build debug version of community" ; \
	echo "make EDITION=community release - build release version of community" ; \
	echo "make EDITION=community clean-debug" ; \
	echo "make EDITION=community clean-release" ; \
	echo "make EDITION=community install-debug" ; \
	echo "make EDITION=enterprise EMBEDDED=yes release - build embedded library" ; \
	echo "make EDITION=enterprise ASSERT=yes release - build release with assert statement enabled" ; \
	echo "make distclean - clean everything" ; \
	echo "" ; \
	echo "General form for building the software:" ; \
	echo "make PREFIX={prefix} LINK_MODE={link} EDITION={edition} {target}" ; \
	echo "where:" ; \
	echo "PREFIX is instalation prefix" ; \
	echo "LINK_MODE is type of linking (shared|static)" ; \
	echo "EDITION is software edition (enterprise|evaluation|community)" ; \
	echo "target is build target (debug|release|clean-debug|clean-release|install-debug|install-release)" ; \
	echo "" ; \
	echo "All of those setting may be put into .my_make file which is read by Makefile." ; \
	false

ifeq ($(MAKECMDGOALS),distclean)
EDITION=enterprise
endif

ifneq ($(EDITION),enterprise)
ifneq ($(EDITION),evaluation)
ifneq ($(EDITION),community)
ifneq ($(MAKECMDGOALS),)
$(error "You must specify edition: enterprise or community (not: $(EDITION)).")
endif
endif
endif
endif

ifeq ($(EDITION),evaluation)
$(shell rm -rf $(VENDOR_SRC_DIR))
$(shell ln -sf mysql-5.1.40-ent $(VENDOR_SRC_DIR))
# no readline source.
OPT_READLINE=
else
ifeq ($(EDITION),enterprise)
$(shell rm -rf $(VENDOR_SRC_DIR))
$(shell ln -sf mysql-5.1.40-ent $(VENDOR_SRC_DIR))
# no readline source.
OPT_READLINE=
else
$(shell rm -rf $(VENDOR_SRC_DIR))
$(shell ln -sf mysql-5.1.40-comm $(VENDOR_SRC_DIR))
OPT_READLINE=--with-readline
endif
endif

ifneq ($(LINK_MODE),shared)
ifneq ($(LINK_MODE),static)
$(error "Valid values for link type are: `shared' and `static' (not: `$(LINK_MODE)').")
endif
endif

ifneq ($(FET),yes)
ifneq ($(FET),no)
$(error "Valid values for FET (Function Execution Time framework) are: `yes' and `no' (not: $(FET)).")
endif
endif

ifneq ($(NUMA),yes)
ifneq ($(NUMA),no)
$(error "Valid values for NUMA (Non Uniform Memory Architecture) are: `yes' and `no' (not: $(NUMA)).")
endif
endif

ifneq ($(ASSERT),yes)
ifneq ($(ASSERT),no)
$(error "Valid values for ASSERT (what on Earth is an ASSERT build???) are: `yes' and `no' (not: $(ASSERT)).")
endif
endif

ifneq ($(BOOST_ROOT),)
OPT_BOOST=--with-boost=$(BOOST_ROOT) BOOST_ROOT=$(BOOST_ROOT)
endif


ifeq ($(LINK_MODE),static)
OPT_LINK=$(OPT_READLINE) --with-all-static --with-client-ldflags=-all-static --with-mysqld-ldflags=-all-static --disable-shared
endif

ifeq ($(ASSERT),yes)
OPT_ASSERT=--with-assert
endif

ifeq ($(EMBEDDED),yes)
OPT_EMBEDDED_SERVER=--with-embedded-server
OPT_EMBEDDED=--with-embedded
endif

ifeq ($(FET),yes)
OPT_FET=--with-FET
endif

ifeq ($(NUMA),yes)
OPT_NUMA=--with-NUMA
endif

ifneq ($(PREFIX),)
OPT_PREFIX=--prefix=$(PREFIX)
endif

OPT_SSL=--with-ssl

INTERMEDIATE_SET_debug=--with-debug $(OPT_LINK) $(OPT_PREFIX) $(OPT_SSL) $(OPT_EMBEDDED_SERVER)
INTERMEDIATE_SET_release=$(OPT_LINK) $(OPT_PREFIX) $(OPT_SSL) $(OPT_EMBEDDED_SERVER)
INTERMEDIATE_SET_coverage=--with-debug --with-coverage $(OPT_LINK) $(OPT_PREFIX) $(OPT_SSL) $(OPT_EMBEDDED_SERVER)
INTERMEDIATE_SET_profile=--with-debug --with-profile --disable-shared $(OPT_PREFIX) $(OPT_SSL) $(OPT_EMBEDDED_SERVER)
TARGET_SET_debug=--with-debug $(OPT_LINK) $(OPT_PREFIX) $(OPT_FET) $(OPT_BOOST) $(OPT_EMBEDDED) $(OPT_NUMA)
TARGET_SET_release=$(OPT_LINK) $(OPT_PREFIX) $(OPT_ASSERT) $(OPT_FET) $(OPT_BOOST) $(OPT_EMBEDDED) $(OPT_NUMA)
TARGET_SET_coverage=--with-debug --with-coverage $(OPT_LINK) $(OPT_PREFIX) $(OPT_FET) $(OPT_BOOST) $(OPT_EMBEDDED) $(OPT_NUMA)
TARGET_SET_profile=--with-debug --with-profile --disable-shared $(OPT_PREFIX) $(OPT_FET) $(OPT_BOOST) $(OPT_EMBEDDED) $(OPT_NUMA)

TARGETS=debug release coverage profile

$(foreach TARGET,$(TARGETS),$(eval $(call INTERMEDIATE_SET,$(TARGET),$(EDITION),$(INTERMEDIATE_SET_$(TARGET)))))
$(foreach TARGET,$(TARGETS),$(eval $(call TARGET_SET,$(TARGET),$(EDITION),$(TARGET_SET_$(TARGET)))))

utpp:
	if [ "x$(WITH_COVERAGE)" != "x" ]; then \
	$(MAKE) -C ./test/ EDITION=$(EDITION) BH_ROOT=$(ROOT) -f ../build/enterprise/coverage/test/Makefile; \
	else \
	$(MAKE) -C ./test/ EDITION=$(EDITION) BH_ROOT=$(ROOT) -f ../build/enterprise/debug/test/Makefile; \
	fi

distclean:
	@/bin/rm -rf $(BUILD_DIR)
	@/usr/bin/find ./test/ -name \*.o | xargs -I {} /bin/rm {}

clean-dep:
	@test -d ./build && find ./build/ -type f -name '*.d' | xargs -I {} /bin/rm {}

# A bug look-a-like in GNU make 3.80.
.my_make:


# DO NOT DELETE
