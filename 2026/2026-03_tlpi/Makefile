CXX      = c++
CXXFLAGS = -Wall -Werror -pedantic -Wextra -fno-rtti -fno-exceptions -std=c++17
BUILD    = .build

CMDS = $(BUILD)/chp54_shm $(BUILD)/chp55_daemon_pid $(BUILD)/chp55_file_lock

.PHONY: all clean

all: $(BUILD)/test_file $(CMDS)

$(BUILD)/test_file: | $(BUILD)
	touch $@

$(BUILD)/chp54_shm: cmd/chp54_shm.cc | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $< -lrt

$(BUILD)/chp55_daemon_pid: cmd/chp55_daemon_pid.cc | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD)/chp55_file_lock: cmd/chp55_file_lock.cc | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
