CC = g++
CFLAGS = -g -Wall -std=c++11 -pthread
CCLINK = $(CC)
OBJS = App.o Atm.o Bank.o Account.o
RM = rm -f

run: $(OBJS)
	$(CCLINK) -o Bank $(OBJS) $(CFLAGS)

App.o: App.cpp Atm.h Bank.h Account.h pthread_includes.h
	$(CC) -c App.cpp $(CFLAGS)

Atm.o: Atm.h Atm.cpp Bank.h
	$(CC) -c Atm.cpp $(CFLAGS)

Bank.o: Bank.h Bank.cpp Account.h
	$(CC) -c Bank.cpp $(CFLAGS)

Account.o: Account.h Account.cpp
	$(CC) -c Account.cpp $(CFLAGS)

tar:
	tar -cvf 200879807.tar *.cpp *.h makefile README

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*
