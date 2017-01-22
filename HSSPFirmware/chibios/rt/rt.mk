# List of all the ChibiOS/RT kernel files, there is no need to remove the files
# from this list, you can disable parts of the kernel by editing chconf.h.
ifeq ($(USE_SMART_BUILD),yes)
CHCONF := $(strip $(shell cat chconf.h | egrep -e "\#define"))

KERNSRC := $(CHIBIOS)/chibios/rt/src/chsys.c \
           $(CHIBIOS)/chibios/rt/src/chdebug.c \
           $(CHIBIOS)/chibios/rt/src/chtrace.c \
           $(CHIBIOS)/chibios/rt/src/chvt.c \
           $(CHIBIOS)/chibios/rt/src/chschd.c \
           $(CHIBIOS)/chibios/rt/src/chthreads.c
ifneq ($(findstring CH_CFG_USE_TM TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chtm.c
endif
ifneq ($(findstring CH_DBG_STATISTICS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chstats.c
endif
ifneq ($(findstring CH_CFG_USE_REGISTRY TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chregistry.c
endif
ifneq ($(findstring CH_CFG_USE_SEMAPHORES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chsem.c
endif
ifneq ($(findstring CH_CFG_USE_MUTEXES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chmtx.c
endif
ifneq ($(findstring CH_CFG_USE_CONDVARS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chcond.c
endif
ifneq ($(findstring CH_CFG_USE_EVENTS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chevents.c
endif
ifneq ($(findstring CH_CFG_USE_MESSAGES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chmsg.c
endif
ifneq ($(findstring CH_CFG_USE_DYNAMIC TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/rt/src/chdynamic.c
endif
ifneq ($(findstring CH_CFG_USE_MAILBOXES TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/common/oslib/src/chmboxes.c
endif
ifneq ($(findstring CH_CFG_USE_MEMCORE TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/common/oslib/src/chmemcore.c
endif
ifneq ($(findstring CH_CFG_USE_HEAP TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/common/oslib/src/chheap.c
endif
ifneq ($(findstring CH_CFG_USE_MEMPOOLS TRUE,$(CHCONF)),)
KERNSRC += $(CHIBIOS)/chibios/common/oslib/src/chmempools.c
endif
else
KERNSRC := $(CHIBIOS)/chibios/rt/src/chsys.c \
           $(CHIBIOS)/chibios/rt/src/chdebug.c \
           $(CHIBIOS)/chibios/rt/src/chtrace.c \
           $(CHIBIOS)/chibios/rt/src/chvt.c \
           $(CHIBIOS)/chibios/rt/src/chschd.c \
           $(CHIBIOS)/chibios/rt/src/chthreads.c \
           $(CHIBIOS)/chibios/rt/src/chtm.c \
           $(CHIBIOS)/chibios/rt/src/chstats.c \
           $(CHIBIOS)/chibios/rt/src/chregistry.c \
           $(CHIBIOS)/chibios/rt/src/chsem.c \
           $(CHIBIOS)/chibios/rt/src/chmtx.c \
           $(CHIBIOS)/chibios/rt/src/chcond.c \
           $(CHIBIOS)/chibios/rt/src/chevents.c \
           $(CHIBIOS)/chibios/rt/src/chmsg.c \
           $(CHIBIOS)/chibios/rt/src/chdynamic.c \
           $(CHIBIOS)/chibios/common/oslib/src/chmboxes.c \
           $(CHIBIOS)/chibios/common/oslib/src/chmemcore.c \
           $(CHIBIOS)/chibios/common/oslib/src/chheap.c \
           $(CHIBIOS)/chibios/common/oslib/src/chmempools.c
endif

# Required include directories
KERNINC := $(CHIBIOS)/chibios/rt/include \
           $(CHIBIOS)/chibios/common/oslib/include
