BUILD_DIR ?= build
GCC ?= gcc
CLANG ?= clang

WARNINGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow \
	-Wstrict-prototypes -Wmissing-prototypes -Wcast-function-type \
	-Wstrict-aliasing=2

TEST_FLAGS = -D_POSIX_C_SOURCE=200809L -I. $(WARNINGS)
EXAMPLE_FLAGS = -I. $(WARNINGS)

.PHONY: all test examples clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(BUILD_DIR)
	$(GCC) -std=c99 -O0 $(TEST_FLAGS) -pthread tests/test_defer.c -o $(BUILD_DIR)/test-gcc-c99
	$(GCC) -std=c11 -O0 $(TEST_FLAGS) -pthread tests/test_defer.c -o $(BUILD_DIR)/test-gcc-c11
	$(CLANG) -std=c99 -O0 $(TEST_FLAGS) -pthread tests/test_defer.c -o $(BUILD_DIR)/test-clang-c99
	$(CLANG) -std=c11 -O0 $(TEST_FLAGS) -pthread tests/test_defer.c -o $(BUILD_DIR)/test-clang-c11
	$(GCC) -std=c99 -O0 $(TEST_FLAGS) -pthread tests/multi_tu_main.c tests/multi_tu_a.c tests/multi_tu_b.c -o $(BUILD_DIR)/multi-tu-gcc
	$(CLANG) -std=c99 -O0 $(TEST_FLAGS) -pthread tests/multi_tu_main.c tests/multi_tu_a.c tests/multi_tu_b.c -o $(BUILD_DIR)/multi-tu-clang
	$(GCC) -std=c11 -O0 $(TEST_FLAGS) -pthread tests/threaded.c -o $(BUILD_DIR)/threaded-gcc
	./$(BUILD_DIR)/test-gcc-c99
	./$(BUILD_DIR)/test-gcc-c11
	./$(BUILD_DIR)/test-clang-c99
	./$(BUILD_DIR)/test-clang-c11
	./$(BUILD_DIR)/multi-tu-gcc
	./$(BUILD_DIR)/multi-tu-clang
	./$(BUILD_DIR)/threaded-gcc
	@if $(GCC) -std=c11 -O0 $(TEST_FLAGS) -fsyntax-only tests/compile_fail/incompatible_callback.c; then \
		echo "expected compile failure: incompatible_callback.c"; \
		exit 1; \
	fi
	@if $(CLANG) -std=c11 -O0 $(TEST_FLAGS) -fsyntax-only tests/compile_fail/jump_into_guarded_scope.c; then \
		echo "expected compile failure: jump_into_guarded_scope.c"; \
		exit 1; \
	fi
	@if $(GCC) -std=c11 -O0 $(TEST_FLAGS) -fsyntax-only tests/compile_fail/dismiss_non_guard.c; then \
		echo "expected compile failure: dismiss_non_guard.c"; \
		exit 1; \
	fi

examples: $(BUILD_DIR)
	$(GCC) -std=c11 -O0 $(EXAMPLE_FLAGS) examples/memory.c -o $(BUILD_DIR)/memory
	$(GCC) -std=c11 -O0 $(EXAMPLE_FLAGS) examples/files.c -o $(BUILD_DIR)/files
	$(GCC) -std=c11 -O0 $(EXAMPLE_FLAGS) -pthread examples/mutex.c -o $(BUILD_DIR)/mutex

clean:
	rm -rf $(BUILD_DIR)
