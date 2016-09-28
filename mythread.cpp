/*
 * mythread.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: gpollep
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <queue>
#include <string>
#include <vector>
#include <ucontext.h>

#include "DThread.h"
#include "DSemaphore.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

	#include "mythread.h"
	#include "mythreadextra.h"

#ifdef __cplusplus
}
#endif

void MyThreadSwitch();

queue<DThread*> Blocked;
DThread* Running = NULL;
queue<DThread*> Ready;

ucontext* masterCtxt;
DThread* masterThd;

ostringstream sss;
#define OUT sss

#define OUT2 cout
/*
 * Creates a new thread
 * Adds it to the ready queue
 */

ucontext_t* x;

MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
	DThread* thd = new DThread(start_funct, args);

	if(Running)
		Running->AddChildren(thd);

	Ready.push(thd);

//	setcontext(&thd->GetContext());

	OUT<<"Added new Thread "<<thd->GetTid()<<" to Ready queue"<<endl;
	return (MyThread)thd;
}

/*
 * Pushes Running thread to the end of the queue
 * Makes the first thread in the Ready queue as the Running thread
 */
void MyThreadYield(void)
{
	//If there are threads in the Ready queue
	if(!Ready.empty())
	{
		DThread *prev = Running;
		Running = Ready.front();
		Ready.pop();
		Ready.push(prev);

		OUT<<"Switching to Thread "<<prev->GetTid()<<" from Thread "<<Running->GetTid()<<endl;
		swapcontext(&prev->GetContext(), &Running->GetContext());
	}

	//No context swap required if its the same thread
}

/*
 * If the thread is not a child or is terminated, return an error
 * Add the Running thread to waiter's list of the thread
 */
int MyThreadJoin(MyThread thread)
{
	DThread* cur = Running;
	DThread* thdToWaitOn = (DThread*) thread;

	//If the given thread is not a immediate child, then return error
	if(!cur->HasChild(thdToWaitOn))
	{
		OUT<<"Thread "<<thdToWaitOn->GetTid()<<" is not a child of Thread "<<cur->GetTid()<<endl;
		return 0;
	}

	//If the child thread has already terminated, return error and do not block
	if(thdToWaitOn->IsTerminated())
	{
		OUT<<"Thread "<<thdToWaitOn->GetTid()<<" is already terminated. So not blocking Thread "<<cur->GetTid()<<endl;
		return -1;
	}


	thdToWaitOn->AddToWaitedByList(Running);//Update the WaitedBy List
	cur->mWaitingOnThds++;//Increment the counter of waiting on threads

	Blocked.push(cur);

	MyThreadSwitch();

	return 0;
}


void MyThreadJoinAll(void)
{
	DThread* cur = Running;

	vector<DThread*>& children = cur->GetChildren();

	for(int i=0; i<children.size(); i++)
	{
		DThread* child = children[i];

		//If child is terminated, do nothing
		if(child->IsTerminated())
			continue;

		child->AddToWaitedByList(cur);
		cur->mWaitingOnThds++;
	}

	//Checking if atleast one child is alive
	if(cur->mWaitingOnThds == 0)
	{
		OUT<<"All children threads are terminated. So not blocking Thread "<<cur->GetTid()<<endl;
		return;
	}

	Blocked.push(cur);

	MyThreadSwitch();
}

void MyThreadExit()
{
	DThread* cur = Running;

	if(cur->IsTerminated())
		OUT<<"Clean up not required for Thread "<<cur->GetTid()<<endl;

	vector<DThread*>& WaitedbyThds = cur->GetWaitedByThreads();

	for(int i=0; i< WaitedbyThds.size(); i++)
	{
		//Decrement the waiting counter on the threads
		WaitedbyThds[i]->mWaitingOnThds--;

		if(WaitedbyThds[i]->mWaitingOnThds == 0)
		{
			OUT<<"Thread "<<WaitedbyThds[i]->GetTid()<<" is not waiting on anymore threads. Added to ready queue"<<endl;
			Ready.push(WaitedbyThds[i]);
		}
	}

	cur->SetTerminated(true);
	cur->ClearStack();

	OUT<<"Killing Thread "<<cur->GetTid()<<endl;

	//Forcing MyThreadSwitch to use setcontext
	Running = NULL;

	MyThreadSwitch();
}

