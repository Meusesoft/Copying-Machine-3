//---------------------------------------------------------------------------

#ifndef CThreadH
#define CThreadH
//---------------------------------------------------------------------------
DWORD WINAPI BaseThreadExecution(LPVOID lpParameter);

class CThread
{            
public:
    CThread(bool pbCreateSuspended);
    ~CThread();

	void __fastcall Suspend();
	void __fastcall Resume();
	void __fastcall Terminate();
	void __fastcall Completed();
	bool __fastcall IsTerminating();
	bool __fastcall IsCompleted();
	int __fastcall  WaitFor();



	virtual void __fastcall Execute();

	int ReturnValue;

protected:

	void __fastcall EnterCriticalSection();
	void __fastcall LeaveCriticalSection();
   
    
private:

	HANDLE hThread;

	CRITICAL_SECTION TerminateCriticalSection;

	bool bTerminate;
	bool bCompleted;
	bool bSuspended;
};
//---------------------------------------------------------------------------
#endif
