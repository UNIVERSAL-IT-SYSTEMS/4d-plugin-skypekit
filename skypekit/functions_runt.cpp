#include "functions_runt.h"

#if VERSIONMAC
#include <sys/sysctl.h>
#define skypekitPluginName @"com.4D.4DPlugin.miyako.skypekit"
#define skypekitRuntimeName @"mac-x86-skypekit"
#define skypekitRuntimeNameC "mac-x86-skypekit"
#else
#include <tlhelp32.h>
#define skypekitPluginName L"skypekit.4DX"
#define skypekitRuntimeName L"windows-x86-skypekit"
#define skypekitRuntimeNameW L"windows-x86-skypekit.exe"
#endif

#pragma mark -

unsigned int SKGetSkypeRuntimePid(){
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
			
			if(comm.compare(skypekitRuntimeNameC) == 0){
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
	
	std::vector <std::wstring> paths;
	std::vector <std::wstring> names;
	std::vector <int> processes;
	std::wstring runtimeName = skypekitRuntimeNameW;
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
								
								if(process_name == runtimeName){
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
	
	return pid;
#endif	
}

void SKTerminateProcess(unsigned int pid){
	if(pid)
	{	
#if VERSIONMAC	
		killpg(getpgid(pid), SIGTERM);
#else
		HANDLE h = OpenProcess(PROCESS_TERMINATE, true, pid);
		if(h) TerminateProcess(h, 0);
#endif	
	}
}

void SKCopyRuntimePath(C_TEXT &path){
#if VERSIONMAC	
	NSBundle *thisBundle = [NSBundle bundleWithIdentifier:skypekitPluginName];
	
	if(thisBundle)
	{
		path.setUTF16String([[[[thisBundle bundlePath] 
							stringByAppendingPathComponent:@"Contents"]
							stringByAppendingPathComponent:@"MacOS"]
							stringByAppendingPathComponent:skypekitRuntimeName]);
	}
	
#else

wchar_t	fDrive[_MAX_DRIVE],
fDir[_MAX_DIR],
fName[_MAX_FNAME],
fExt[_MAX_EXT];

wchar_t	dllPath[_MAX_PATH] = {0};
wchar_t thisPath[_MAX_PATH] = {0};
wchar_t windowsPath[_MAX_PATH] = {0};

HMODULE hplugin = GetModuleHandleW(skypekitPluginName);
GetModuleFileNameW(hplugin, thisPath, _MAX_PATH);

_wsplitpath_s(thisPath, fDrive, fDir, fName, fExt);

std::wstring p = fDrive;
p += fDir;//path to plugin parent folder

if(p.at(p.size() - 1) == L'\\')//remove delimiter
p = p.substr(0, p.size() - 1);

_wsplitpath_s(p.c_str(), fDrive, fDir, fName, fExt);
_wmakepath_s(windowsPath, fDrive, fDir, L"Windows\\", NULL);//the exe is 32 bits

_wsplitpath_s(windowsPath, fDrive, fDir, fName, fExt);	
_wmakepath_s(dllPath, fDrive, fDir, skypekitRuntimeName, L"exe");

path.setUTF16String((PA_Unichar *)dllPath, wcslen(dllPath));

#endif
}

void SKLaunchTask(C_TEXT &path){
#if VERSIONMAC	
	
	NSString *launchPath = path.copyUTF16String();
	
	if(launchPath)
		[NSTask launchedTaskWithLaunchPath:launchPath arguments:[NSArray array]];
	
#else
	
	PROCESS_INFORMATION processInformation = {0};
	STARTUPINFO         startupInfo = {0};
	
	startupInfo.cb = sizeof(STARTUPINFO);
	
	CreateProcess((LPCTSTR)path.getUTF16StringPtr(), 
					 NULL, 
					 NULL, 
					 NULL,
					 FALSE,
					 CREATE_NO_WINDOW,
					 NULL,
					 NULL,
					 &startupInfo,
					 &processInformation
					 );
#endif
}

#pragma mark -

// ------------------------------------ Runtime -----------------------------------

void SKYPE_RUNTIME_LAUNCH(sLONG_PTR *pResult, PackagePtr pParams)
{	
	if(!SKGetSkypeRuntimePid())
	{	
		C_TEXT path;
		SKCopyRuntimePath(path);
		SKLaunchTask(path);
	}
}

void SKYPE_RUNTIME_TERMINATE(sLONG_PTR *pResult, PackagePtr pParams)
{		
	SKTerminateProcess(SKGetSkypeRuntimePid());
}

void SKYPE_RUNTIME_Get_id(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	
	returnValue.setIntValue(SKGetSkypeRuntimePid());
	returnValue.setReturn(pResult);
}