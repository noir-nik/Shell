TARGET = shell
SRCS = \
 		main.c \
		word_list.c \
		tree.c \
		exec.c
.PHONY: clean build

$(TARGET): $(SRCS)
		gcc -ggdb $(SRCS) -o $(TARGET) -g

build: $(TARGET)
clean:
		rm -rf $(TARGET)
run:
		rlwrap valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)
		