void MyThreadInit2(void(*start_funct)(void *), void *args)
{
	DThread* t = (DThread*) MyThreadCreate(start_funct,args);

	MyThreadSwitch();
//	MyThreadJoinAll();
}

void MyThreadInit(void(*start_funct)(void *), void *args)
{
#if 0
	//Allocation for exit-call context
	getcontext(exitCtxt);
	mExitCtxt.uc_stack.ss_sp = exitStack;
	mExitCtxt.uc_stack.ss_size = sizeof mExitStack;
	makecontext(&mExitCtxt, MyThreadExit,0);
#endif

	masterCtxt = new ucontext_t();
	masterThd = new DThread(masterCtxt);
	OUT<<"Master thread created with TID = "<<masterThd->GetTid()<<endl;

	DThread* t = (DThread*) MyThreadCreate(start_funct,args);

	masterThd->AddChildren(t);

	Running = masterThd;
	MyThreadJoin(t);
	OUT<<"After End of Thd Switch"<<endl;
}

int MyThreadInitExtra(void)
{
	masterCtxt = new ucontext_t();
	masterThd = new DThread(masterCtxt);
	Running = masterThd;
	OUT<<"Master thread created with TID = "<<masterThd->GetTid()<<endl;
}

void MyThreadSwitch()
{
	DThread* cur = Running;

	if(Ready.empty())
	{
		OUT<<"Ready queue is empty"<<endl;
//		return;
		exit(-1);
	}

	Running = Ready.front();//Get a new thread from ready queue
	Ready.pop();

	//For starting cases
	if(cur==NULL)
		setcontext(&Running->GetContext());
	else
		swapcontext(&cur->GetContext(), &Running->GetContext());
}

MySemaphore MySemaphoreInit(int initialValue)
{
	DSemaphore* s = new DSemaphore(initialValue);
	return (MySemaphore)s;
}

void MySemaphoreSignal(MySemaphore sem)
{
	DSemaphore* s = (DSemaphore*)sem;

	//If semaphore has 0, remove from blocking queue (if any) and add it to ready queue
	if(s->mValue <= 0)
	{
		OUT2<<"Thread "<<Running->GetTid()<<" released Semaphore "<<s->GetSid()<<endl;

		std::queue<DThread*>& queue = s->GetWaitingQ();

		if(!queue.empty())
		{
			DThread* t = queue.front();
			queue.pop();

			Ready.push(t);
			OUT2<<"Tranferred Thread "<<t->GetTid()<<" from Semaphore "<<s->GetSid()<<" Blocked queue to Ready queue"<<endl;
		}
		else//If Blocked queue is empty, then increment the semaphore value
		{
			s->mValue++;
		}

	}
	else
	{
		s->mValue++;
	}

}

void MySemaphoreWait(MySemaphore sem)
{
	DSemaphore* s = (DSemaphore*)sem;
	DThread* cur = Running;

	//If semaphore has 0, add to blocking queue and switch the thread (from ready queue)
	if(s->mValue <= 0)
	{
		OUT2<<"Thread "<<cur->GetTid()<<" is blocked on Semaphore "<<s->GetSid()<<endl;
		s->AddToWaitingQ(cur);
		MyThreadSwitch();
	}
	else
	{
		s->mValue--;
	}
}

int MySemaphoreDestroy(MySemaphore sem)
{
	DSemaphore* s = (DSemaphore*)sem;

	if(!s->GetWaitingQ().empty())
	{
		OUT2<<"Semaphore "<<s->GetSid()<<" has non-empty waiting queue. So not destroying it"<<endl;
		return -1;
	}

	delete s;
	return 0;
}




