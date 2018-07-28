#if !defined(AFX_SHOWCOORDINARY_H__6180060C_C18B_4CC8_BA0D_E79904498E46__INCLUDED_)
#define AFX_SHOWCOORDINARY_H__6180060C_C18B_4CC8_BA0D_E79904498E46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShowCoordinary.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShowCoordinary dialog

class CShowCoordinary : public CDialog
{
// Construction
public:
	CShowCoordinary(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShowCoordinary)
	enum { IDD = IDD_SHOWCOR };
	int		m_Height_Show;
	int		m_Width_Show;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowCoordinary)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShowCoordinary)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWCOORDINARY_H__6180060C_C18B_4CC8_BA0D_E79904498E46__INCLUDED_)
