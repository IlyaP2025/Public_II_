# ------------------------------------------------------------
# Makefile для тестов 3DViewer v2.0
# ------------------------------------------------------------

# Пути передаются из корневого Makefile (абсолютные)
SRC_DIR ?= ../src
REPORT_DIR ?= ../report
QT_LIBS ?= -lQt6Core -lQt6Gui -lQt6Widgets -lQt6OpenGL -lQt6OpenGLWidgets -lGL

# Компиляторы и флаги
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC
GCOV_FLAGS = -fprofile-arcs -ftest-coverage
LDFLAGS_TEST = -lgtest -lgtest_main -lpthread -lgcov

# MOC
MOC = /usr/lib/qt6/libexec/moc

# Флаги include (Qt требуется для некоторых заголовков, даже если тесты их не используют)
QT_CFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6OpenGL Qt6OpenGLWidgets)
INCLUDES = -I$(SRC_DIR) -I$(SRC_DIR)/matrix $(QT_CFLAGS) -DQT_OPENGLWIDGETS_LIB -DQT_OPENGL_LIB -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB

# Директория для объектных файлов
OBJ_DIR = ./obj
MOC_DIR = ./moc

# Исходные файлы библиотек (все модули, необходимые для тестов)
LIB_SRCS = \
    $(SRC_DIR)/common/transform.cpp \
    $(SRC_DIR)/matrix/s21_matrix_oop.cpp \
    $(SRC_DIR)/scene/mesh.cpp \
    $(SRC_DIR)/scene/scene.cpp \
    $(SRC_DIR)/scene/camera.cpp \
    $(SRC_DIR)/scene/model_builder.cpp \
    $(SRC_DIR)/scene/light_manager.cpp \
    $(SRC_DIR)/scene/kd_tree_index.cpp \
    $(SRC_DIR)/loaders/obj_loader.cpp \
    $(SRC_DIR)/loaders/loader_factory.cpp \
    $(SRC_DIR)/settings/settings.cpp

# Тестовые файлы (все test_*.cpp в текущей папке)
TEST_SRCS = $(wildcard test_*.cpp)

# MOC-файлы (нужны для Settings, т.к. он QObject)
MOC_SRCS = $(MOC_DIR)/moc_settings.cpp
MOC_OBJS = $(MOC_DIR)/moc_settings.o

# Объектные файлы (с поддиректориями)
LIB_OBJS = $(patsubst $(SRC_DIR)/common/%.cpp,$(OBJ_DIR)/lib_common/%.o,$(filter $(SRC_DIR)/common/%,$(LIB_SRCS))) \
           $(patsubst $(SRC_DIR)/matrix/%.cpp,$(OBJ_DIR)/lib_matrix/%.o,$(filter $(SRC_DIR)/matrix/%,$(LIB_SRCS))) \
           $(patsubst $(SRC_DIR)/scene/%.cpp,$(OBJ_DIR)/lib_scene/%.o,$(filter $(SRC_DIR)/scene/%,$(LIB_SRCS))) \
           $(patsubst $(SRC_DIR)/loaders/%.cpp,$(OBJ_DIR)/lib_loaders/%.o,$(filter $(SRC_DIR)/loaders/%,$(LIB_SRCS))) \
           $(patsubst $(SRC_DIR)/settings/%.cpp,$(OBJ_DIR)/lib_settings/%.o,$(filter $(SRC_DIR)/settings/%,$(LIB_SRCS)))

TEST_OBJS = $(patsubst test_%.cpp,$(OBJ_DIR)/test_%.o,$(TEST_SRCS))

# Исполняемый файл
TEST_EXEC = test_runner

.PHONY: all test gcov valgrind clean

all: test

test: $(TEST_EXEC)
	@./$(TEST_EXEC)

$(TEST_EXEC): $(LIB_OBJS) $(TEST_OBJS) $(MOC_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS_TEST) $(QT_LIBS)

# Правила для создания поддиректорий в OBJ_DIR
$(OBJ_DIR)/lib_common $(OBJ_DIR)/lib_matrix $(OBJ_DIR)/lib_scene $(OBJ_DIR)/lib_loaders $(OBJ_DIR)/lib_settings $(MOC_DIR):
	mkdir -p $@

# Правила компиляции библиотек
$(OBJ_DIR)/lib_common/%.o: $(SRC_DIR)/common/%.cpp | $(OBJ_DIR)/lib_common
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/lib_matrix/%.o: $(SRC_DIR)/matrix/%.cpp | $(OBJ_DIR)/lib_matrix
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/lib_scene/%.o: $(SRC_DIR)/scene/%.cpp | $(OBJ_DIR)/lib_scene
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/lib_loaders/%.o: $(SRC_DIR)/loaders/%.cpp | $(OBJ_DIR)/lib_loaders
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/lib_settings/%.o: $(SRC_DIR)/settings/%.cpp | $(OBJ_DIR)/lib_settings
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

# Правила компиляции тестов
$(OBJ_DIR)/test_%.o: test_%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

# Генерация MOC-файла для Settings
$(MOC_DIR)/moc_settings.cpp: $(SRC_DIR)/settings/settings.h | $(MOC_DIR)
	$(MOC) $< -o $@

# Компиляция MOC-файла
$(MOC_DIR)/moc_settings.o: $(MOC_DIR)/moc_settings.cpp
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $(INCLUDES) -c $< -o $@

# Генерация отчёта о покрытии
gcov: test
	@echo "Генерация отчёта покрытия..."
	@lcov --capture --directory $(OBJ_DIR) --output-file coverage.info --rc geninfo_auto_base=1 --ignore-errors mismatch
	@lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_final.info
	@genhtml coverage_final.info --output-directory $(REPORT_DIR)
	@rm -f coverage.info coverage_final.info
	@echo "Отчёт готов: $(REPORT_DIR)/index.html"

# Проверка утечек памяти
valgrind: $(TEST_EXEC)
	@valgrind --tool=memcheck --leak-check=full --log-file=valgrind.log ./$(TEST_EXEC)
	@echo "Результаты valgrind в valgrind.log"

clean:
	rm -rf $(OBJ_DIR) $(MOC_DIR) $(TEST_EXEC) *.gcda *.gcno *.gcov valgrind.log coverage.* $(REPORT_DIR)
