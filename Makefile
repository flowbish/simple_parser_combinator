# directory to store object files
BUILD_DIR = build
BUILD_DIR_RELEASE = $(BUILD_DIR)/release
BUILD_DIR_DEBUG = $(BUILD_DIR)/debug

# define the EXES
EXE_PARSE_TEST=parse_test
SRC_PARSERS = $(wildcard parser/*.c)
OBJS_PARSERS = $(SRC_PARSERS:%.c=%.o)
OBJS_PARSE_TEST=$(EXE_PARSE_TEST).o assert.o parse.o state.o test.o $(OBJS_PARSERS)

EXE_STATE_TEST=state_test
OBJS_STATE_TEST=$(EXE_STATE_TEST).o state.o test.o

EXE_ISTREAM_TEST=istream_test
OBJS_ISTREAM_TEST=$(EXE_ISTREAM_TEST).o istream.o test.o assert.o

EXES_TEST=$(EXE_PARSE_TEST) $(EXE_STATE_TEST) $(EXE_ISTREAM_TEST)

# set up compiler
CC = clang
INCLUDES=-I. -Iparser
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(INCLUDES) $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -DDEBUG
CFLAGS_RELEASE = -O2 $(INCLUDES) $(WARNINGS) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE

# set up linker
LD = clang
LDFLAGS =

# utilities
MKDIR = mkdir -p

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: release
.PHONY: debug

release: $(EXES_TEST:%=$(BUILD_DIR_RELEASE)/%)
debug: $(EXES_TEST:%=$(BUILD_DIR_DEBUG)/%)

# include dependencies
DEPS = $(wildcard $(BUILD_DIR)/*.d)
-include $(DEPS)

$(BUILD_DIR):
	@$(MKDIR) $(BUILD_DIR)

$(BUILD_DIR_RELEASE): $(BUILD_DIR)
	@$(MKDIR) $(BUILD_DIR_RELEASE)

$(BUILD_DIR_DEBUG): $(BUILD_DIR)
	@$(MKDIR) $(BUILD_DIR_DEBUG)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(BUILD_DIR_DEBUG)/%.o: %.c | $(BUILD_DIR_DEBUG)
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(BUILD_DIR_RELEASE)/%.o: %.c | $(BUILD_DIR_RELEASE)
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
$(BUILD_DIR_DEBUG)/$(EXE_SHELL): $(OBJS_SHELL:%.o=$(BUILD_DIR_DEBUG)/%.o) | $(BUILD_DIR_DEBUG)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_RELEASE)/$(EXE_SHELL): $(OBJS_SHELL:%.o=$(BUILD_DIR_RELEASE)/%.o) | $(BUILD_DIR_RELEASE)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_DEBUG)/$(EXE_PARSE_TEST): $(OBJS_PARSE_TEST:%.o=$(BUILD_DIR_DEBUG)/%.o) | $(BUILD_DIR_DEBUG)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_RELEASE)/$(EXE_PARSE_TEST): $(OBJS_PARSE_TEST:%.o=$(BUILD_DIR_RELEASE)/%.o) | $(BUILD_DIR_RELEASE)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_DEBUG)/$(EXE_STATE_TEST): $(OBJS_STATE_TEST:%.o=$(BUILD_DIR_DEBUG)/%.o) | $(BUILD_DIR_DEBUG)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_RELEASE)/$(EXE_STATE_TEST): $(OBJS_STATE_TEST:%.o=$(BUILD_DIR_RELEASE)/%.o) | $(BUILD_DIR_RELEASE)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_DEBUG)/$(EXE_ISTREAM_TEST): $(OBJS_ISTREAM_TEST:%.o=$(BUILD_DIR_DEBUG)/%.o) | $(BUILD_DIR_DEBUG)
	$(LD) $^ $(LDFLAGS) -o $@

$(BUILD_DIR_RELEASE)/$(EXE_ISTREAM_TEST): $(OBJS_ISTREAM_TEST:%.o=$(BUILD_DIR_RELEASE)/%.o) | $(BUILD_DIR_RELEASE)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
