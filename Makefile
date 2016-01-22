
MAKEFLAGS += -rR --include-dir=$(CURDIR)
MAKEFLAGS += --no-print-directory

# Avoid funny character set dependencies
unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC

# Avoid interference with shell env settings
unexport GREP_OPTIONS


ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

ifneq ($(filter 4.%,$(MAKE_VERSION)),)	# make-4
ifneq ($(filter %s ,$(firstword x$(MAKEFLAGS))),)
  quiet=silent_
endif
else					# make-3.8x
ifneq ($(filter s% -s%,$(MAKEFLAGS)),)
  quiet=silent_
endif
endif

export quiet Q KBUILD_VERBOSE

KCONFIG_CONFIG  ?= .config
export KCONFIG_CONFIG

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)

HOSTCC       = gcc
HOSTCXX      = g++
HOSTCFLAGS   = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89
HOSTCXXFLAGS = -O2

KBUILD_MODULES :=
KBUILD_BUILTIN := 1


export KBUILD_MODULES KBUILD_BUILTIN
export KBUILD_CHECKSRC KBUILD_SRC KBUILD_EXTMOD


ifeq ("$(origin C)", "command line")
  KBUILD_CHECKSRC = $(C)
endif
ifndef KBUILD_CHECKSRC
  KBUILD_CHECKSRC = 0
endif

#default target
PHONY := _all
_all:

# Cancel implicit rules on top Makefile
$(CURDIR)/Makefile Makefile: ;


ifeq ($(KBUILD_SRC),)
        # building in the source tree
        srctree := .
else
        ifeq ($(KBUILD_SRC)/,$(dir $(CURDIR)))
                # building in a subdirectory of the source tree
                srctree := ..
        else
                srctree := $(KBUILD_SRC)
        endif
endif
objtree         := .
src             := $(srctree)
obj             := $(objtree)

VPATH           := $(srctree)$(if $(KBUILD_EXTMOD),:$(KBUILD_EXTMOD))

export srctree objtree VPATH

SUBARCH		= $(CONFIG_ARCH)



ARCH            ?= $(SUBARCH)
CROSS_COMPILE   ?= $(CONFIG_CROSS_COMPILE:"%"=%)
CONFIG_ARCH	?=
MCU		?= $(CONFIG_MCU:"%"=%)
MCPU		?= $(CONFIG_CPU:"%"=%)

ifeq ($(SUBARCH),armv7-m)
	ARCH := arm
endif




scripts/Kbuild.include: ;
include scripts/Kbuild.include

# Make variables (CC, etc...)
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CPP             = $(CC) -E
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
AWK             = awk
GENKSYMS        = scripts/genksyms/genksyms
INSTALLKERNEL  := installkernel
DEPMOD          = /sbin/depmod
PERL            = perl
PYTHON          = python
CHECK           = sparse

CHECKFLAGS     := -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ \
                  -Wbitwise -Wno-return-void $(CF)
CFLAGS_MODULE   =
AFLAGS_MODULE   =
LDFLAGS_MODULE  =
CFLAGS_KERNEL   =
AFLAGS_KERNEL   =
CFLAGS_GCOV     = -fprofile-arcs -ftest-coverage


LINUXINCLUDE := \
	$(if $(KBUILD_SRC), -I$(srctree)/include) \
	-Iinclude \
	-Isrc \
	-Iinclude/FreeRTOS \
	-Isrc/board/inc \
	-Isrc/chip/inc \
	-IFreeRTOS/include \
	-IFreeRTOS/portable/GCC/ARM_CM3 \
	-include $(srctree)/include/kconfig.h



KBUILD_CFLAGS = -mcpu=$(MCPU) -mthumb
LDFLAGS_jammci += --gc-sections

export VERSION PATCHLEVEL SUBLEVEL KERNELRELEASE KERNELVERSION
export ARCH SRCARCH CONFIG_SHELL HOSTCC HOSTCFLAGS CROSS_COMPILE AS LD CC
export CPP AR NM STRIP OBJCOPY OBJDUMP
export MAKE AWK GENKSYMS INSTALLKERNEL PERL PYTHON UTS_MACHINE
export HOSTCXX HOSTCXXFLAGS LDFLAGS_MODULE CHECK CHECKFLAGS

