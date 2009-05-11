//
// Thread_POSIX.h
//
// $Id: //poco/1.3/Foundation/include/Poco/Thread_POSIX.h#9 $
//
// Library: Foundation
// Package: Threading
// Module:  Thread
//
// Definition of the ThreadImpl class for POSIX Threads.
//
// Copyright (c) 2004-2007, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Foundation_Thread_POSIX_INCLUDED
#define Foundation_Thread_POSIX_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/Runnable.h"
#include "Poco/SignalHandler.h"
#include "Poco/Event.h"
#include "Poco/RefCountedObject.h"
#include "Poco/AutoPtr.h"
#include <pthread.h>
// must be limits.h (not <climits>) for PTHREAD_STACK_MIN on Solaris
#include <limits.h>
#if !defined(POCO_NO_SYS_SELECT_H)
#include <sys/select.h>
#endif
#include <errno.h>


namespace Poco {


class Foundation_API ThreadImpl
{
public:	
	typedef void (*Callable)(void*);

	enum Priority
	{
		PRIO_LOWEST_IMPL,
		PRIO_LOW_IMPL,
		PRIO_NORMAL_IMPL,
		PRIO_HIGH_IMPL,
		PRIO_HIGHEST_IMPL
	};

	struct CallbackData: public RefCountedObject
	{
		CallbackData(): callback(0), pData(0)
		{
		}

		Callable  callback;
		void*     pData; 
	};

	ThreadImpl();				
	~ThreadImpl();

	void setPriorityImpl(int prio);
	int getPriorityImpl() const;
	void setOSPriorityImpl(int prio);
	int getOSPriorityImpl() const;
	static int getMinOSPriorityImpl();
	static int getMaxOSPriorityImpl();
	void setStackSizeImpl(int size);
	int getStackSizeImpl() const;
	void startImpl(Runnable& target);
	void startImpl(Callable target, void* pData = 0);

	void joinImpl();
	bool joinImpl(long milliseconds);
	bool isRunningImpl() const;
	static void sleepImpl(long milliseconds);
	static void yieldImpl();
	static ThreadImpl* currentImpl();

protected:
	static void* runnableEntry(void* pThread);
	static void* callableEntry(void* pThread);
	static int mapPrio(int prio);
	static int reverseMapPrio(int osPrio);

private:
	class CurrentThreadHolder
	{
	public:
		CurrentThreadHolder()
		{
			if (pthread_key_create(&_key, NULL))
				throw SystemException("cannot allocate thread context key");
		}
		~CurrentThreadHolder()
		{
			pthread_key_delete(_key);
		}
		ThreadImpl* get() const
		{
			return reinterpret_cast<ThreadImpl*>(pthread_getspecific(_key));
		}
		void set(ThreadImpl* pThread)
		{
			pthread_setspecific(_key, pThread);
		}
	
	private:
		pthread_key_t _key;
	};

	struct ThreadData: public RefCountedObject
	{
		ThreadData():
			pRunnableTarget(0),
			pCallbackTarget(0),
			thread(0),
			prio(PRIO_NORMAL_IMPL),
			done(false),
			stackSize(POCO_THREAD_STACK_SIZE)
		{
		}

		Runnable*     pRunnableTarget;
		AutoPtr<CallbackData> pCallbackTarget;
		pthread_t     thread;
		int           prio;
		int           osPrio;
		Event         done;
		std::size_t   stackSize;
	};

	AutoPtr<ThreadData> _pData;

	static CurrentThreadHolder _currentThreadHolder;
	
#if defined(POCO_OS_FAMILY_UNIX)
	SignalHandler::JumpBufferVec _jumpBufferVec;
	friend class SignalHandler;
#endif
};


//
// inlines
//
inline int ThreadImpl::getPriorityImpl() const
{
	return _pData->prio;
}


inline int ThreadImpl::getOSPriorityImpl() const
{
	return _pData->osPrio;
}


inline bool ThreadImpl::isRunningImpl() const
{
	return _pData->pRunnableTarget != 0 ||
		(_pData->pCallbackTarget.get() != 0 && _pData->pCallbackTarget->callback != 0);
}


inline void ThreadImpl::yieldImpl()
{
	sched_yield();
}


inline int ThreadImpl::getStackSizeImpl() const
{
	return _pData->stackSize;
}


} // namespace Poco


#endif // Foundation_Thread_POSIX_INCLUDED
