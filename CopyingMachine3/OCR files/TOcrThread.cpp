//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TocrThread.h"
//---------------------------------------------------------------------------
TOcrThread::TOcrThread(bool pbCreateSuspended) {

	ReturnValue = 0;
	hThread = NULL;
	bTerminate = false;
	bCompleted = false;
	bSuspended = pbCreateSuspended;

	bTerminate = !InitializeCriticalSectionAndSpinCount(&TerminateCriticalSection, 0x80000400);

	//Create a thread only if the critical section is initialized
	if (!bTerminate) {

		hThread = CreateThread(
			NULL,				// default security attributes
			0,					// use default stack size  
			&ThreadExecution,     // thread function 
			this,				// argument to thread function 
			CREATE_SUSPENDED,	// use default creation flags 
			NULL);				// returns the thread identifier 
			}

	if (!pbCreateSuspended) Resume();
	}

TOcrThread::~TOcrThread() {

	//Terminate the thread
	Terminate();

	//Wait for the thread to finish.
	if (!bSuspended) WaitForSingleObject(hThread, INFINITE);

	//Close the thread
	if (hThread!=NULL) CloseHandle(hThread);

	//Delete the critical section
	DeleteCriticalSection(&TerminateCriticalSection);
}

//This function suspends the execution of this thread
void __fastcall 
TOcrThread::Suspend() {

	if (hThread!=NULL) {
		SuspendThread(hThread);
		bSuspended = true;
		}	
	}

//This function resumes/starts the execution of this thread
void __fastcall 
TOcrThread::Resume() {

	if (hThread!=NULL) {
		ResumeThread(hThread);
		bSuspended = false;
		}
	}

//This function terminates this thread
void __fastcall 
TOcrThread::Terminate() {

	EnterCriticalSection(&TerminateCriticalSection); 

	bTerminate = true;

	LeaveCriticalSection(&TerminateCriticalSection); 
	}

//This function sets the completion flag of this thread
void __fastcall 
TOcrThread::Completed() {

	bCompleted = true;
	}

//This function waits for the completion of the thread
int __fastcall 
TOcrThread::WaitFor() {

	while (!bCompleted && !bTerminate) {

		Sleep(100);
		}

	return ReturnValue;
	}

//This function returns true if the thread is terminating
bool __fastcall 
TOcrThread::IsTerminating() {

	return bTerminate;
	}

//This function returns true if the thread is terminating
bool __fastcall 
TOcrThread::IsCompleted() {

	return bCompleted;
	}

//
void __fastcall 
TOcrThread::Execute() {




	}

//This is the thread execution function. It calls the execution method of the given thread
DWORD 
WINAPI ThreadExecution(LPVOID lpParameter) {

	TOcrThread* oThread;

	oThread = (TOcrThread*)lpParameter;

	oThread->Execute();

	oThread->Completed();

	return 0;
	}

    
