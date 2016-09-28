/*
 * DSemaphore.cpp
 *
 *  Created on: Sep 28, 2016
 *      Author: guru
 */

#include "DSemaphore.h"
#include "mythread.h"
#include "DThread.h"

int DSemaphore::mNumSemaphores = 0;

DSemaphore::DSemaphore(int maxVal) : mMaxValue(maxVal), mValue(maxVal), mSid(++mNumSemaphores)
{

}
