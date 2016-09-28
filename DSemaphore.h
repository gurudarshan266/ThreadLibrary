/*
 * DSemaphore.h
 *
 *  Created on: Sep 28, 2016
 *      Author: guru
 */

#ifndef DSEMAPHORE_H_
#define DSEMAPHORE_H_

#include "DThread.h"
#include <queue>

class DSemaphore {
public:
	DSemaphore(int);

	void AddToWaitingQ(DThread* t) { mWaitingQ.push(t); }
	std::queue<DThread*>& GetWaitingQ(){ return mWaitingQ; }
	int GetSid() { return mSid; }

	int mValue;
	static int mNumSemaphores;

private:
	int mSid;
	std::queue<DThread*> mWaitingQ;
	int mMaxValue;

};

#endif /* DSEMAPHORE_H_ */
