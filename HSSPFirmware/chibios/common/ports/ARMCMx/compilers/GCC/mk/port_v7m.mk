# List of the ChibiOS/RT ARMv7M generic port files.
PORTSRC = $(CHIBIOS)/chibios/common/ports/ARMCMx/chcore.c \
          $(CHIBIOS)/chibios/common/ports/ARMCMx/chcore_v7m.c
          
PORTASM = $(CHIBIOS)/chibios/common/ports/ARMCMx/compilers/GCC/chcoreasm_v7m.S

PORTINC = $(CHIBIOS)/chibios/common/ports/ARMCMx \
          $(CHIBIOS)/chibios/common/ports/ARMCMx/compilers/GCC
