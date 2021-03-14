DIROBJ := obj/
DIREXE := exec/
DIRSRC := src/

COMMFLAGS := -Wall -Wextra -c
CFLAGS := $(COMMFLAGS) -std=c99
CXXFLAGS := $(COMMFLAGS) -std=c++14
LDLIBS := -lpthread -lrt
CC := gcc
CXX := g++


all : dirs manager pa pb pc pd backupd

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

manager: $(DIROBJ)manager.o 
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

pa: $(DIROBJ)pa.o 
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

pb: $(DIROBJ)pb.o 
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

pc: $(DIROBJ)pc.o
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

pd: $(DIROBJ)pd.o
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

backupd: $(DIROBJ)backupd.o
	$(CXX) -o $(DIREXE)$@ $^ $(LDLIBS) -lstdc++

$(DIROBJ)%.o: $(DIRSRC)%.c
	$(CC) $(CFLAGS) $^ -o $@

$(DIROBJ)%.o: $(DIRSRC)%.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test:
	./$(DIREXE)manager 3 2 5

solution:
	./$(DIREXE)manager 2 3 4

clean: dirs pd
	./$(DIREXE)pd
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRHEA)*~ $(DIRSRC)*~
