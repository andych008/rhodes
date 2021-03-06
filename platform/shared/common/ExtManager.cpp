/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#include "ExtManager.h"
#include "rubyext/WebView.h"
#include <common/RhodesApp.h>

#if defined (WINDOWS_PLATFORM)
#define IDM_NAVIGATE                    40022
#define IDM_EXECUTEJS                   40033
extern "C" HWND getMainWnd();
extern "C" HINSTANCE rho_wmimpl_get_appinstance();
extern "C" bool rho_webview_exist_javascript(const wchar_t* szJSFunction, int index);
#endif

namespace rho {
namespace common {

IMPLEMENT_LOGCLASS(CExtManager, "ExtManager");

void CExtManager::registerExtension(const String& strName, IRhoExtension* pExt)
{
    m_hashExtensions.put(strName, pExt);
}

IRhoExtension* CExtManager::getExtByName(const String& strName)
{
    return m_hashExtensions.get(strName);
}

CRhoExtData CExtManager::makeExtData()
{
    CRhoExtData oData;
#if defined (OS_WINDOWS)
    oData.m_hWnd = getMainWnd();
    oData.m_hInstance = rho_wmimpl_get_appinstance();
#endif

    oData.m_iTabIndex = rho_webview_active_tab();
    return oData;
}

void CExtManager::onSetPropertiesData( const wchar_t* pPropID, const wchar_t* pData)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        (it->second)->onSetPropertiesData( pPropID, pData, makeExtData() );
    }
}

void CExtManager::onUnhandledProperty( const wchar_t* pModuleName, const wchar_t* pName, const wchar_t* pValue, const CRhoExtData& oExtData )
{
    rho::common::IRhoExtension* pExt = getExtByName( rho::common::convertToStringA(pModuleName) );
    if (pExt)
        pExt->onSetProperty( pName, pValue, oExtData );
}

void CExtManager::onBeforeNavigate(const wchar_t* szUrlBeingNavigatedTo)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        (it->second)->onBeforeNavigate( szUrlBeingNavigatedTo, makeExtData() );
    }
}

void CExtManager::onNavigateComplete(const wchar_t* szUrlBeingNavigatedTo)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        (it->second)->onNavigateComplete( szUrlBeingNavigatedTo, makeExtData() );
    }
}

void CExtManager::onDocumentComplete(const wchar_t* szUrlOfDocument)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        (it->second)->onNavigateComplete( szUrlOfDocument, makeExtData() );
    }
}

void CExtManager::close()
{
   m_hashExtensions.clear();
}

void CExtManager::executeRubyCallback( const char* szCallback, const char* szCallbackBody, const char* szCallbackData, bool bWaitForResponse)
{
    RHODESAPP().callCallbackWithData(szCallback, szCallbackBody, szCallbackData, bWaitForResponse );
}

void CExtManager::navigate(const wchar_t* szUrl)
{
    ::PostMessage( getMainWnd(), WM_COMMAND, IDM_NAVIGATE, (LPARAM)_wcsdup(szUrl) );
}

bool CExtManager::existsJavascript(const wchar_t* szJSFunction)
{
    return rho_webview_exist_javascript(szJSFunction, rho_webview_active_tab());
}

void CExtManager::executeJavascript(const wchar_t* szJSFunction)
{
    ::PostMessage( getMainWnd(), WM_COMMAND, IDM_EXECUTEJS, (LPARAM)_wcsdup(szJSFunction) );
}

StringW CExtManager::getCurrentUrl()
{
    return convertToStringW(RHODESAPP().getCurrentUrl(rho_webview_active_tab()));
}

void CExtManager::stopNavigate()
{
}

void CExtManager::historyForward()
{
    rho_webview_navigate_forward();
}

void CExtManager::historyBack()
{
    rho_webview_navigate_back();
}

void CExtManager::refreshPage(bool bFromCache)
{
    rho_webview_refresh(rho_webview_active_tab());
}

void CExtManager::quiteApp()
{
}

void CExtManager::minimizeApp()
{
}

void CExtManager::restoreApp()
{
}

void CExtManager::resizeBrowserWindow(RECT rc)
{
}

void CExtManager::zoomPage(float fZoom)
{
}

void CExtManager::zoomText(int nZoom)
{
}

extern "C" unsigned long rb_require(const char *fname);

void CExtManager::requireRubyFile( const char* szFilePath )
{
    rb_require(szFilePath);
}

void CExtManager::rhoLog(int nSeverity, const char* szModule, const char* szMsg, const char* szFile, int nLine)
{
    rhoPlainLog(szFile, nLine, nSeverity, szModule, szMsg);
}

bool CExtManager::onWndMsg(MSG& oMsg)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        if ( (it->second)->onWndMsg( oMsg ) )
            return true;
    }

    return false;
}

long CExtManager::OnNavigateTimeout(const wchar_t* szUrlBeingNavigatedTo)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        long lRes = (it->second)->OnNavigateTimeout( szUrlBeingNavigatedTo, makeExtData() );
        if ( lRes )
            return lRes;
    }

    return 0;
}

long CExtManager::OnSIPState(bool bSIPState)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        long lRes = (it->second)->OnSIPState( bSIPState, makeExtData() );
        if ( lRes )
            return lRes;
    }

    return 0;
}

long CExtManager::OnAlertPopup(int nEnum, void* pData)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        long lRes = (it->second)->OnAlertPopup( nEnum, pData, makeExtData() );
        if ( lRes )
            return lRes;
    }

    return 0;
}

long CExtManager::OnNavigateError(const wchar_t* szUrlBeingNavigatedTo)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        long lRes = (it->second)->OnNavigateError( szUrlBeingNavigatedTo, makeExtData() );
        if ( lRes )
            return lRes;
    }

    return 0;
}

void CExtManager::OnAppActivate(bool bActivate)
{
    for ( HashtablePtr<String, IRhoExtension*>::iterator it = m_hashExtensions.begin(); it != m_hashExtensions.end(); ++it )
    {
        (it->second)->OnAppActivate( bActivate, makeExtData() );
    }
}

} //namespace common
} //namespace rho

