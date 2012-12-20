#include "functions_desk.h"

#if VERSIONMAC
#include "skype_delegate.h"
SKDelegate *skypekitDesktopDelegate = nil;
#include <sys/sysctl.h>
#define skypekitDesktopName @"Skype"
#define skypekitDesktopNameC "Skype"
#else
#include "skype_delegate_win.h"
SKDelegate *skypekitDesktopDelegate = 0;
#include <tlhelp32.h>
#define skypekitDesktopPathRegistryKey L"Software\\Skype\\Phone"
#define skypekitDesktopPathRegistryValue L"SkypePath"
wchar_t skypekitDesktopPath[_MAX_PATH] = {0}; 
#endif

#define skypekitDesktopDelegateName "4D"

#pragma mark -
#pragma mark error codes
#pragma mark -

#define SKYPEKIT_ERROR_DESKTOP_NOT_CONNECTED	1
#define SKYPEKIT_ERROR_DESKTOP_NOT_ATTACHED		2
#define SKYPEKIT_ERROR_DESKTOP_NOT_RUNNING		3
#define SKYPEKIT_ERROR_DESKTOP_NOT_AVAILABLE	4

#pragma mark -
#pragma mark platform neutral functions
#pragma mark -

#if VERSIONWIN
unsigned int SKGetDesktopPathLength()
{
	if(!wcslen(skypekitDesktopPath))
	{
		HKEY hKey;
		DWORD size = _MAX_PATH;
		DWORD dwType = REG_SZ;
		
		//first query current user
		if(RegOpenKey(HKEY_CURRENT_USER, skypekitDesktopPathRegistryKey, &hKey) == ERROR_SUCCESS)
			if(RegQueryValueEx(hKey,skypekitDesktopPathRegistryValue, 0, &dwType, (LPBYTE)skypekitDesktopPath, &size) != ERROR_SUCCESS)
				//if error, query local machine
				if(RegOpenKey(HKEY_LOCAL_MACHINE, skypekitDesktopPathRegistryKey, &hKey) == ERROR_SUCCESS)
					RegQueryValueEx(hKey,skypekitDesktopPathRegistryValue, 0, &dwType, (LPBYTE)skypekitDesktopPath, &size);
					}
	
	return wcslen(skypekitDesktopPath);
}
#endif

unsigned int SKGetSkypeDesktopPid(){
#if VERSIONMAC	
	unsigned int pid = 0;
	
	size_t length = 0;
	size_t procCount = 0;
	
	struct kinfo_proc *kp;
	
	unsigned int i = 0;	
	static const int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
	
	int err = sysctl((int *)name, 
					 (sizeof(name)/sizeof(*name))-1,
					 NULL, 
					 &length,
					 NULL, 
					 0);
	
	if (err == 0) {
		
		std::vector<uint8_t> buf(length);	
		
		err = sysctl((int *)name, 
					 (sizeof(name)/sizeof(*name))-1,
					 &buf[0], 
					 &length,
					 NULL, 
					 0);
		
		kp = (kinfo_proc *)&buf[0];
		procCount = length/sizeof(kinfo_proc);
		
		for (i = 0; i < procCount; i++) {
			std::string comm = std::string(kp->kp_proc.p_comm);	
			
			if(comm.compare(skypekitDesktopNameC) == 0){
				pid = kp->kp_proc.p_pid;
				i = procCount;
				break;
			}
			
			kp++;	
			
		}
		
	}
	
	return pid;
#else
	unsigned int pid = 0;
		
	if(SKGetDesktopPathLength())
	{	
		std::vector <std::wstring> paths;
		std::vector <std::wstring> names;
		std::vector <int> processes;
		std::wstring desktopPath = skypekitDesktopPath;
		bool done = false;
		
		HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
		PROCESSENTRY32 pe32;
		
		HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
		MODULEENTRY32 me32;
		
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		
		if(hProcessSnap != INVALID_HANDLE_VALUE){
			pe32.dwSize = sizeof(PROCESSENTRY32);
			
			if(Process32First(hProcessSnap, &pe32)){
				do{
					hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
					std::wstring process_name = pe32.szExeFile;
					if(hModuleSnap != INVALID_HANDLE_VALUE){
						me32.dwSize = sizeof(MODULEENTRY32);
						
						if(Module32First(hModuleSnap, &me32)){
							do{
								std::wstring module_name = me32.szModule;
								
								if(module_name == process_name){
									std::wstring exe_path = me32.szExePath;
									if(exe_path == desktopPath){
										pid = me32.th32ProcessID;
										done = true;
									}
								}
								
							}while(Module32Next(hModuleSnap, &me32) && (!done));
							CloseHandle(hModuleSnap);
						}
					}
					
				}while(Process32Next(hProcessSnap, &pe32) && (!done));	
				CloseHandle(hProcessSnap);			 
			}
		}	
	}
		
	return pid;
#endif	
}

