# directory to store object files
OBJS_DIR = .objs

# define the EXES
EXE_PARSE_TEST=parse_test
OBJS_PARSE_TEST=$(EXE_PARSE_TEST).o parse.o state.o test.o

EXE_STATE_TEST=state_test
OBJS_STATE_TEST=$(EXE_STATE_TEST).o state.o test.o

EXES_TEST=$(EXE_PARSE_TEST) $(EXE_STATE_TEST)

# set up compiler
CC = clang
INCLUDES=-I./includes/
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(INCLUDES) $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(INCLUDES) $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS =

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: release
.PHONY: debug

release: $(EXES_TEST)
debug: clean $(EXES_TEST:%=%-debug)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
$(EXE_SHELL)-debug: $(OBJS_SHELL:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SHELL): $(OBJS_SHELL:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_PARSE_TEST)-debug: $(OBJS_PARSE_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_PARSE_TEST): $(OBJS_PARSE_TEST:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_STATE_TEST)-debug: $(OBJS_STATE_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_STATE_TEST): $(OBJS_STATE_TEST:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf .objs $(EXES_TEST) $(EXES_TEST:%=%-debug)
