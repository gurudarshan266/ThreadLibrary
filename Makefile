all:
	gcc -g -c mythread.cpp DThread.cpp DSemaphore.cpp -lstdc++ 
	ar rcs gpollep.a mythread.o DThread.o DSemaphore.o
