# Define default targets in a way that's compatible w/make v3.80 and before.
.PHONY: default
default: optimized ;

# Declare all our compilation variables and sourc/object files.
SRC_DIR     := ./src
OBJ_DIR     := ./obj
BIN_DIR     := ./bin
INSTALL_DIR := /usr/local/include
SRC_FILES   := $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES  := $(wildcard $(SRC_DIR)/Tests/*.cpp)
OBJ_FILES   := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(patsubst $(SRC_DIR)/Tests/%.cpp,$(OBJ_DIR)/Tests/%.o,$(TEST_FILES))
OPTFLAGS    := -O3
LDFLAGS     := -g -pthread
CPPFLAGS    := -g
CXXFLAGS    := -std=c++17 -W -Wall -Wno-unused-parameter -Wno-unused-function -I$(SRC_DIR)

#$(BIN_DIR)/Main.out: $(OBJ_FILES)
#	g++ $(LDFLAGS) $(OPTFLAGS) -o $@ $^
#$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
#	g++ $(CPPFLAGS) $(OPTFLAGS) $(CXXFLAGS) -c -o $@ $<

# The base-level target which defines Main.out reliant upon all obj files we've specified.
$(BIN_DIR)/Main-O3.out: clean set-optimized | $(OBJ_FILES)
	g++ $(LDFLAGS) $(OPTFLAGS) -o $@ $|

$(BIN_DIR)/Main-O0.out: clean set-not-optimized | $(OBJ_FILES)
	g++ $(LDFLAGS) $(OPTFLAGS) -o $@ $|

# The next base-level target which defines how all object files are compiled.
$(OBJ_DIR)/%.o: clean | $(SRC_DIR)/%.cpp
	g++ $(CPPFLAGS) $(OPTFLAGS) $(CXXFLAGS) -c -o $@ $|

# Targets for setting the optimization level.
set-optimized: clean
	$(eval OPTFLAGS=-O3)
set-not-optimized: clean
	$(eval OPTFLAGS=-O0)

# Target for building Main.out with optimizations enabled.
optimized: clean set-optimized $(BIN_DIR)/Main-O3.out

# Target for building Main.out with optimizations disabled.
not-optimized: clean set-not-optimized $(BIN_DIR)/Main-O0.out

clean:
	rm -Rf $(OBJ_DIR)/*
	rm -f $(BIN_DIR)/Main*
	mkdir $(OBJ_DIR)/Tests

install:
	echo "Installing into $(INSTALL_DIR)..."
	sudo rm -Rf $(INSTALL_DIR)/DispatchCPP
	sudo cp -r $(SRC_DIR)/DispatchCPP $(INSTALL_DIR)/

uninstall:
	echo "Uninstalling from $(INSTALL_DIR)..."
	sudo rm -Rf $(INSTALL_DIR)/DispatchCPP
