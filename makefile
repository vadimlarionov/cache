all: compile
clean:
	@-rm -f cache.out
compile: clean
	@echo Compile
	@g++ -std=c++11 -o cache.out cache.cpp
	@echo Success
