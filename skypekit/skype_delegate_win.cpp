/*
 *  skype_delegate_win.cpp
 *  4D Plugin
 *
 *  Created by miyako on 12/12/20.
 *
 */

#include "skype_delegate_win.h"

std::vector<CUTF8String> SKDelegate::_notificationStrings;
unsigned int SKDelegate::_capacity = 8192;
bool SKDelegate:: _isDelegateAttached = false;
bool SKDelegate::_isAvailable = false;
		
HWND SKDelegate::_skypekitDesktopDelegate = 0;
HWND SKDelegate::_skypekitDesktopConnection = 0;
WNDPROC SKDelegate::_skypekitDesktopDefaultWindowProc = 0;
		
UINT SKDelegate::_messageSkypeControlAPIAttach = 0;
UINT SKDelegate:: _messageSkypeControlAPIDiscover = 0;

bool SKDelegate::isAttached()
{
	return _isDelegateAttached;
}

bool SKDelegate::isAvailable()
{
	return _isAvailable;
}

LRESULT CALLBACK SKDelegate::SkypeDelegateWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == _messageSkypeControlAPIAttach)
	{
		switch (lParam) 
			{
				case 1:				//SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION
				case 0x8001:	//SKYPECONTROLAPI_ATTACH_API_AVAILABLE
					_isAvailable = true;
					break;
				case 0:			//SKYPECONTROLAPI_ATTACH_SUCCESS
					_skypekitDesktopConnection = (HWND)wParam;
					_isDelegateAttached = true;
					_isAvailable = true;
					break;					
				case 2:			//SKYPECONTROLAPI_ATTACH_REFUSED
					_isDelegateAttached = false;
					break;
				case 3:			//SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE
					_isAvailable = false;
					break;
				default:			
					break;
			}
			return 1;	
	}

	switch (message) 
	{			
		case WM_COPYDATA:
			if(_skypekitDesktopConnection)
			{
				if(wParam == (WPARAM)_skypekitDesktopConnection)
				{
					PCOPYDATASTRUCT data = (PCOPYDATASTRUCT)lParam;
					if(data->cbData)
					{
						CUTF8String notificationString = CUTF8String((uint8_t *)data->lpData, data->cbData);
						
						if(_notificationStrings.size() == _capacity)
							_notificationStrings.erase(_notificationStrings.begin());
						
						_notificationStrings.push_back(notificationString);
					}
				}
			}
			return 1;
		
		default:
			return CallWindowProc(_skypekitDesktopDefaultWindowProc, hWnd, message, wParam, lParam);
	}
}

void SKDelegate::connect()
{
	SetWindowLongPtr(_skypekitDesktopDelegate, GWLP_WNDPROC, (LONG_PTR)SkypeDelegateWndProc);	
	SendMessage(HWND_BROADCAST, _messageSkypeControlAPIDiscover, (WPARAM)_skypekitDesktopDelegate, 0);
}

void SKDelegate::disconnect()
{
	_skypekitDesktopConnection = 0;
	_isDelegateAttached = false;
	
	SetWindowLongPtr(_skypekitDesktopDelegate, GWLP_WNDPROC, (LONG_PTR)_skypekitDesktopDefaultWindowProc);
}

SKDelegate::SKDelegate()
{
	_messageSkypeControlAPIAttach = RegisterWindowMessage(L"SkypeControlAPIAttach");  
	_messageSkypeControlAPIDiscover = RegisterWindowMessage(L"SkypeControlAPIDiscover");	
	_skypekitDesktopDelegate = CreateWindowEx(0, L"Message", NULL, WS_CHILD, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	_skypekitDesktopDefaultWindowProc = (WNDPROC)GetWindowLongPtr(_skypekitDesktopDelegate, GCLP_WNDPROC);
}

void SKDelegate::getNotificationStrings(ARRAY_TEXT &notificationStrings)
{
	notificationStrings.setSize(0);//to clear element{0} for 4D array		
	notificationStrings.setSize(1);	
	
	for(size_t i = 0; i < _notificationStrings.size(); ++i) 
	{
		CUTF8String u = _notificationStrings.at(i);
		notificationStrings.appendUTF8String(&u);
	}

}

void SKDelegate::clearNotificationStrings()
{
	_notificationStrings.clear();
}

void SKDelegate::sendSkypeCommand(C_TEXT &command)
{
	if(this->isAttached())
	{
		CUTF8String u;
		command.copyUTF8String(&u);

		COPYDATASTRUCT data;
		data.cbData = u.size() + 1;
		data.dwData = 0;
		data.lpData = (PVOID)u.c_str();
		
		if(_skypekitDesktopConnection) SendMessage(_skypekitDesktopConnection, WM_COPYDATA, (WPARAM)_skypekitDesktopDelegate, (LPARAM)&data);
	}
}

SKDelegate::~SKDelegate()
{ 
	this->disconnect();
	
	DestroyWindow(_skypekitDesktopDelegate);
}
