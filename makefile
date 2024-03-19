CC = gcc
CFLAGS = -Wall -g -Iobj -Isrc
PARSER = le_parser
LEXER = le_lexer

bin/tpcas: obj/$(LEXER).o obj/$(PARSER).o obj/tree.o
	$(CC) -o $@ $^

obj/$(PARSER).o: obj/$(PARSER).c src/tree.h
obj/$(LEXER).o: obj/$(LEXER).c obj/$(PARSER).h

obj/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

obj/$(LEXER).c: src/$(LEXER).l obj/$(PARSER).h
	flex -o $@ $<

obj/$(PARSER).c obj/$(PARSER).h: src/$(PARSER).y
	bison -d -o obj/$(PARSER).c $<

clean:
	rm -f obj/* bin/*