# ------------------------------------------------------------
# Корневой Makefile проекта Multilayer Perceptron
# Управляет сборкой библиотеки, тестов и GUI-приложения
# ------------------------------------------------------------

.PHONY: all tests clean install uninstall dvi dist gcov_report gui

LIBMLP_DIR = libmlp
TEST_DIR = tests/unit
GUI_DIR = gui_app

all: libmlp
	@echo "Build finished. GUI app not included yet."

libmlp:
	$(MAKE) -C $(LIBMLP_DIR) all

gui: libmlp
	$(MAKE) -C $(GUI_DIR) all

tests:
	$(MAKE) -C $(TEST_DIR) test

gcov_report:
	$(MAKE) -C $(TEST_DIR) gcov

install:
	install -d $(PREFIX)/lib $(PREFIX)/include/mlp
	cp $(LIBMLP_DIR)/libmlp.a $(PREFIX)/lib/
	cp $(LIBMLP_DIR)/include/mlp/matrix/s21_matrix_oop.h $(PREFIX)/include/mlp/

uninstall:
	rm -f $(PREFIX)/lib/libmlp.a
	rm -f $(PREFIX)/include/mlp/s21_matrix_oop.h

dvi:
	@echo "Documentation will be generated here (Doxygen)."

DIST_NAME = multilayer_perceptron
dist:
	rm -rf $(DIST_NAME)
	mkdir $(DIST_NAME)
	cp -r libmlp gui_app tests datasets report scripts rag_data Makefile README.md $(DIST_NAME)/
	tar -czf $(DIST_NAME).tar.gz $(DIST_NAME)
	rm -rf $(DIST_NAME)

clean:
	$(MAKE) -C $(LIBMLP_DIR) clean
	$(MAKE) -C $(TEST_DIR) clean
	$(MAKE) -C $(GUI_DIR) clean
