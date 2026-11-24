CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -fPIC
LDFLAGS = -ldl

PREFIX = /usr/local
BIN_DIR = $(PREFIX)/bin
LIB_DIR = $(PREFIX)/lib/cipher_rgr

TAPP_SRC = src/main.cpp src/auth.cpp
APP_OBJ = $(TAPP_SRC:.cpp=.o)

TARGET = cipher_app
LIBS = columnar.so polybius.so fixed.so

# Основная цель
all: libraries $(TARGET)

# Сборка основного приложения
$(TARGET): $(APP_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ -ldl $(LDFLAGS)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

libraries: $(LIBS)

columnar.so: libs/columnar.cpp
	$(CXX) $(CXXFLAGS) -shared $< -o $@

polybius.so: libs/polybius.cpp
	$(CXX) $(CXXFLAGS) -shared $< -o $@

fixed.so: libs/fixed.cpp
	$(CXX) $(CXXFLAGS) -shared $< -o $@

clean:
	rm -f $(TARGET) $(LIBS)
	rm -rf build

install: all
	mkdir -p $(DESTDIR)$(BIN_DIR)
	mkdir -p $(DESTDIR)$(LIB_DIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BIN_DIR)
	install -m 644 $(LIBS) $(DESTDIR)$(LIB_DIR)
	@echo "Установка завершена. Библиотеки находятся в $(LIB_DIR)"

uninstall:
	rm -f $(DESTDIR)$(BIN_DIR)/$(TARGET)
	rm -rf $(DESTDIR)$(LIB_DIR)
	@echo "Деинсталляция завершена."

.PHONY: all clean install uninstall
