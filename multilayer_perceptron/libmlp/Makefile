# ------------------------------------------------------------
# Makefile для сборки библиотеки libmlp.a
# Компилирует все модули ядра (матрицы, активации, перцептроны и т.д.)
# ------------------------------------------------------------

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC
AR = ar rcs

INCLUDE_DIR = include/mlp
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/matrix/s21_matrix_oop.cpp \
          $(SRC_DIR)/perceptron/matrix_perceptron.cpp \
          $(SRC_DIR)/data/emnist_loader.cpp \
          $(SRC_DIR)/trainer/simple_trainer.cpp \
          $(SRC_DIR)/data/bmp_loader.cpp \
          $(SRC_DIR)/serialization/simple_weights_serializer.cpp \
          $(SRC_DIR)/metrics/metrics_calculator.cpp

OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET = libmlp.a

INCLUDES = -I$(INCLUDE_DIR)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
