# C++ template (COMPILER=(gcc or clang), CC=(executable name/path, e.g. g++ or clang++))
COMPILER := clang
CC := clang++
CPP_STANDARD := c++2a
EXTRA_CXXFLAGS := -DLOCAL
LIBS := -lfmt

export GIT_ROOT := $(shell run get-root)
export CXX := $(CC) -std=$(CPP_STANDARD) -pipe -pthread -pedantic -march=native

include $(GIT_ROOT)/common/cxxwarns.make
GCH_ROOT := $(GIT_ROOT)/.precompiled-headers

export USE_CLANG_LIBCPP := -stdlib=libc++
export USE_GLIBS_DEBUG := -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
export USE_GCH := -I$(GCH_ROOT)

export INCLUDES := -I$(GIT_ROOT)/code
export CXXFLAGS := $(WARNS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(LIBS) $(INCLUDES)
export OPTIM := -g -O3 -flto -funroll-loops -ftree-vectorize
export DEBUG := -g -Og -ggdb $(USE_GLIBS_DEBUG)

.PHONY: debug perfm clean dry-debug dry-perfm

debug: MODE := debug
debug: CXXFLAGS += $(DEBUG)
debug: ./solver

perfm: MODE := perfm
perfm: CXXFLAGS += $(OPTIM)
perfm: ./solver

clean::
	@rm -f solver hacker judger core vgcore.* *.log output.txt answer.txt

./solver: code.cpp
	@echo CC ${COMPILER} ${MODE} $@
	@$(CXX) code.cpp $(CXXFLAGS) -o $@

./hacker: hack.cpp
	@echo CC ${COMPILER} $@
	@$(CXX) hack.cpp $(CXXFLAGS) $(OPTIM) -o $@

./judger: judge.cpp
	@echo CC ${COMPILER} $@
	@$(CXX) judge.cpp $(CXXFLAGS) $(OPTIM) -o $@
