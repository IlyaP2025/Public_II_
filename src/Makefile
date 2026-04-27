CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -g -fPIC
DEBUG_FLAGS = -DDEBUG

QT_CFLAGS = $(shell pkg-config --cflags Qt6Widgets Qt6OpenGL Qt6OpenGLWidgets Qt6Concurrent)
QT_LIBS   = $(shell pkg-config --libs   Qt6Widgets Qt6OpenGL Qt6OpenGLWidgets Qt6Concurrent) -lGL

INCLUDES = -I./src -I./src/matrix -I./src/state_machine $(QT_CFLAGS)

SRC_DIR = src
COMMON_DIR = $(SRC_DIR)/common
MATRIX_DIR = $(SRC_DIR)/matrix
SCENE_DIR = $(SRC_DIR)/scene
LOADERS_DIR = $(SRC_DIR)/loaders
RENDERER_DIR = $(SRC_DIR)/renderer
VIEW_DIR = $(SRC_DIR)/view
COMMANDS_DIR = $(SRC_DIR)/commands
FACADE_DIR = $(SRC_DIR)/facade
SETTINGS_DIR = $(SRC_DIR)/settings
THIRDPARTY_DIR = $(SRC_DIR)/3rd_party
STATE_MACHINE_DIR = $(SRC_DIR)/state_machine

SOURCES = src/main.cpp \
          src/common/transform.cpp \
          src/matrix/s21_matrix_oop.cpp \
          src/scene/mesh.cpp \
          src/scene/scene.cpp \
          src/scene/camera.cpp \
          src/scene/model_builder.cpp \
          src/scene/picking_manager.cpp \
          src/loaders/obj_loader.cpp \
          src/loaders/loader_factory.cpp \
          src/loaders/loader_worker.cpp \
          src/renderer/opengl_renderer.cpp \
          src/view/gl_widget.cpp \
          src/facade/facade.cpp \
          src/view/main_window.cpp \
          src/settings/settings.cpp \
          src/view/gif_recorder.cpp \
          src/view/settings_dialog.cpp \
          src/state_machine/app_state_machine.cpp \
		  src/renderer/shader_program.cpp

BUILD_DIR = build
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

MOC = /usr/lib/qt6/libexec/moc
MOC_DIR = moc
MOC_SRCS = $(MOC_DIR)/moc_gl_widget.cpp \
           $(MOC_DIR)/moc_main_window.cpp \
           $(MOC_DIR)/moc_settings.cpp \
           $(MOC_DIR)/moc_settings_dialog.cpp \
           $(MOC_DIR)/moc_loader_worker.cpp \
           $(MOC_DIR)/moc_gif_recorder.cpp
MOC_OBJS = $(MOC_DIR)/moc_gl_widget.o \
           $(MOC_DIR)/moc_main_window.o \
           $(MOC_DIR)/moc_settings.o \
           $(MOC_DIR)/moc_settings_dialog.o \
           $(MOC_DIR)/moc_loader_worker.o \
           $(MOC_DIR)/moc_gif_recorder.o

TEST_DIR = tests
REPORT_DIR = report

TARGET = 3DViewer

.PHONY: all clean test gcov_report valgrind style format cppcheck install uninstall dvi dist

all: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Правила для MOC
$(MOC_DIR)/moc_gl_widget.cpp: $(VIEW_DIR)/gl_widget.h | $(MOC_DIR)
	$(MOC) $< -o $@

$(MOC_DIR)/moc_main_window.cpp: $(VIEW_DIR)/main_window.h | $(MOC_DIR)
	$(MOC) $< -o $@

$(MOC_DIR)/moc_settings.cpp: $(SETTINGS_DIR)/settings.h | $(MOC_DIR)
	$(MOC) $< -o $@

$(MOC_DIR)/moc_settings_dialog.cpp: $(VIEW_DIR)/settings_dialog.h | $(MOC_DIR)
	$(MOC) $< -o $@

$(MOC_DIR)/moc_loader_worker.cpp: $(LOADERS_DIR)/loader_worker.h | $(MOC_DIR)
	$(MOC) $< -o $@

$(MOC_DIR)/moc_gif_recorder.cpp: $(VIEW_DIR)/gif_recorder.h | $(MOC_DIR)
	$(MOC) $< -o $@

$(MOC_DIR):
	mkdir -p $(MOC_DIR)

