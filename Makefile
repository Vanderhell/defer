BUILD_DIR ?= build
GCC ?= gcc
CLANG ?= clang

CFLAGS_STRICT = -I. -std=c11 -Wall -Wextra -Wpedantic -Werror \
	-Wconversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes \
	-Wcast-function-type -Wstrict-aliasing=2

CFLAGS_C99 = -I. -std=c99 -Wall -Wextra -Wpedantic -Werror \
	-Wconversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes \
	-Wcast-function-type -Wstrict-aliasing=2

SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer

.PHONY: test-gcc test-clang asan ubsan examples clean

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test-gcc: $(BUILD_DIR)
	$(GCC) $(CFLAGS_C99) -pthread -O0 -o $(BUILD_DIR)/test-gcc tests/test_defer.c
	./$(BUILD_DIR)/test-gcc

test-clang: $(BUILD_DIR)
	$(CLANG) $(CFLAGS_C99) -pthread -O0 -o $(BUILD_DIR)/test-clang tests/test_defer.c
	./$(BUILD_DIR)/test-clang

asan: $(BUILD_DIR)
	$(CLANG) $(CFLAGS_C99) $(SAN_FLAGS) -pthread -O0 -o $(BUILD_DIR)/asan tests/test_defer.c
	./$(BUILD_DIR)/asan

ubsan: $(BUILD_DIR)
	$(CLANG) $(CFLAGS_C99) -fsanitize=undefined -pthread -O0 -o $(BUILD_DIR)/ubsan tests/test_defer.c
	./$(BUILD_DIR)/ubsan

examples: $(BUILD_DIR)
	$(CLANG) $(CFLAGS_STRICT) -O0 -o $(BUILD_DIR)/example-memory examples/memory.c
	$(CLANG) $(CFLAGS_STRICT) -O0 -o $(BUILD_DIR)/example-files examples/files.c
	$(CLANG) $(CFLAGS_STRICT) -pthread -O0 -o $(BUILD_DIR)/example-mutex examples/mutex.c

clean:
	$(RM) -r $(BUILD_DIR)
