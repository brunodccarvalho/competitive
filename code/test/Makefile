include ../../common/Makefile
# DEBUG += $(SANIT)

BIN_DIR := ./bin
DEP_DIR := ./dep

HEADERS := $(shell find .. -name '*.hpp' -wholename '*/*.hpp')
CXX_FILES := $(wildcard *.cpp)
CXX_FILES := $(filter-out $(FILE), $(CXX_FILES))
EXE_FILES := $(CXX_FILES:%.cpp=${BIN_DIR}/%)
DEP_FILES := $(CXX_FILES:%.cpp=${DEP_DIR}/%.d)

-include $(DEP_FILES)

# global build

CMD_MKDIRS = @mkdir -p $(BIN_DIR) $(DEP_DIR)
CMD_MV_DEPENDENCY = @mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d && touch $@

DEPCOMPILEARGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.Td

$(DEP_DIR)/%.d: ;
.PRECIOUS: $(DEP_DIR)/%.d

$(EXE_FILES): $(BIN_DIR)/%: %.cpp $(DEP_DIR)/%.d
	@echo "CC (${COMPILER} ${MODE})  $<"
	@$(CMD_MKDIRS)
	@$(CXX) $< $(CXXFLAGS) $(DEPCOMPILEARGS) -o $@
	@$(CMD_MV_DEPENDENCY)

.PHONY: all allperfm

all: MODE := debug
all: CXXFLAGS += $(DEBUG)
all: $(EXE_FILES)

allperfm: MODE := perfm
allperfm: CXXFLAGS += $(OPTIM)
allperfm: $(EXE_FILES)

clean::
	@rm -rf $(DEP_DIR) $(BIN_DIR)

# ---

./solver: $(HEADERS)
