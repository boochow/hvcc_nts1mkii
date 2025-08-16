##############################################################################
# Configuration for Makefile
#

.DEFAULT_GOAL := all

PROJECT := {{patch_name}}
PROJECT_TYPE := delfx

ifndef HEAP_SIZE
# Estimate the necessary heap size and store it into HEAP_SIZE_FILE
HEAP_SIZE_FILE := logue_heap_size.mk

$(HEAP_SIZE_FILE):
	@if command -v gcc >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1; then \
		$(MAKE) -f Makefile.testmem ; \
		TMPFILE=$$(mktemp) ; \
		./testmem > $$TMPFILE ; \
		HEAP_SIZE_TESTMEM=$$(grep '^total:' $$TMPFILE | awk '{print $$2}') ; \
		echo "HEAP_SIZE := $$HEAP_SIZE_TESTMEM" > $(HEAP_SIZE_FILE) ; \
		SDRAM_SIZE_TESTMEM=$$(grep '^sdram:' $$TMPFILE | awk '{print $$2}') ; \
		echo "SDRAM_SIZE := $$SDRAM_SIZE_TESTMEM" >> $(HEAP_SIZE_FILE) ; \
		rm -f $$TMPFILE ; \
		$(MAKE) -f Makefile.testmem clean ; \
	else \
		echo "HEAP_SIZE := 3072" > $(HEAP_SIZE_FILE) ; \
	fi

include $(HEAP_SIZE_FILE)
endif

##############################################################################
# Sources
#

# C sources 
UCSRC = header.c logue_mem.c {{heavy_files_c}}

UCSRC += _unit_base.c

# C++ sources 
UCXXSRC = logue_heavy.cpp {{heavy_files_cpp}}

# List ASM source files here
UASMSRC = 

UASMXSRC = 

##############################################################################
# Include Paths
#

UINCDIR  = 

##############################################################################
# Library Paths
#

ULIBDIR = 

##############################################################################
# Libraries
#

ULIBS  = -lm
ULIBS  += -lstdc++
ULIBS  += -Wl,--gc-sections

##############################################################################
# Macros
#

UDEFS = -DNDEBUG -DUNIT_HEAP_SIZE=$(HEAP_SIZE) -fvisibility=hidden

ifdef SDRAM_SIZE
UDEFS += -DUNIT_SDRAM_SIZE=$(SDRAM_SIZE)
endif

# Assume Unix-like to suppress warning messages
UDEFS += -U_WIN32 -U_WIN64 -U_MSC_VER -D__unix

# Try disabling this option when the results are inaccurate.
UDEFS += -DLOGUE_FAST_MATH

# Enable this to reduce the processing load
# UDEFS += -DRENDER_HALF

