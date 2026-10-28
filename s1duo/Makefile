CC = clang
CFLAGS = -O3 -Wall -Wextra -Wno-unused-parameter -Iinclude -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE
LDFLAGS = -pie -Wl,-z,relro,-z,now -lm

SRCS = src/common.c src/lexer.c src/value.c src/ast.c src/parser.c src/env.c src/modules.c src/evaluator.c src/main.c
OBJS = $(SRCS:.c=.o)
TARGET = bin/airalang3
INSTALL_DIR = /data/data/com.termux/files/usr/bin

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "\033[1;32m[✓] AiraLang S1 Duo Native Binary successfully compiled!\033[0m"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	@mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)/airalang3
	chmod 755 $(INSTALL_DIR)/airalang3
	@echo "\033[1;32m[✓] Installed airalang3 to $(INSTALL_DIR)/airalang3\033[0m"

clean:
	rm -rf src/*.o bin/
	@echo "\033[1;33m[*] Cleaned build artifacts.\033[0m"

.PHONY: all clean install
