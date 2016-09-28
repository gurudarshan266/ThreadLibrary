/*
 * DThread.h
 *
 *  Created on: Sep 27, 2016
 *      Author: gpollep
 */

#ifndef DTHREAD_H_
#define DTHREAD_H_

#include <ucontext.h>
#include <vector>

#define STACK_SIZE (1<<13) //8K

class DThread
{
public:
	DThread(void(*start_funct)(void *), void *args);
	DThread(ucontext_t* uc);

	int GetTid() { return mTid; }
	ucontext_t& GetContext() { return mCtxt; };
	std::vector<DThread*>& GetChildren() { return mChildren; }

	void SetResumePoint(DThread& t);
	void SetContext(ucontext_t* cxt) { mCtxt = *cxt; }

	void AddToWaitedByList(DThread* t) { mWaitedByThds.push_back(t); }
	std::vector<DThread*>& GetWaitedByThreads() { return mWaitedByThds; }

	void AddChildren(DThread* t) { mChildren.push_back(t); }

	bool HasChild(DThread* t);

	bool IsTerminated() { return mTerminated; }
	void SetTerminated(bool val) { mTerminated = val; }

	void ClearStack();

	static int NumOfThds;
	static ucontext_t* sSuccessorCtxt;
	static ucontext_t* GetSuccessorCtxt();

	int mWaitingOnThds;

private:
	int mTid;
	ucontext_t mCtxt;
	ucontext_t mExitCtxt;
	std::vector<DThread*> mChildren;
	std::vector<DThread*> mWaitedByThds;
	bool mTerminated;
	char mStack[STACK_SIZE];
	char mExitStack[512];


};



#endif /* DTHREAD_H_ */
