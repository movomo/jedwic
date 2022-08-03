CC = gcc
# CFLAGS = -c
# CFLAGS = -O2

EXEC = test.exe
CFLAGS_TEST = -Wall
ALLHEADERS = json.h jsonarr.h jsonobj.h ast.h token.h lexer.h
ALLOBJECTS = json.o jsonarr.o jsonobj.o ast.o token.o lexer.o

run: $(EXEC)
	./$(EXEC)

test.exe: test.c $(ALLHEADERS) $(ALLOBJECTS)
	$(CC) $(CFLAGS_TEST) -o $(EXEC) test.c $(ALLOBJECTS)

json.o: json.h jsonarr.h
jsonarr.o: json.h jsonarr.h
jsonobj.o: json.h jsonobj.h
ast.o: ast.h
token.o: token.h
lexer.o: token.h lexer.h
