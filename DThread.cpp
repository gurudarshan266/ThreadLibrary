/*
 * DThread.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: gpollep
 */
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mythread.h"
#include "DThread.h"

int DThread::NumOfThds = 1;
ucontext_t* DThread::sSuccessorCtxt = NULL;

DThread::DThread(void(*start_funct)(void *), void *args) : mWaitingOnThds(0), mTerminated(false)
{
	getcontext(&mCtxt);
	mCtxt.uc_stack.ss_sp = mStack;
	mCtxt.uc_stack.ss_size = sizeof mStack;
	mCtxt.uc_link = DThread::GetSuccessorCtxt();
	//TODO: Add code for handling args
    makecontext(&mCtxt, (void (*)())start_funct, 0);

    mTid = NumOfThds++;


}

void DThread::SetResumePoint(DThread& t)
{
	t.GetContext().uc_link = &mCtxt;
}

/* Creates a new copy of the ssuccessor's context */
ucontext_t* DThread::GetSuccessorCtxt()
{
	if(sSuccessorCtxt)
	{
		ucontext_t* ctxt = new ucontext_t();
		memcpy(ctxt, sSuccessorCtxt, sizeof(ucontext_t));
		return ctxt;
	}

	return NULL;
}


bool DThread::HasChild(DThread* t)
{
	for(int i=0; i<mChildren.size(); i++)
		if(t == mChildren[i])
			return true;
	return false;
}

