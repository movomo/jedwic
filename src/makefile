CC = gcc
# CFLAGS = -c
# CFLAGS = -O2

EXEC = test.exe
CFLAGS_TEST = -Wall
ALLHEADERS = json.h jsonarr.h jsonobj.h ast.h token.h lexer.h parser.h
ALLOBJECTS = json.o jsonarr.o jsonobj.o ast.o token.o lexer.o parser.o

run: $(EXEC)
	./$(EXEC)

test.exe: test.c $(ALLHEADERS) $(ALLOBJECTS)
	$(CC) $(CFLAGS_TEST) -o $(EXEC) test.c $(ALLOBJECTS)

json.o: $(ALLHEADERS)
jsonarr.o: json.h jsonarr.h
jsonobj.o: json.h jsonobj.h
ast.o: ast.h token.h
token.o: token.h
lexer.o: token.h lexer.h
parser.o: ast.h token.h lexer.h parser.h
