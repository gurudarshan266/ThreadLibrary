all:
	gcc -g -c mythread.cpp DThread.cpp DSemaphore.cpp -lstdc++ 
	ar rcs mythread.a mythread.o DThread.o DSemaphore.o

clean:
	rm mythread.a mythread.o DThread.o DSemaphore.o
