CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC
AR = ar rcs

SRC_DIR = src
MATRIX_DIR = $(SRC_DIR)/matrix

SOURCES = $(MATRIX_DIR)/s21_matrix_oop.cpp
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,build/%.o,$(SOURCES))

TARGET = libs21_matrix.a
TEST_DIR = tests
REPORT_DIR = report

INCLUDES = -I$(SRC_DIR) -I$(MATRIX_DIR)

.PHONY: all clean test gcov_report style format cppcheck install uninstall dvi dist

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) $@ $^

build/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Тестирование
test:
	@$(MAKE) -C $(TEST_DIR) test \
		SRC_DIR=$(abspath $(SRC_DIR)) \
		MATRIX_DIR=$(abspath $(MATRIX_DIR))

gcov_report:
	@$(MAKE) -C $(TEST_DIR) gcov \
		SRC_DIR=$(abspath $(SRC_DIR)) \
		MATRIX_DIR=$(abspath $(MATRIX_DIR)) \
		REPORT_DIR=$(abspath $(REPORT_DIR))

# Стиль (можно временно закомментировать, так как Qt-файлов нет)
STYLE_FILES = $(shell find $(SRC_DIR) $(TEST_DIR) -type f \( -name '*.cpp' -o -name '*.h' \) -not -path '*/build/*')

style:
	@clang-format -n --style=Google $(STYLE_FILES)

format:
	@clang-format -i --style=Google $(STYLE_FILES)

cppcheck:
	@cppcheck --enable=all --suppress=missingIncludeSystem --error-exitcode=1 \
	          -I$(SRC_DIR) -I$(MATRIX_DIR) $(STYLE_FILES)

# Установка
PREFIX = /usr/local
INSTALL_LIB = $(PREFIX)/lib
INSTALL_INCLUDE = $(PREFIX)/include/s21

install: $(TARGET)
	install -d $(INSTALL_LIB) $(INSTALL_INCLUDE)
	install -m 644 $(TARGET) $(INSTALL_LIB)/
	install -m 644 $(MATRIX_DIR)/s21_matrix_oop.h $(INSTALL_INCLUDE)/

uninstall:
	rm -f $(INSTALL_LIB)/$(TARGET)
	rm -f $(INSTALL_INCLUDE)/s21_matrix_oop.h

# Документация (заглушка, позже добавим Doxyfile)
dvi:
	@echo "Documentation not configured yet."

# Дистрибутив
DIST_NAME = MLP_Matrix
dist:
	rm -rf $(DIST_NAME)
	mkdir $(DIST_NAME)
	cp -r Makefile src tests $(DIST_NAME)/
	tar -czf $(DIST_NAME).tar.gz $(DIST_NAME)
	rm -rf $(DIST_NAME)

clean:
	rm -rf build $(TARGET) $(REPORT_DIR)
	@$(MAKE) -C $(TEST_DIR) clean
