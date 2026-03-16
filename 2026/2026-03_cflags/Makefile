CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Werror -pedantic -Wextra -fno-rtti -fno-exceptions
BUILD    = .build

all: $(BUILD)/flagsc

$(BUILD)/flagsc: $(BUILD)/dsl.o $(BUILD)/codegen.o $(BUILD)/flagsc.o | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/dsl.o: src/dsl.cc src/dsl.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

$(BUILD)/codegen.o: src/codegen.cc src/codegen.h src/dsl.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

$(BUILD)/flagsc.o: cmd/flagsc.cc src/dsl.h src/codegen.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

# ── integration test ──────────────────────────────────────────────────────────
# 1. Generate test_flags.h/.cc from test.flags
# 2. Compile test_driver against the generated files
# 3. Run three scenarios and assert expected output / exit codes

TEST_BUILD = $(BUILD)/test

test: $(BUILD)/flagsc
	mkdir -p $(TEST_BUILD)
	$(BUILD)/flagsc test.flags $(TEST_BUILD)/test_flags
	$(CXX) $(CXXFLAGS) -I$(TEST_BUILD) -o $(TEST_BUILD)/test_driver \
	    test_driver.cc $(TEST_BUILD)/test_flags.cc
	@echo "--- test 1: normal run ---"
	$(TEST_BUILD)/test_driver --output /tmp/result --count 5 --threshold 100000
	@echo "--- test 2: missing required flag (expect error) ---"
	$(TEST_BUILD)/test_driver --count 5 ; test $$? -ne 0
	@echo "--- test 3: invalid int (expect error) ---"
	$(TEST_BUILD)/test_driver --output /tmp --count abc ; test $$? -ne 0
	@echo "--- all tests passed ---"

clean:
	rm -rf $(BUILD)

fmt:
	~/Workspace/y/tools/scripts/clang_format_all.sh .
