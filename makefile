CC = gcc
TARGET = serialHelper

ifdef SystemRoot
RM = del
else
RM = rm -f
endif

# --------------------------------------

all: serialHelp cleansmall

# SystemRoot is only defined in Windows
ifdef SystemRoot
serialHelp: $(TARGET)Win.exe
else
serialHelp: $(TARGET)Unix
endif

unixSerial.o: unixSerial.c serial.h
	gcc -x c -o unixSerial.o -c unixSerial.c

serialHelper.o: serialHelper.c
	gcc -x c -o serialHelper.o -c serialHelper.c

winSerial.o: winSerial.c serial.h
	gcc -x c -o winSerial.o -c winSerial.c

$(TARGET)Win.exe: winSerial.o serialHelper.o
	gcc -o $(TARGET)Win.exe winSerial.o serialHelper.o -D PlatformWindows

$(TARGET)Unix: unixSerial.o serialHelper.o
	gcc -o $(TARGET)Unix unixSerial.o serialHelper.o

# Delete intermediate files
clean: cleansmall
ifdef SystemRoot
	$(RM) $(TARGET)Win.exe
else
	$(RM) $(TARGET)Unix
endif

cleansmall:
	$(RM) *.o