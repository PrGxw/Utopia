BIN=.\bin
SRC=.\src
CC=cl
LINK=link
EXE=win64_utopia.exe
LIB=/IMPLIB:user32.lib /IMPLIB:gdi32.lib
all: $(EXE)

# win64_utopia.exe: $(BIN)\win64_utopia.obj $(BIN)\utopia.obj
# 		$(LINK) $(BIN)\win64_utopia.obj $(BIN)\utopia.obj $(LIB) /OUT:$(BIN)\win32_utopia.exe
win64_utopia.exe: $(SRC)\utopia.cpp $(SRC)\win64_utopia.cpp
		$(CC) -Zi $(SRC)\utopia.cpp $(SRC)\win64_utopia.cpp /link user32.lib gdi32.lib /OUT:$(BIN)\win64_utopia.exe

$(BIN)\utopia.obj: $(SRC)\utopia.cpp $(SRC)\utopia.h
		$(CC) /c /Fo$(BIN)\\ $(SRC)\utopia.cpp 
$(BIN)\win64_utopia.obj: $(SRC)\win64_utopia.cpp $(SRC)\win64_utopia.h
		$(CC) /c /Fo$(BIN)\\ $(SRC)\win64_utopia.cpp
clean :
		del $(BIN)\*.obj $(BIN)\*.exe