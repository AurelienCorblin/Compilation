CC = gcc
CFLAGS = -std=c17 -Wall -g -Iobj -Isrc
PARSER = le_parser
LEXER = le_lexer

all: bin/tpcc


help:
	@echo "Use command make to build the compiler, then do ./bin/tpcc -h to know how to use it.\n\
	After you got an <nasm_name>.asm file you can build its executable using the command :\
	 make build TARGET=<nasm_name>.asm" 


# to build compiler
bin/tpcc: obj/$(LEXER).o obj/$(PARSER).o obj/tree.o obj/symbol.o obj/trad.o | bin
	$(CC) -o $@ $^ 

obj/$(PARSER).o: obj/$(PARSER).c src/tree.h src/symbol.h src/trad.h | obj
obj/$(LEXER).o: obj/$(LEXER).c obj/$(PARSER).h 

obj/%.o: src/%.c | obj
	$(CC) -c -o $@ $< $(CFLAGS)

obj/$(LEXER).c: src/$(LEXER).l obj/$(PARSER).h | obj
	flex -o $@ $<

obj/$(PARSER).c obj/$(PARSER).h: src/$(PARSER).y | obj
	bison -d -o obj/$(PARSER).c $<


# to build executable from .asm
build: bin/$(basename $(TARGET))

bin/$(basename $(TARGET)): $(basename $(TARGET)).o | bin
	$(CC) -o $@ $^ -nostartfiles -no-pie
	rm $(basename $(TARGET)).o

$(basename $(TARGET)).o: $(TARGET)
	nasm -f elf64 $< -o $@


# create dir if they dont exists
bin:
	mkdir -p bin
obj:
	mkdir -p obj


# rule to clean object file and executable
clean:
	rm -f obj/* bin/*