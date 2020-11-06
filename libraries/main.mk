# This is the main makefile of hila applications
#  - to be called in application makefiles 
#  - calls platform specific makefiles in directory "platforms" 
#

# If "make clean", don't worry about targets, platforms and options
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)

# PLATFORM needs to be defined. Check.
ifndef PLATFORM
  $(error "make <goal> PLATFORM=<name>" required.  See directory "platforms" at top level)
endif

# allow only 0 or 1 goals in make
ifneq ($(words $(MAKECMDGOALS)), 1)
ifneq ($(words $(MAKECMDGOALS)), 0)
  $(error Use (at most) one goal in make)
endif
endif

.PRECIOUS: build/%.cpt build/%.o


################

LIBRARIES_DIR := $(TOP_DIR)/libraries
PLATFORM_DIR := $(LIBRARIES_DIR)/platforms
THESE_MAKEFILES := $(LIBRARIES_DIR)/main.mk $(PLATFORM_DIR)/$(PLATFORM).mk
HILA_INCLUDE_DIR := $(TOP_DIR)/libraries

# Read in the appropriate platform bits
include $(PLATFORM_DIR)/$(PLATFORM).mk

# To force a full remake when changing platforms or targets
LASTMAKE := build/lastmake.${MAKECMDGOALS}.${PLATFORM}

$(LASTMAKE): $(MAKEFILE_LIST)
	-rm -f build/lastmake.*
	make clean
	touch ${LASTMAKE}

	
HILA_OBJECTS = \
  build/inputs.o \
  build/mersenne_inline.o \
  build/lattice.o \
  build/setup_layout_vector.o \
  build/map_node_layout_trivial.o \
  build/com_mpi.o \
  build/memalloc.o \
  build/test_gathers.o


# Use all headers inside libraries for dependencies
HILA_HEADERS := $(wildcard $(TOP_DIR)/libraries/*/*.h) $(wildcard $(TOP_DIR)/libraries/*/*/*.h)

ALL_DEPEND := $(LASTMAKE) $(HILA_HEADERS)

HILAPP = $(TOP_DIR)/hilapp/build/hilapp $(HILAPP_OPTS)
HILA_OPTS += -I$(HILA_INCLUDE_DIR)

# Standard rules for creating and building cpt files. These
# build .o files in the build folder by first running them
# through the 

endif
endif   # close the "clean" bracket


build/%.cpt: %.cpp Makefile $(THIS_MAKEFILE) $(ALL_DEPEND) $(HEADERS)
	mkdir -p build
	$(HILAPP) $(HILA_OPTS) $(OPTS) $< -o $@

build/%.o : build/%.cpt
	$(CC) $(CXXFLAGS) $(OPTS) $(HILA_OPTS) $< -c -o $@

build/%.cpt: $(LIBRARIES_DIR)/plumbing/%.cpp $(ALL_DEPEND) $(HILA_HEADERS)
	mkdir -p build
	$(HILAPP) $(HILA_OPTS) $< -o $@


clean:
	rm -f build/*.o build/*.cpt build/lastmake*
	
cleanall:
	rm -f build/*
