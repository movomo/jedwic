CC = gcc
# CFLAGS = -O2

EXEC = test.exe
CFLAGS_TEST = -Wall
ALLHEADERS = src/src/json.h src/jsonarr.h src/jsonobj.h \
	src/ast.h src/token.h src/lexer.h src/parser.h
ALLOBJECTS = src/json.o src/jsonarr.o src/jsonobj.o \
	src/ast.o src/token.o src/lexer.o src/parser.o

run: $(EXEC)
	./$(EXEC)

test.exe: test.c lib/libjedwic.dll
	$(CC) $(CFLAGS_TEST) -o $(EXEC) -Iinclude -Isrc -Llib -ljedwic test.c
	ln -s lib/libjedwic.dll libjedwic.dll

lib: $(ALLOBJECTS)
	$(CC) -o lib/libjedwic.dll -shared $(ALLOBJECTS)

src/json.o: $(ALLHEADERS)
src/jsonarr.o: src/json.h src/jsonarr.h
src/jsonobj.o: src/json.h src/jsonobj.h
src/ast.o: src/ast.h src/token.h
src/token.o: src/token.h
src/lexer.o: src/token.h src/lexer.h
src/parser.o: src/ast.h src/token.h src/lexer.h src/parser.h