# Компиляция MOC-файлов
$(MOC_DIR)/moc_gl_widget.o: $(MOC_DIR)/moc_gl_widget.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(MOC_DIR)/moc_main_window.o: $(MOC_DIR)/moc_main_window.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(MOC_DIR)/moc_settings.o: $(MOC_DIR)/moc_settings.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(MOC_DIR)/moc_settings_dialog.o: $(MOC_DIR)/moc_settings_dialog.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(MOC_DIR)/moc_loader_worker.o: $(MOC_DIR)/moc_loader_worker.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(MOC_DIR)/moc_gif_recorder.o: $(MOC_DIR)/moc_gif_recorder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJECTS) $(MOC_OBJS)
	$(CXX) $^ -o $@ $(QT_LIBS)

# ------------------------------------------------------------
# ТЕСТИРОВАНИЕ
# ------------------------------------------------------------
test:
	@$(MAKE) -C $(TEST_DIR) test \
		SRC_DIR=$(abspath $(SRC_DIR)) \
		REPORT_DIR=$(abspath $(REPORT_DIR))

gcov_report:
	@$(MAKE) -C $(TEST_DIR) gcov \
		SRC_DIR=$(abspath $(SRC_DIR)) \
		REPORT_DIR=$(abspath $(REPORT_DIR))

valgrind:
	@$(MAKE) -C $(TEST_DIR) valgrind \
		SRC_DIR=$(abspath $(SRC_DIR))

# ------------------------------------------------------------
# СТИЛЬ КОДА
# ------------------------------------------------------------
STYLE_FILES = $(shell find $(SRC_DIR) $(TEST_DIR) -type f \( -name '*.cpp' -o -name '*.h' \) -not -path '*/build/*' -not -path '*/moc/*')

style:
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "clang-format не установлен. Установите: sudo apt install clang-format"; \
		exit 1; \
	fi
	@clang-format -n --style=Google $(STYLE_FILES)

format:
	@if ! command -v clang-format >/dev/null 2>&1; then \
		echo "clang-format не установлен."; \
		exit 1; \
	fi
	@clang-format -i --style=Google $(STYLE_FILES)

# ------------------------------------------------------------
# СТАТИЧЕСКИЙ АНАЛИЗ
# ------------------------------------------------------------
cppcheck:
	@if ! command -v cppcheck >/dev/null 2>&1; then \
		echo "cppcheck не установлен. Установите: sudo apt install cppcheck"; \
		exit 1; \
	fi
	@cppcheck --enable=all --suppress=missingIncludeSystem --error-exitcode=1 \
	          -I$(SRC_DIR) -I$(SRC_DIR)/matrix $(STYLE_FILES)

# ------------------------------------------------------------
# УСТАНОВКА / УДАЛЕНИЕ
# ------------------------------------------------------------
PREFIX = /usr/local
INSTALL_BIN = $(PREFIX)/bin

install: $(TARGET)
	install -d $(INSTALL_BIN)
	install -m 755 $(TARGET) $(INSTALL_BIN)/
	@echo "Installed $(TARGET) to $(INSTALL_BIN)"

uninstall:
	rm -f $(INSTALL_BIN)/$(TARGET)
	@echo "Removed $(TARGET) from $(INSTALL_BIN)"

# ------------------------------------------------------------
# ДОКУМЕНТАЦИЯ
# ------------------------------------------------------------
dvi: Doxyfile
	doxygen Doxyfile
	@echo "Documentation generated in ./docs"

# ------------------------------------------------------------
# ДИСТРИБУТИВ
# ------------------------------------------------------------
DIST_NAME = 3DViewer_v2.0
DIST_FILES = Makefile README.md Doxyfile src tests
dist:
	rm -rf $(DIST_NAME)
	mkdir -p $(DIST_NAME)
	cp -r $(DIST_FILES) $(DIST_NAME)/
	tar -czf $(DIST_NAME).tar.gz $(DIST_NAME)
	rm -rf $(DIST_NAME)
	@echo "Created $(DIST_NAME).tar.gz"

# ------------------------------------------------------------
# ОЧИСТКА
# ------------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(MOC_DIR)
	rm -rf docs $(DIST_NAME).tar.gz $(REPORT_DIR)
	rm -rf ./docs\ \>\ Doxyfile/
	@$(MAKE) -C $(TEST_DIR) clean

# -----------------------------------------------------------
#  ОТЛАДКА
# ---------------------------------------- ------------------
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all
