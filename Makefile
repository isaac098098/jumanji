FLAGS = -Wall -pedantic
LIB = -lmupdf -lGL -lglfw -lm
INCLUDE = -Iinclude/
OBJS = build/main.o build/glad.o build/callbacks.o build/window.o \
       build/pdf.o build/renderer.o
TEST_FILE = [put a local test file here]

build/jumanji : $(OBJS)
	gcc $(OBJS) -o build/jumanji $(LIB) $(FLAGS)

build/main.o : src/main.c
	mkdir -p build
	gcc -c src/main.c -o build/main.o $(INCLUDE) $(FLAGS)

build/glad.o : src/window/glad.c
	gcc -c src/window/glad.c -o build/glad.o $(INCLUDE)

build/callbacks.o : src/window/callbacks.c src/window/callbacks.h include/state.h
	gcc -c src/window/callbacks.c -o build/callbacks.o $(INCLUDE)

build/window.o : src/window/window.c include/window.h
	gcc -c src/window/window.c -o build/window.o $(INCLUDE)

build/renderer.o : src/renderer/renderer.c include/renderer.h
	gcc -c src/renderer/renderer.c -o build/renderer.o $(INCLUDE)

build/pdf.o : src/pdf/pdf.c include/pdf.h
	gcc -c src/pdf/pdf.c -o build/pdf.o $(INCLUDE)

.PHONY : run debug clean

run : build/jumanji
	./build/jumanji $(TEST_FILE)

debug : $(OBJS)
	gcc $(OBJS) -o build/jumanji $(LIB) $(FLAGS) -g -O0
	valgrind --leak-check=yes ./build/jumanji $(TEST_FILE) 2> valgrind.log

clean :
	rm -rf build/ valgrind.log
