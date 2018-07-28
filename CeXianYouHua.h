// CeXianYouHua.h : main header file for the CEXIANYOUHUA application
//

#if !defined(AFX_CEXIANYOUHUA_H__355D59A2_0F24_474C_8145_AB66DADA769D__INCLUDED_)
#define AFX_CEXIANYOUHUA_H__355D59A2_0F24_474C_8145_AB66DADA769D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaApp:
// See CeXianYouHua.cpp for the implementation of this class
//

class CCeXianYouHuaApp : public CWinApp
{
public:
	CCeXianYouHuaApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeXianYouHuaApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CCeXianYouHuaApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEXIANYOUHUA_H__355D59A2_0F24_474C_8145_AB66DADA769D__INCLUDED_)
