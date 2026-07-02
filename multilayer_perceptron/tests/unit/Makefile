# ------------------------------------------------------------
# Makefile для тестов Multilayer Perceptron (этап матрицы)
# Содержит unit-тесты библиотеки и цели для покрытия кода
# ------------------------------------------------------------

LIBMLP_DIR = ../../libmlp
INCLUDES = -I$(LIBMLP_DIR)/src -I$(LIBMLP_DIR)/include/mlp
SRC_DIR = $(LIBMLP_DIR)/src
REPORT_DIR = ../../reporti

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC
GCOV_FLAGS = -fprofile-arcs -ftest-coverage
LDFLAGS = -lgtest -lgtest_main -lpthread -lgcov

OBJ_DIR = obj

LIB_SRCS = $(SRC_DIR)/matrix/s21_matrix_oop.cpp
TEST_SRCS = $(wildcard *.cpp)

LIB_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/lib_%.o,$(LIB_SRCS))
TEST_OBJS = $(patsubst %.cpp,$(OBJ_DIR)/test_%.o,$(TEST_SRCS))

TEST_EXEC = test_runner

.PHONY: all test gcov valgrind clean

all: test

test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/lib_%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

gcov: test
	lcov --capture --directory $(OBJ_DIR) --output-file coverage.info --rc geninfo_auto_base=1
	lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_final.info
	genhtml coverage_final.info --output-directory $(REPORT_DIR)
	rm -f coverage.info coverage_final.info

valgrind: $(TEST_EXEC)
	valgrind --tool=memcheck --leak-check=full --log-file=valgrind.log ./$(TEST_EXEC)

clean:
	rm -rf $(OBJ_DIR) $(TEST_EXEC) *.gcda *.gcno *.gcov valgrind.log coverage.* $(REPORT_DIR)/*