export KBUILD_CPPFLAGS NOSTDINC_FLAGS LINUXINCLUDE OBJCOPYFLAGS LDFLAGS
export KBUILD_CFLAGS CFLAGS_KERNEL CFLAGS_MODULE CFLAGS_GCOV CFLAGS_KASAN
export KBUILD_AFLAGS AFLAGS_KERNEL AFLAGS_MODULE
export KBUILD_AFLAGS_MODULE KBUILD_CFLAGS_MODULE KBUILD_LDFLAGS_MODULE
export KBUILD_AFLAGS_KERNEL KBUILD_CFLAGS_KERNEL
export KBUILD_ARFLAGS

export MCPU MCU


# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic
	$(Q)rm -f .tmp_quiet_recordmcount

scripts/basic/%: scripts_basic ;

PHONY += outputmakefile
outputmakefile: ;


PHONY += all
ifeq ($(KBUILD_EXTMOD),)
_all: all
else
_all: modules
endif



# To make sure we do not include .config for any of the *config targets
# catch them early, and hand them over to scripts/kconfig/Makefile
# It is allowed to specify more targets when calling make, including
# mixing *config targets and build targets.
# For example 'make oldconfig all'.
# Detect when mixed targets is specified, and make a second invocation
# of make so .config is not included in this case either (for *config).

version_h := include/generated/uapi/linux/version.h
old_version_h := include/linux/version.h

no-dot-config-targets := clean mrproper distclean \
			 cscope gtags TAGS tags help% %docs check% coccicheck \
			 $(version_h) headers_% archheaders archscripts \
			 kernelversion %src-pkg

config-targets := 0
mixed-targets  := 0
dot-config     := 1

ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

ifeq ($(KBUILD_EXTMOD),)
        ifneq ($(filter config %config,$(MAKECMDGOALS)),)
                config-targets := 1
                ifneq ($(words $(MAKECMDGOALS)),1)
                        mixed-targets := 1
                endif
        endif
endif

ifeq ($(mixed-targets),1)
# ===========================================================================
# We're called with mixed targets (*config and build targets).
# Handle them one by one.

PHONY += $(MAKECMDGOALS) __build_one_by_one

$(filter-out __build_one_by_one, $(MAKECMDGOALS)): __build_one_by_one
	@:

__build_one_by_one:
	$(Q)set -e; \
	for i in $(MAKECMDGOALS); do \
		$(MAKE) -f $(srctree)/Makefile $$i; \
	done

else
ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

# Read arch specific Makefile to set KBUILD_DEFCONFIG as needed.
# KBUILD_DEFCONFIG may point out an alternative default configuration
# used for 'make defconfig'
#include arch/$(SRCARCH)/Makefile
export KBUILD_DEFCONFIG KBUILD_KCONFIG

config: scripts_basic outputmakefile FORCE
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

%config: scripts_basic outputmakefile FORCE
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

else
# ===========================================================================
# Build targets only - this includes vmlinux, arch specific targets, clean
# targets and others. In general all targets except *config targets.

ifeq ($(KBUILD_EXTMOD),)
# Additional helpers built in scripts/
# Carefully list dependencies so we do not try to build scripts twice
# in parallel
PHONY += scripts
scripts: scripts_basic include/config/auto.conf include/config/tristate.conf
	$(Q)$(MAKE) $(build)=$(@)

# Objects we will link into vmlinux / subdirs we need to visit
core-y		:= core/ FreeRTOS/ src/
endif # KBUILD_EXTMOD

ifeq ($(dot-config),1)
# Read in config
-include include/config/auto.conf

ifeq ($(KBUILD_EXTMOD),)
# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
-include include/config/auto.conf.cmd

# To avoid any implicit rule to kick in, define an empty command
$(KCONFIG_CONFIG) include/config/auto.conf.cmd: ;

# If .config is newer than include/config/auto.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
include/config/%.conf: $(KCONFIG_CONFIG) include/config/auto.conf.cmd
	$(Q)$(MAKE) -f $(srctree)/Makefile silentoldconfig
