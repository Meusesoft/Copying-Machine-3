//---------------------------------------------------------------------------

#ifndef TOcrThreadH
#define TOcrThreadH
//---------------------------------------------------------------------------
DWORD WINAPI ThreadExecution(LPVOID lpParameter);

class TOcrThread
{            
public:
    TOcrThread(bool pbCreateSuspended);
    ~TOcrThread();

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
   
    
private:

	HANDLE hThread;

	CRITICAL_SECTION TerminateCriticalSection;

	bool bTerminate;
	bool bCompleted;
	bool bSuspended;
};
//---------------------------------------------------------------------------
#endif
