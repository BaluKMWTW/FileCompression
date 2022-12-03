build:
	rm -f program.exe
	g++ -g -Wall -std=c++11 main.cpp hashmap.cpp -o program.exe
	
run:
	./program.exe

clean:
	rm -f program.exe

valgrind:
	valgrind --tool=memcheck --leak-check=yes ./program.exe
