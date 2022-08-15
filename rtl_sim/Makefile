# Include path to Boost area on local machine
#
#worked with both CENTOS and Ubuntu: export BOOST    := /usr/
export BOOST    := /usr/
#export BOOST    := /sarc/spa/tools/conda/envs/spa-1

VERILATOR_OPTS =
VERILATOR_OPTS += --exe
VERILATOR_OPTS += --cc
VERILATOR_OPTS += -I../src
VERILATOR_OPTS += -I../src/fpnew/src/common_cells/include
VERILATOR_OPTS += -I../tb
VERILATOR_OPTS += -I${VERILATOR_HOME}/include/vltstd

ifeq ($(LINT),on)
	VERILATOR_OPTS += --lint-only
else
	VERILATOR_OPTS += --build
endif

VERILATOR_OPTS += --trace-fst
VERILATOR_OPTS += --trace-params
VERILATOR_OPTS += --trace-structs
VERILATOR_OPTS += --trace-underscore

VERILATOR_OPTS += --unroll-count 1024
VERILATOR_OPTS += -j $(NPROCS)
VERILATOR_OPTS += +define+SIMULATION
VERILATOR_OPTS += --compiler clang
VERILATOR_OPTS += -Wno-BLKANDNBLK
# VERILATOR_OPTS += -Wno-fatal
VERILATOR_OPTS += -Wno-LITENDIAN
VERILATOR_OPTS += -Wno-WIDTH

VERILATOR_OPTS += -LDFLAGS -lboost_iostreams
VERILATOR_OPTS += -LDFLAGS "-L$(BOOST)/lib -Wl,-rpath $(BOOST)/lib"

TOP = predictor
# PREDICTOR_RTL = rtl/bimodal.sv
# PREDICTOR_SRC = ../sim/predictor_bimodal.cc

PREDICTOR_RTL = rtl/gshare.sv


ifndef BOOST
$(error "You must define BOOST")
endif


export CBP_BASE := $(CURDIR)/..
CXX := g++

OBJDIR := $(CBP_BASE)/bin
SRC    := $(CBP_BASE)/sim
VPATH  := $(SRC)

LDLIBS   += -lboost_iostreams

LDFLAGS += -L$(BOOST)/lib -Wl,-rpath $(BOOST)/lib

CPPFLAGS := -O3 -Wall -std=c++11 -Wextra -Winline -Winit-self -Wno-sequence-point\
           -Wno-unused-function -Wno-inline -fPIC -W -Wcast-qual -Wpointer-arith -Woverloaded-virtual\
           -I$(CBP_BASE) -I/usr/include -I/user/include/boost/ -I/usr/include/boost/iostreams/ -I/usr/include/boost/iostreams/device/

PROGRAMS := sim_predictor

objects = main.o

all: $(PROGRAMS)

# predictor : $(objects)
# 	$(CXX) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

sim_predictor: rtl_main.cc $(PREDICTOR_RTL) $(PREDICTOR_SRC)
	verilator --top-module $(TOP) \
		-o ../sim_predictor \
		$(VERILATOR_OPTS) \
		$(PREDICTOR_RTL) $(PREDICTOR_SRC) \
		rtl_main.cc

dbg: clean
	$(MAKE) DBG_BUILD=1 all

clean:
	rm -rf $(PROGRAMS) $(objects) simx.fst* obj_dir
