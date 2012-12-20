#include "4DPluginAPI.h"
#include "4DPlugin.h"

void SKDisconnect();

// --- Desktop API
void SKYPE_DESKTOP_Connect(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_DESKTOP_Send_command(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_DESKTOP_Disconnect(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_DESKTOP_Is_connected(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_DESKTOP_Get_messages(sLONG_PTR *pResult, PackagePtr pParams);

// --- Desktop
void SKYPE_DESKTOP_LAUNCH(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_DESKTOP_Get_id(sLONG_PTR *pResult, PackagePtr pParams);
void SKYPE_DESKTOP_TERMINATE(sLONG_PTR *pResult, PackagePtr pParams);
