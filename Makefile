CXX = clang++
CXXFLAGS = -std=c++23 #-Wall -Wextra -Werror -pedantic
TARGET = main
TEST_TARGET = test
BUILD_DIR = build
SOURCES = main.cpp glib.cpp
TEST_SOURCES = test.cpp glib.cpp
HEADERS = glib.h
OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.cpp=.o))
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/, $(TEST_SOURCES:.cpp=.o))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJECTS)
	./$(TEST_TARGET)
	$(MAKE) clean

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

.PHONY: all clean test