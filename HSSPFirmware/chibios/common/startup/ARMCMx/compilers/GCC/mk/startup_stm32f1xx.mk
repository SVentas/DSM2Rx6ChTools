# List of the ChibiOS generic STM32F1xx startup and CMSIS files.
STARTUPSRC = $(CHIBIOS)/chibios/common/startup/ARMCMx/compilers/GCC/crt1.c \
             $(CHIBIOS)/chibios/common/startup/ARMCMx/compilers/GCC/vectors.c
          
STARTUPASM = $(CHIBIOS)/chibios/common/startup/ARMCMx/compilers/GCC/crt0_v7m.S

STARTUPINC = $(CHIBIOS)/chibios/common/startup/ARMCMx/compilers/GCC \
             $(CHIBIOS)/chibios/common/startup/ARMCMx/devices/STM32F1xx \
             $(CHIBIOS)/chibios/common/ext/CMSIS/include \
             $(CHIBIOS)/chibios/common/ext/CMSIS/ST/STM32F1xx

STARTUPLD  = $(CHIBIOS)/chibios/common/startup/ARMCMx/compilers/GCC/ld
