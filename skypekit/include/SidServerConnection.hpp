#ifndef Sid_SidServerConnection_INCLUDED_HPP
#define Sid_SidServerConnection_INCLUDED_HPP

#ifdef _WINRT
#include "SidServerConnectionWinRt.hpp"
#elif defined (_WIN32)
#include "SidServerConnectionWin.hpp"
#elif defined (_SYMBIAN)
#include "SidServerConnectionSym.hpp"
#else
#include "SidServerConnectionUnx.hpp"
#endif

#endif
