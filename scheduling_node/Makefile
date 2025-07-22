CXX = g++

HOME := ${HOME}
# TO BE DEFINED BY USER
PROJECT_DIR := 
SYSTEMC_HOME := $(SYSTEMC_HOME)
AC_SIMUTILS_HOME := $(AC_SIMUTILS_HOME)
CONNECTIONS_HOME := $(CONNECTIONS_HOME)
AC_TYPES_HOME := $(AC_TYPES_HOME)

INCDIR ?=
INCDIRS = -isystem $(SYSTEMC_HOME)/include
INCDIRS += -isystem $(SYSTEMC_HOME)/src/
INCDIRS += -isystem $(AC_SIMUTILS_HOME)/include/
INCDIRS += -isystem $(CONNECTIONS_HOME)/include/
INCDIRS += -isystem $(AC_TYPES_HOME)/include/
INCDIRS += -isystem $(PROJECT_DIR)/src/

LIBDIR = -L. -L$(SYSTEMC_HOME)/lib -Wl,-rpath=$(SYSTEMC_HOME)/lib

CFLAGS =   -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-label $(INCDIRS) $(LIBDIR)
USER_FLAGS = -DCONNECTIONS_ACCURATE_SIM -DSC_INCLUDE_DYNAMIC_PROCESSES -DCONNECTIONS_NAMING_ORIGINAL

# RAND_STALL
# 0 = Random stall of ports and channels disabled (default)
# 1 = Random stall of ports and channels enabled
#
# This feature aids in latency insensitive design verication.
# Note: Only valid if SIM_MODE = 1 (accurate) or 2 (fast)
ifeq ($(RAND_STALL),1)
	USER_FLAGS += -DCONN_RAND_STALL
endif

LIBS = -lsystemc


.PHONY: Build
Build: all

CFLAGS += -O0 -g -std=c++11 

all: sim_sc

build: sim_sc

run:
	./sim_sc

# Choose processor version: caches, core, or floating_point
PROC_VER ?= core

SRC_DIR = $(PROC_VER)/src

sim_sc: $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.h)
	$(CXX) -o sim_sc $(CFLAGS) $(USER_FLAGS) $(wildcard $(SRC_DIR)/*.cpp) $(LIBS)

clean:
	rm -f sim_sc