void SKLaunchDesktop(){
#if VERSIONMAC
	//[[NSWorkspace sharedWorkspace]launchApplication:skypekitDesktopName];
	//use applescript instead, to keep 4D in the foreground
	NSDictionary *errInfo;
	NSAppleScript *script = [[NSAppleScript alloc]initWithSource:[NSString stringWithFormat:@"tell application \"%@\" to launch", skypekitDesktopName]];	
	[script executeAndReturnError:&errInfo];
	[script release];	
#else
	if(SKGetDesktopPathLength()) ShellExecute(NULL, L"open", skypekitDesktopPath, NULL, NULL, SW_SHOWNOACTIVATE);
#endif
}

void SKTerminateDesktop(){
#if VERSIONMAC
	NSDictionary *errInfo;
	NSAppleScript *script = [[NSAppleScript alloc]initWithSource:[NSString stringWithFormat:@"tell application \"%@\" to quit", skypekitDesktopName]];	
	[script executeAndReturnError:&errInfo];
	[script release];
#else
	HANDLE h = OpenProcess(PROCESS_TERMINATE, true, SKGetSkypeDesktopPid());
		if(h) TerminateProcess(h, 0);
#endif	
}

void SKSetDefaultClientApplicationName(C_TEXT &name)
{
	name.setUTF8String((const uint8_t *)skypekitDesktopDelegateName, strlen(skypekitDesktopDelegateName));
}

int SKCheck()
{
	int status = 0;
#if VERSIONMAC	
	if(skypekitDesktopDelegate){	
		if([skypekitDesktopDelegate isAttached]){
			if([SKDelegate isSkypeRunning]){
				if(![SKDelegate isSkypeAvailable]) status = SKYPEKIT_ERROR_DESKTOP_NOT_AVAILABLE;
			}else{status = SKYPEKIT_ERROR_DESKTOP_NOT_RUNNING;}	
		}else{status = SKYPEKIT_ERROR_DESKTOP_NOT_ATTACHED;}	
	}else{status = SKYPEKIT_ERROR_DESKTOP_NOT_CONNECTED;}
#else
	if(skypekitDesktopDelegate){
		if(skypekitDesktopDelegate->isAttached()){
			if(SKGetSkypeDesktopPid() != 0){
				if(!skypekitDesktopDelegate->isAvailable()) status = SKYPEKIT_ERROR_DESKTOP_NOT_AVAILABLE;
			}else{status = SKYPEKIT_ERROR_DESKTOP_NOT_RUNNING;}
		}else{status = SKYPEKIT_ERROR_DESKTOP_NOT_ATTACHED;}	
	}else{status = SKYPEKIT_ERROR_DESKTOP_NOT_CONNECTED;}	
#endif	
	return status;
}

int SKConnect(C_TEXT &name)
{
	int err = 0;
	
	if(!name.getUTF16Length()) SKSetDefaultClientApplicationName(name);	
	
#if VERSIONMAC	
	//create delegate if it doesn't exist already
	if(!skypekitDesktopDelegate)
	{
		NSString *clientApplicationName = name.copyUTF16String();
		skypekitDesktopDelegate = [[SKDelegate alloc]initWithClientApplicationName:clientApplicationName capacity:8192];
		[clientApplicationName release];
	}
	//do nothing if we are already connected
	if(![skypekitDesktopDelegate isAttached])
	{
		if([SKDelegate isSkypeRunning])
		{
			if([SKDelegate isSkypeAvailable])
			{
				
				[SKDelegate setSkypeDelegate:skypekitDesktopDelegate];
				[SKDelegate connect];
				
			}else{err = SKYPEKIT_ERROR_DESKTOP_NOT_AVAILABLE;}			
		}else{err = SKYPEKIT_ERROR_DESKTOP_NOT_RUNNING;}	
	}
#else
	//create delegate if it doesn't exist already
	if(!skypekitDesktopDelegate)
		skypekitDesktopDelegate = new SKDelegate;
	
	//do nothing if we are already connected
	if(!skypekitDesktopDelegate->isAttached())
	{
		if(SKGetSkypeDesktopPid() != 0)
		{			
			//on windows, we can only find out api availability after we have posted a connection request
			skypekitDesktopDelegate->connect();

		}else{err = SKYPEKIT_ERROR_DESKTOP_NOT_RUNNING;}			
	}
#endif	
	return err;
}

