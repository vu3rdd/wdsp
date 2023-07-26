/*  linux_port.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013 Warren Pratt, NR0V and John Melton, G0ORX/N6LYT

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The author can be reached by email at  

warren@wpratt.com
john.d.melton@googlemail.com

*/

#include <errno.h>

#include "linux_port.h"
#include "comm.h"

/********************************************************************************************************
*													*
*	Linux Port Utilities										*
*													*
********************************************************************************************************/

#if defined(linux) || defined(__APPLE__)

void QueueUserWorkItem(void *function,void *context,int flags) {
	pthread_t t;
	pthread_create(&t, NULL, function, context);
	pthread_join(t, NULL);
}

void InitializeCriticalSectionAndSpinCount(pthread_mutex_t *mutex,int count) {
	pthread_mutexattr_t mAttr;
	pthread_mutexattr_init(&mAttr);
#ifdef __APPLE__
	// DL1YCF: MacOS X does not have PTHREAD_MUTEX_RECURSIVE_NP
	pthread_mutexattr_settype(&mAttr,PTHREAD_MUTEX_RECURSIVE);
#else
	pthread_mutexattr_settype(&mAttr,PTHREAD_MUTEX_RECURSIVE_NP);
#endif
	pthread_mutex_init(mutex,&mAttr);
	pthread_mutexattr_destroy(&mAttr);
	// ignore count
}

void EnterCriticalSection(pthread_mutex_t *mutex) {
	pthread_mutex_lock(mutex);
}

void LeaveCriticalSection(pthread_mutex_t *mutex) {
	pthread_mutex_unlock(mutex);
}

void DeleteCriticalSection(pthread_mutex_t *mutex) {
	pthread_mutex_destroy(mutex);
}

int LinuxWaitForSingleObject(sem_t *sem,int ms) {
	int result=0;
	if(ms==INFINITE) {
		// wait for the lock
		result=sem_wait(sem);
	} else {
		// try to get the lock
		result=sem_trywait(sem);
		if(result!=0) {
			// didn't get the lock
			if(ms!=0) {
				// sleep if ms not zero
				Sleep(ms);
				// try to get the lock again
				result=sem_trywait(sem);
			}
		}
	}
	
	return result;
}

sem_t *LinuxCreateSemaphore(int attributes,int initial_count,int maximum_count,char *name) {
        sem_t *sem;
#ifdef __APPLE__
        //
        // DL1YCF
	// This routine is usually invoked with name=NULL, so we have to make
	// a unique name of tpye WDSPxxxxxxxx for each invocation, since MacOS only
        // supports named semaphores. We shall unlink in due course, but first we
        // need to check whether the name is possibly already in use, e.g. by
        // another SDR program running on the same machine.
        //
	static long semcount=0;
	char sname[20];
        for (;;) {
          sprintf(sname,"WDSP%08ld",semcount++);
          sem=sem_open(sname, O_CREAT | O_EXCL, 0700, initial_count);
          if (sem == SEM_FAILED && errno == EEXIST) continue;
          break;
        }
	if (sem == SEM_FAILED) {
	  perror("WDSP:CreateSemaphore");
	}
        //
        // we can unlink the semaphore NOW. It will remain functional
        // until sem_close() has been called by all threads using that
        // semaphore.
        //
	sem_unlink(sname);
#else
        sem=malloc(sizeof(sem_t));
	int result;
        // DL1YCF: added correct initial count
	result=sem_init(sem, 0, initial_count);
        if (result < 0) {
	  perror("WDSP:CreateSemaphore");
        }
#endif
	return sem;
}

void LinuxReleaseSemaphore(sem_t* sem,int release_count, int* previous_count) {
	//
	// Note WDSP always calls this with previous_count==NULL
        // so we do not bother about obtaining the previous value and 
        // storing it in *previous_count.
        //
	while(release_count>0) {
		sem_post(sem);
		release_count--;
	}
}

sem_t *CreateEvent(void* security_attributes,int bManualReset,int bInitialState,char* name) {
	//
	// From within WDSP, this is always called with bManualReset = bInitialState = FALSE
	//
        sem_t *sem;
	sem=LinuxCreateSemaphore(0,0,0,0);
	return sem;
}

void LinuxSetEvent(sem_t* sem) {
	sem_post(sem);
}

HANDLE _beginthread( void( __cdecl *start_address )( void * ), unsigned stack_size, void *arglist) {
	pthread_t threadid;
	pthread_attr_t  attr;

	if (pthread_attr_init(&attr)) {
 	    return (HANDLE)-1;
	}
      
	if(stack_size!=0) {
	    if (pthread_attr_setstacksize(&attr, stack_size)) {
	        return (HANDLE)-1;
	    }
	}

        if(pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED)) {
            return (HANDLE)-1;
        }
     
	if (pthread_create(&threadid, &attr, (void*(*)(void*))start_address, arglist)) {
	     return (HANDLE)-1;
	}

        //pthread_attr_destroy(&attr);
#ifndef __APPLE__
	// DL1YCF: this function does not exist on MacOS. You can only name the
        //         current thread.
       //          If this call should fail, continue anyway.
        (void) pthread_setname_np(threadid, "WDSP");
#endif

	return (HANDLE)threadid;

}

void _endthread() {
	pthread_exit(NULL);
}

void SetThreadPriority(HANDLE thread, int priority)  {
//
// In Linux, the scheduling priority only affects
// real-time threads (SCHED_FIFO, SCHED_RR), so this
// is basically a no-op here.
//
/*
	int policy;
	struct sched_param param;

	pthread_getschedparam(thread, &policy, &param);
	param.sched_priority = sched_get_priority_max(policy);
	pthread_setschedparam(thread, policy, &param);
*/
}

void CloseHandle(HANDLE hObject) {
//
// This routine is *ONLY* called to release semaphores
// The WDSP transmitter thread terminates upon each TX/RX
// transition, where it closes and re-opens a semaphore
// in flush_buffs() in iobuffs.c. Therefore, we have to
// release any resource associated with this semaphore, which
// may be a small memory patch (LINUX) or a file descriptor
// (MacOS).
//
#ifdef __APPLE__
if (sem_close((sem_t *)hObject) < 0) {
  perror("WDSP:CloseHandle:SemCLose");
}
#else
if (sem_destroy((sem_t *)hObject) < 0) {
  perror("WDSP:CloseHandle:SemDestroy");
} else {
  // if sem_destroy failed, do not release storage
  free(hObject);
}
#endif

return;
}

#endif
