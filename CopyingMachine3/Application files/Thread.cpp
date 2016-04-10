//---------------------------------------------------------------------------
#include "stdafx.h"
#include "thread.h"
//---------------------------------------------------------------------------
CThread::CThread(bool pbCreateSuspended) {

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
			&BaseThreadExecution,     // thread function 
			this,				// argument to thread function 
			CREATE_SUSPENDED,	// use default creation flags 
			NULL);				// returns the thread identifier 
			}

	if (!pbCreateSuspended) Resume();
	}

CThread::~CThread() {

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
CThread::Suspend() {

	if (hThread!=NULL) {
		SuspendThread(hThread);
		bSuspended = true;
		}	
	}

//This function resumes/starts the execution of this thread
void __fastcall 
CThread::Resume() {

	if (hThread!=NULL) {
		ResumeThread(hThread);
		bSuspended = false;
		}
	}

//This function terminates this thread
void __fastcall 
CThread::Terminate() {

	EnterCriticalSection(); 

	bTerminate = true;

	LeaveCriticalSection(); 
	}

//This function sets the completion flag of this thread
void __fastcall 
CThread::Completed() {

	bCompleted = true;
	}

//This function waits for the completion of the thread
int __fastcall 
CThread::WaitFor() {

	while (!bCompleted) {

		Sleep(100);
		}

	return ReturnValue;
	}

//This function returns true if the thread is terminating
bool __fastcall 
CThread::IsTerminating() {

	return bTerminate;
	}

//This function returns true if the thread is terminating
bool __fastcall 
CThread::IsCompleted() {

	return bCompleted;
	}

//Enter a critical section
void __fastcall 
CThread::EnterCriticalSection() {

	::EnterCriticalSection(&TerminateCriticalSection); 
	}

//Leave a critical section
void __fastcall 
CThread::LeaveCriticalSection() {

	::LeaveCriticalSection(&TerminateCriticalSection); 
	}

//
void __fastcall 
CThread::Execute() {




	}

//This is the thread execution function. It calls the execution method of the given thread
DWORD 
WINAPI BaseThreadExecution(LPVOID lpParameter) {

	CThread* oThread;

	oThread = (CThread*)lpParameter;

	oThread->Execute();

	oThread->Completed();

	return 0;
	}

    
