CXX      := g++
CXXFLAGS := -std=c++23 -O3 -march=native -flto -Wall -Wextra -Wpedantic

SRC_DIR  := src
APP_DIR  := apps
INC_DIR  := include
BUILD_DIR := build
OBJ_DIR  := $(BUILD_DIR)/obj

CXXFLAGS += -I$(INC_DIR)

EXCHANGE_TARGET := $(BUILD_DIR)/exchange_simulator
CONSUMER_TARGET := $(BUILD_DIR)/market_data_consumer

EXCHANGE_SRC := $(APP_DIR)/exchange_simulator.cpp
CONSUMER_SRC := $(APP_DIR)/market_data_consumer.cpp

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

EXCHANGE_OBJ := $(EXCHANGE_SRC:$(APP_DIR)/%.cpp=$(OBJ_DIR)/apps/%.o)
CONSUMER_OBJ := $(CONSUMER_SRC:$(APP_DIR)/%.cpp=$(OBJ_DIR)/apps/%.o)


.PHONY: all clean exchange consumer

all: $(EXCHANGE_TARGET) $(CONSUMER_TARGET)


exchange: $(EXCHANGE_TARGET)

consumer: $(CONSUMER_TARGET)


$(EXCHANGE_TARGET): $(OBJS) $(EXCHANGE_OBJ)
	@echo "Linking binary: $@"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Run with: ./$(EXCHANGE_TARGET)"


$(CONSUMER_TARGET): $(OBJS) $(CONSUMER_OBJ)
	@echo "Linking binary: $@"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Run with: ./$(CONSUMER_TARGET)"


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@


$(OBJ_DIR)/apps/%.o: $(APP_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
	@echo "Cleaning up build artifacts..."
	@rm -rf $(BUILD_DIR)