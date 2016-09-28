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
/*
 * Creates a new thread
 * Adds it to the ready queue
 */
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

void MyThreadInit(void(*start_funct)(void *), void *args)
{
	DThread* t = (DThread*) MyThreadCreate(start_funct,args);

	MyThreadSwitch();
//	MyThreadJoinAll();
}

int MyThreadInitExtra(void(*start_funct)(void *), void *args)
{
	masterCtxt = new ucontext_t();
	masterThd = new DThread(masterCtxt);
	OUT<<"Master thread created with TID = "<<masterThd->GetTid()<<endl;
//	getcontext(masterCtxt);

	DThread* t = (DThread*) MyThreadCreate(start_funct,args);

	masterThd->AddChildren(t);

	Running = masterThd;
	MyThreadJoin(t);
//
//	Running = masterThd;
//	masterThd->mWaitingOnThds++;
//	t->AddToWaitedByList(masterThd);
//
//
//	MyThreadSwitch();
	OUT<<"After Thd Switch"<<endl;
//	MyThreadJoinAll();
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

void HouseKeeping()
{
}




