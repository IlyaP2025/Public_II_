# ------------------------------------------------------------
# Makefile для тестов Multilayer Perceptron (этап матрицы)
# ------------------------------------------------------------

# Пути передаются из корневого Makefile
SRC_DIR ?= ../src
MATRIX_DIR ?= $(SRC_DIR)/matrix
REPORT_DIR ?= ../report

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC
GCOV_FLAGS = -fprofile-arcs -ftest-coverage
LDFLAGS_TEST = -lgtest -lgtest_main -lpthread -lgcov

INCLUDES = -I$(SRC_DIR) -I$(MATRIX_DIR)

OBJ_DIR = ./obj

# Только нужные исходники библиотеки
LIB_SRCS = $(MATRIX_DIR)/s21_matrix_oop.cpp

# Все тестовые файлы
TEST_SRCS = $(wildcard test_*.cpp)

# Объектные файлы
LIB_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/lib_%.o,$(LIB_SRCS))
TEST_OBJS = $(patsubst test_%.cpp,$(OBJ_DIR)/test_%.o,$(TEST_SRCS))

TEST_EXEC = test_runner

.PHONY: all test gcov valgrind clean

all: test

test: $(TEST_EXEC)
	@./$(TEST_EXEC)

$(TEST_EXEC): $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS_TEST)

# Создание директории obj
$(OBJ_DIR):
	mkdir -p $@

# Правила компиляции библиотечных объектных файлов
$(OBJ_DIR)/lib_%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

# Правила компиляции тестов
$(OBJ_DIR)/test_%.o: test_%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

# Генерация отчёта покрытия
gcov: test
	lcov --capture --directory $(OBJ_DIR) --output-file coverage.info --rc geninfo_auto_base=1
	lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_final.info
	genhtml coverage_final.info --output-directory $(REPORT_DIR)
	rm -f coverage.info coverage_final.info

valgrind: $(TEST_EXEC)
	valgrind --tool=memcheck --leak-check=full --log-file=valgrind.log ./$(TEST_EXEC)

clean:
	rm -rf $(OBJ_DIR) $(TEST_EXEC) *.gcda *.gcno *.gcov valgrind.log coverage.* $(REPORT_DIR)