void SKDisconnect()
{
#if VERSIONMAC	
	[SKDelegate removeSkypeDelegate];
	[SKDelegate disconnect];
	
	if(skypekitDesktopDelegate)
	{
		[skypekitDesktopDelegate release];
		skypekitDesktopDelegate = nil;	
	}	
#else
	if(skypekitDesktopDelegate)
	{
		delete skypekitDesktopDelegate;
		skypekitDesktopDelegate = 0;
	}
#endif	
}

bool SKIsConnected()
{
	bool isConnected = false;
#if VERSIONMAC
	if(skypekitDesktopDelegate)
		isConnected = [skypekitDesktopDelegate isAttached];	
#else
	if(skypekitDesktopDelegate)
		isConnected = skypekitDesktopDelegate->isAttached();		
#endif	
	return isConnected;
}

void SKGetMessages(ARRAY_TEXT &messages)
{
#if VERSIONMAC
	messages.setSize(0);//to clear element{0} for 4D array		
	messages.setSize(1);	
	if(skypekitDesktopDelegate)
	{
		NSArray *notificationStrings =[skypekitDesktopDelegate copyNotificationStrings];
		
		if(notificationStrings)
		{			
			for(unsigned int i = 0; i < [notificationStrings count]; ++i)
				messages.appendUTF16String([notificationStrings objectAtIndex:i]);
			
			[skypekitDesktopDelegate clearNotificationStrings];
			[notificationStrings release];	
		}
	}
#else
	if(skypekitDesktopDelegate)
	{
		skypekitDesktopDelegate->getNotificationStrings(messages);
		skypekitDesktopDelegate->clearNotificationStrings();
	}
#endif
}

void SKSendCommand(C_TEXT &request, C_TEXT &response)
{
#if VERSIONMAC
	NSString *command = request.copyUTF16String();
	response.setUTF16String([SKDelegate sendSkypeCommand:command]);
	[command release];
#else
	if(skypekitDesktopDelegate)
	{
		skypekitDesktopDelegate->sendSkypeCommand(request);
	}	
#endif
}

#pragma mark -

// ---------------------------------- Desktop API ---------------------------------

void SKYPE_DESKTOP_Connect(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_LONGINT returnValue;
	
	Param1.fromParamAtIndex(pParams, 1);

	returnValue.setIntValue(SKConnect(Param1));
	returnValue.setReturn(pResult);
}

void SKYPE_DESKTOP_Send_command(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_TEXT returnValue;
	
	Param1.fromParamAtIndex(pParams, 1);
	
	if(!SKCheck()) SKSendCommand(Param1, returnValue);
	
	returnValue.setReturn(pResult);
}

void SKYPE_DESKTOP_Disconnect(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	
	returnValue.setIntValue(SKCheck());
	
	if(!returnValue.getIntValue()) SKDisconnect();
	
	returnValue.setReturn(pResult);	
}

void SKYPE_DESKTOP_Is_connected(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;

	returnValue.setIntValue(SKIsConnected());
	returnValue.setReturn(pResult);
}

void SKYPE_DESKTOP_Get_messages(sLONG_PTR *pResult, PackagePtr pParams)
{
	ARRAY_TEXT Param1;
	C_LONGINT returnValue;
	
	returnValue.setIntValue(SKCheck());
	
	if(!returnValue.getIntValue()) SKGetMessages(Param1);
	
	Param1.toParamAtIndex(pParams, 1);
	returnValue.setReturn(pResult);
}

// ------------------------------------ Desktop -----------------------------------


void SKYPE_DESKTOP_LAUNCH(sLONG_PTR *pResult, PackagePtr pParams)
{
	SKLaunchDesktop();
}

void SKYPE_DESKTOP_Get_id(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	
	returnValue.setIntValue(SKGetSkypeDesktopPid());
	returnValue.setReturn(pResult);
}

void SKYPE_DESKTOP_TERMINATE(sLONG_PTR *pResult, PackagePtr pParams)
{
	SKDisconnect();
	SKTerminateDesktop();	
}