else
# external modules needs include/generated/autoconf.h and include/config/auto.conf
# but do not care if they are up-to-date. Use auto.conf to trigger the test
PHONY += include/config/auto.conf

include/config/auto.conf:
	$(Q)test -e include/generated/autoconf.h -a -e $@ || (		\
	echo >&2;							\
	echo >&2 "  ERROR: Kernel configuration is invalid.";		\
	echo >&2 "         include/generated/autoconf.h or $@ are missing.";\
	echo >&2 "         Run 'make oldconfig && make prepare' on kernel src to fix it.";	\
	echo >&2 ;							\
	/bin/false)

endif # KBUILD_EXTMOD

else
# Dummy target needed, because used as prerequisite
include/config/auto.conf: ;
endif # $(dot-config)


ifeq ($(KBUILD_EXTMOD),)

jammci-dirs    := $(patsubst %/,%,$(filter %/, $(core-y) $(core-m)))

jammci-alldirs := $(sort $(vmlinux-dirs) $(patsubst %/,%,$(filter %/, \
			$(core-))))

core-y          := $(patsubst %/, %/built-in.o, $(core-y))

export KBUILD_JAMMCI_CORE := $(core-y)
export KBUILD_LDS          := linker/afc.lds

jammci-deps := $(KBUILD_LDS) $(KBUILD_JAMMCI_CORE)

      cmd_link-jammci = $(CONFIG_SHELL) $< $(LD) $(LDFLAGS) $(LDFLAGS_jammci)
quiet_cmd_link-jammci = LINK    $@


echo: ;

jammci: scripts/link-jammci.sh $(jammci-deps) FORCE
	+$(call if_changed,link-jammci)

$(sort $(jammci-deps)): $(jammci-dirs);


#NOSTDINC_FLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)
NOSTDINC_FLAGS += -isystem $(shell $(CC) -print-file-name=include)
CHECKFLAGS     += $(NOSTDINC_FLAGS)

ifdef CONFIG_FREERTOS

CC_USE_FREERTOS := $(call cc-option, -DUSE_FREERTOS=1)

endif

CC_JAMMCI_FLAGS := $(call cc-option, -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCR_INTEGER_PRINTF -D__LPC17XX__ -D__REDLIB__)

#-DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_LPCOPEN -DCR_INTEGER_PRINTF -D__LPC17XX__ -D__REDLIB__ -DUSE_FREERTOS=1 -I"D:\Devel\projekty\uTCA\afc_afck\bootloader_lpcxpresso_1769\src\board\inc" -I"D:\Devel\projekty\uTCA\afc_afck\bootloader_lpcxpresso_1769\src\chip\inc" -I"D:\Devel\projekty\uTCA\afc_afck\bootloader_lpcxpresso_1769\src" -I"D:\Devel\projekty\uTCA\afc_afck\bootloader_lpcxpresso_1769\FreeRTOS\include" -I"D:\Devel\projekty\uTCA\afc_afck\bootloader_lpcxpresso_1769\FreeRTOS\portable\GCC\ARM_CM3" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs

KBUILD_CFLAGS += $(CC_USE_FREERTOS) $(CC_JAMMCI_FLAGS)

prepare:

test-echo:
	$(Q)echo MCPU=$(MCPU)
	$(Q)echo FLAGS=$(KBUILD_CFLAGS)

PHONY += $(jammci-dirs)
$(jammci-dirs): prepare
	$(Q)$(MAKE) $(build)=$@

all: test-echo jammci
endif # KBUILD_EXTMOD


clean: $(clean-dirs)
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@find $(if $(KBUILD_EXTMOD), $(KBUILD_EXTMOD), .) $(RCS_FIND_IGNORE) \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '*.ko.*' \
		-o -name '*.dwo'  \
		-o -name '*.su'  \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.symtypes' -o -name 'modules.order' \
		-o -name modules.builtin -o -name '.tmp_*.o.*' \
		-o -name '*.gcno' \) -type f -print | xargs rm -f

endif #ifeq ($(config-targets),1)
endif #ifeq ($(mixed-targets),1)

PHONY += FORCE
FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
