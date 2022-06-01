COMPILER = g++ -std=c++11

.PHONY: clean

main: BPlusTree.o BufferManager.o IndexManager.o RecordManager.o CatalogManager.o \
API.o Interpreter.o main.o
	$(COMPILER) -o $@ $^  

main.o: main.cpp API.h Address.h CatalogManager.h Interpreter.h
	$(COMPILER) -o $@ -c $<

Interpreter.o:Interpreter.cpp Interpreter.h API.o
	$(COMPILER) -o $@ -c $<

RecordManager.o:RecordManager.cpp RecordManager.h BufferManager.o
	$(COMPILER) -o $@ -c $<

API.o: API.cpp API.h RecordManager.o CatalogManager.o IndexManager.o
	$(COMPILER) -o $@ -c $<

BufferManager.o: BufferManager.cpp BufferManager.h
	$(COMPILER) -o $@ -c $<

IndexManager.o: IndexManager.cpp IndexManager.h BufferManager.o BPlusTree.o
	$(COMPILER) -o $@ -c $<

CatalogManager.o: CatalogManager.cpp CatalogManager.h BufferManager.o
	$(COMPILER) -o $@ -c $<

BPlusTree.o: BPlusTree.cpp BPlusTree.h Node.h
	$(COMPILER) -o $@ -c $<

clean:
	rm *.o main