//============================================================================
// Name        	: ThreadBase.h
// Author      	: Trung
// Version     	: 0.01.2016.0910.2255
// Copyright   	:
// Description 	: A wrapper class of POSIX's thread
// Note		: User should derive this class and/or override methods for specific tasks
// Usage	: Header library; compile with -pthread and link -lpthread.a
//
//============================================================================
#ifndef THREADBASE_H_
#define THREADBASE_H_

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <iostream>
#include <cstring>

class Semaphore {
protected:
	sem_t*					m_handle;
public:
	Semaphore() {
		m_handle = new sem_t();
		sem_init(m_handle, 0, 1);
	}
	virtual ~Semaphore() {
		if(m_handle)
		{
			sem_destroy(m_handle);
			delete m_handle;
			m_handle =NULL;
		}
	}

	bool Lock() {
		sem_wait(m_handle);
		return true;
	}
	bool UnLock() {
		sem_post(m_handle);
		return true;
	}
	bool IsLocked( ) {
		int sval = -1;
		sem_getvalue(m_handle, &sval);
		return (sval==0);
	}
};

class ThreadBase {
private:
	enum THREAD_FLAGS {
		THREAD_RUN		=	0,
		THREAD_STOP		=	1,
		THREAD_PAUSE		=	2
	};
	/*	Description		- Handle the POSIX thread object.
	 */
	pthread_t 			m_ThreadHandle;
	/*	Description		- Handle the POSIX thread's attributes.
	 */
	pthread_attr_t     	m_ThreadAttribute;
	/*	Description		- Storing the thread's name.
	 */
	char 				m_ThreadName[64];
	/*	Description		- Flag for controlling the thread.
	 */
	int					m_ThreadStatus;
	/*	Description		- Flag for controlling the thread.
	*/
	int					m_ThreadControl;

	/*	Description		- Helper function to start the thread.
	 */
	static void *Dispatch(void *arg) {
		ThreadBase *pThread(static_cast<ThreadBase *>(arg));
		pThread->Run();
		return NULL;
	}

public:
	/*	Description		- Default constructor.
	 */
	explicit ThreadBase() : m_ThreadHandle (0), m_ThreadStatus(THREAD_STOP), m_ThreadControl(THREAD_STOP) {
		m_ThreadName[0] = 0 ;
		pthread_attr_init(&m_ThreadAttribute);
	}
	virtual ~ThreadBase() {
		pthread_attr_destroy(&m_ThreadAttribute);
	}
	/*	Description		- Return the thread's name
	 *		return pointer to the name.
	 */
	char* GetThreadName() {
		return m_ThreadName;
	}

	/*	Description		- Helper function to sleep on milisecond.
	 */
	void ThreadSleep(unsigned int milisecond) {
		if(milisecond>=1000)
			sleep(milisecond/1000);
		else
			usleep(milisecond*1000);
	}
	/*	Description		- Start the thread.
	 *	Note			- Call back local function Dispatch() which will call Run() method.
	 *		return true for successfully start thread.
	 *		return false for failure.
	 */
	virtual bool Start(const char* thread_name) {
		if(pthread_create(&m_ThreadHandle, &m_ThreadAttribute, &Dispatch, this)!=0)
				m_ThreadHandle = (pthread_t)NULL;

			if (!m_ThreadHandle) {
				std::cerr << "BaseThread: failed to Start." << std::endl;
				return false;
			}
			//Give it a name 
			strcpy(m_ThreadName,thread_name);
			m_ThreadControl		= THREAD_RUN;
			return true;
	}
	/*	Description		- Function in the Run() loop, for user specified thread's tasks.
	 * 	Note			- MUST be overridden.
	 *		return nothing.
	 */
	virtual void OnRun() {
		//Put your tasks here:
		std::cout << GetThreadName() <<" says: Hello" << std::endl;
	}
	/*	Description		- The main loop of the thread. The loop will be broken by Stop() method.
	 *		return nothing.
	 */
	virtual void Run() {
		while(m_ThreadControl != THREAD_STOP) {
			m_ThreadStatus = THREAD_RUN;
			OnRun();
		}
		m_ThreadStatus = THREAD_STOP;
	}
	/*	Description		- Stop the thread. Used to break the Run() loop.
	 *		return nothing.
	 */
	virtual void Stop(int iTimeOut) {
		while(m_ThreadStatus != THREAD_STOP) {
			m_ThreadControl = THREAD_STOP;
			if(iTimeOut<=0) {
				pthread_cancel(m_ThreadHandle);
				break;
			}
			ThreadSleep(100);
			iTimeOut -= 100;
		}
		pthread_attr_destroy(&m_ThreadAttribute);
		m_ThreadHandle = (pthread_t)NULL;
	}
	/*	Description		- Pause the thread. Resume with Resume() method.
	 * 	Note			- Overriding is NOT encouraging.
	 *		return nothing.
	 */
	virtual void Pause() {
		if (m_ThreadStatus == THREAD_RUN) {
			m_ThreadControl = 	THREAD_PAUSE;
			m_ThreadStatus 	=	THREAD_PAUSE;
		}
	}
	/*	Description		- Resume the paused thread.
	 * 	Note			- Overriding is NOT encouraging.
	 *		return nothing.
	 */
	virtual void Resume() {
		if (m_ThreadStatus == THREAD_PAUSE) {
			m_ThreadControl	= THREAD_RUN;
		}
	}
	/*	Description		- Called from outside to get the thread status.
	 *	Note			- Overriding is NOT encouraging.
	 *		return the current status.
	 */
	int	GetThreadStatus() {
		return m_ThreadStatus;
	}
	/*	Description		- Called from outside for waiting the thread to run. Timeout in milisecond
	 *  	Note			- The default timeout is: 0 (INFINITY).
	 *		  		- If the timeout is set, should call GetThreadStatus() to make sure thread is running.
	 * 				- Overriding is NOT encouraging.
	 *		return nothing.
	 */
	void WaitThreadReady(int iTimeOut = 0) {
		while (m_ThreadStatus != THREAD_RUN){
			ThreadSleep(30);
			if (iTimeOut == 0)
				continue;
			iTimeOut -= 30;
			if(iTimeOut<=0)
				return;
		}
		return;
	}
};



#endif /* THREADBASE_H_ */

