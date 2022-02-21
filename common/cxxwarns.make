WARNS := -Wall -Wextra -Wpedantic

# Unused code
WARNS += -Wunused -Wredundant-decls -Wextra-semi
WARNS += -Wno-unused-function

# Type casting
WARNS += -Wcast-align -Wcast-qual -Wold-style-cast
WARNS += -Wsign-compare -Wfloat-equal -Wno-double-promotion

WARNS += -Wformat
WARNS += -Woverloaded-virtual -Wnon-virtual-dtor
WARNS += -Wpessimizing-move -Wredundant-move
WARNS += -Wdate-time

# GCC https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
ifeq ($(COMPILER),gcc)
	WARNS += -fmax-errors=5

	WARNS += -Wrestrict
	WARNS += -Wduplicated-cond
	WARNS += -Wduplicated-branches
	WARNS += -Wuseless-cast
	WARNS += -Wlogical-op
	WARNS += -Wshadow=local
	WARNS += -Wformat-signedness
	WARNS += -Wformat-truncation
	WARNS += -Wtrampolines
	WARNS += -Wsuggest-attribute=noreturn
	WARNS += -Wsuggest-final-types
	WARNS += -Wsuggest-final-methods
	WARNS += -Wsuggest-override
endif

# Clang https://clang.llvm.org/docs/DiagnosticsReference.html
ifeq ($(COMPILER),clang)
	WARNS += -ferror-limit=5
	WARNS += -Qunused-arguments
    WARNS += -Wno-float-conversion

	WARNS += -Watomic-implicit-seq-cst
	WARNS += -Wbad-function-cast
	WARNS += -Wc++17-compat
	WARNS += -Wconditional-uninitialized
	WARNS += -Wdeprecated
	WARNS += -Wduplicate-enum
	WARNS += -Wduplicate-method-arg
	WARNS += -Wduplicate-method-match
	WARNS += -Wdynamic-exception-spec
	WARNS += -Wextra-semi-stmt
	WARNS += -Wformat-non-iso
	WARNS += -Wformat-pedantic
	WARNS += -Wgcc-compat
	WARNS += -Wgnu
	# WARNS += -Wheader-hygiene
	WARNS += -Widiomatic-parentheses
	WARNS += -Wimplicit-fallthrough
	WARNS += -Winfinite-recursion
	WARNS += -Wloop-analysis
	WARNS += -Wmethod-signatures
	WARNS += -Wmissing-braces
	WARNS += -Wmissing-field-initializers
	WARNS += -Wmissing-noreturn
	WARNS += -Wnewline-eof
	WARNS += -Wover-aligned
	WARNS += -Wpacked
	WARNS += -Wredundant-parens
	WARNS += -Wreorder
	WARNS += -Wreturn-std-move
	WARNS += -Wself-assign
	WARNS += -Wself-move
	WARNS += -Wshadow-uncaptured-local
	WARNS += -Wshift-sign-overflow
	WARNS += -Wsigned-enum-bitfield
	WARNS += -Wsometimes-uninitialized
	WARNS += -Wstatic-self-init
	WARNS += -Wstrict-prototypes
	WARNS += -Wstring-conversion
	WARNS += -Wtautological-compare
	WARNS += -Wtautological-constant-in-range-compare
	WARNS += -Wthread-safety
	# WARNS += -Wundef
	WARNS += -Wundefined-func-template
	WARNS += -Wundefined-internal-type
	WARNS += -Wunreachable-code
	WARNS += -Wunreachable-code-aggressive
	WARNS += -Wunused-const-variable
	WARNS += -Wunused-exception-parameter
	WARNS += -Wvla
	WARNS += -Wweak-template-vtables
	WARNS += -Wweak-vtables
	WARNS += -Wzero-as-null-pointer-constant
	WARNS += -Wzero-length-array
    WARNS += -Wno-shadow-field-in-constructor
endif

# Not errors
WARNS += -Wno-missing-declarations
WARNS += -Wno-unknown-pragmas
WARNS += -Wno-null-dereference # Too many spurious warnings
