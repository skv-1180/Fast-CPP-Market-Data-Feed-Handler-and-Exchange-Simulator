CXX      := g++
CXXFLAGS := -std=c++20 -O3 -march=native -flto -Wall -Wextra -Wpedantic

SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj

CXXFLAGS += -I$(INC_DIR)

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

TARGET := trading_engine

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking binary: $@"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)
	@echo "Build successful! Run with: ./$(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "Compiling: $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "Cleaning up build artifacts..."
	@rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
