/*
 * mythreadextra.h
 *
 *  Created on: Sep 28, 2016
 *      Author: gpollep
 */

#ifndef MYTHREADEXTRA_H_
#define MYTHREADEXTRA_H_
typedef void *MyThread;
typedef void *MySemaphore;

// ****** THREAD OPERATIONS ******
// Create a new thread.
MyThread MyThreadCreate(void(*start_funct)(void *), void *args);

// Yield invoking thread
void MyThreadYield(void);

// Join with a child thread
int MyThreadJoin(MyThread thread);

// Join with all children
void MyThreadJoinAll(void);

// Terminate invoking thread
void MyThreadExit(void);

// ****** SEMAPHORE OPERATIONS ******
// Create a semaphore
MySemaphore MySemaphoreInit(int initialValue);

// Signal a semaphore
void MySemaphoreSignal(MySemaphore sem);

// Wait on a semaphore
void MySemaphoreWait(MySemaphore sem);

// Destroy on a semaphore
int MySemaphoreDestroy(MySemaphore sem);

// ****** CALLS ONLY FOR UNIX PROCESS ******
// Create and run the "main" thread
void MyThreadInit(void(*start_funct)(void *), void *args);

int MyThreadInitExtra();


#endif /* MYTHREADEXTRA_H_ */
