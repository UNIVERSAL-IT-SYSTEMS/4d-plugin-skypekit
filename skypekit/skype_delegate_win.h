/*
 *  skype_delegate_win.h
 *  4D Plugin
 *
 *  Created by miyako on 11/01/03.
 *
 */

#ifndef __SKYPE_DELEGAT_WIN_H__
#define __SKYPE_DELEGAT_WIN_H__ 1

#include "4DPluginAPI.h"

#ifdef __cplusplus
extern "C" {
#endif

	class SKDelegate
	{
		
	private:
		
		static std::vector<CUTF8String> _notificationStrings;
		static unsigned int _capacity;
		static bool _isDelegateAttached;
		static bool _isAvailable;
		
		static HWND _skypekitDesktopDelegate;
		static HWND _skypekitDesktopConnection;
		static WNDPROC _skypekitDesktopDefaultWindowProc;
		
		static UINT _messageSkypeControlAPIAttach;
		static UINT _messageSkypeControlAPIDiscover;

		static LRESULT CALLBACK SkypeDelegateWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void disconnect();
		
	public:
			
		bool isAttached();	
		bool isAvailable();
		
		void connect();	

		void getNotificationStrings(ARRAY_TEXT &notificationStrings);
		void clearNotificationStrings();
		void sendSkypeCommand(C_TEXT &command);
		
		SKDelegate();
		~SKDelegate();
		
	};
	
#ifdef __cplusplus
}
#endif

#endif