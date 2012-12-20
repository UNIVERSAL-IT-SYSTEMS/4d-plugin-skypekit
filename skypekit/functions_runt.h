#include "4DPluginAPI.h"
#include "4DPlugin.h"

unsigned int SKGetSkypeRuntimePid();
void SKTerminateProcess(unsigned int pid);
void SKCopyRuntimePath(C_TEXT &path);
void SKLaunchTask(C_TEXT &path);

// --- Runtime
void SKYPE_RUNTIME_LAUNCH(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_RUNTIME_Get_id(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_RUNTIME_TERMINATE(sLONG_PTR *pResult, PackagePtr pParams);
