# Declare all our compilation variables and sourc/object files.
SRC_DIR     := ./src
OBJ_DIR     := ./obj
BIN_DIR     := ./bin
INSTALL_DIR := /usr/local/include
SRC_FILES   := $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES  := $(wildcard $(SRC_DIR)/Tests/*.cpp)
OBJ_FILES   := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(patsubst $(SRC_DIR)/Tests/%.cpp,$(OBJ_DIR)/Tests/%.o,$(TEST_FILES))
LDFLAGS     := -O3 -g -pthread
CPPFLAGS    := -O3 -g
CXXFLAGS    := -std=c++17 -W -Wall -Wno-unused-parameter

# Now actually define all our targets.
$(BIN_DIR)/Main.out: $(OBJ_FILES)
	g++ $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -Rf $(OBJ_DIR)/*
	rm -f $(BIN_DIR)/Main.out
	mkdir $(OBJ_DIR)/Tests

install:
	echo "Installing into $(INSTALL_DIR)..."
	sudo rm -Rf $(INSTALL_DIR)/DispatchCPP
	sudo cp -r $(SRC_DIR)/DispatchCPP $(INSTALL_DIR)/
