// CeXianYouHuaView.h : interface of the CCeXianYouHuaView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CEXIANYOUHUAVIEW_H__38266BAC_F85B_43E3_A71D_207444F25D97__INCLUDED_)
#define AFX_CEXIANYOUHUAVIEW_H__38266BAC_F85B_43E3_A71D_207444F25D97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CCeXianYouHuaView : public CScrollView
{
protected: // create from serialization only
	CCeXianYouHuaView();
	DECLARE_DYNCREATE(CCeXianYouHuaView)

// Attributes
public:
	CCeXianYouHuaDoc* GetDocument();
	BOOL m_onlyDrawExtractResult;  // 是否只显示提取结果

	CPoint Point1;
	CPoint s_Point;
	float m_jingdu;
	float m_weidu;
	float m_Scale_x;
	float m_Scale_y;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCeXianYouHuaView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	int m_ImgHScrollPos;
	int m_ImgVScrollPos;
	virtual ~CCeXianYouHuaView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	CArray<CPoint, CPoint> m_pointArray;
	short m_extractType;
	BOOL m_shiftDown;
	HCURSOR m_hCursor;
	BOOL m_lButtonDown;

	//{{AFX_MSG(CCeXianYouHuaView)
	afx_msg void OnInteractRoadExtract();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnInteractLakeExtract();
	afx_msg void OnDrawPolygon();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEarser();
	afx_msg void OnUpdateEarser(CCmdUI* pCmdUI);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDrawLine();
	afx_msg void OnEraseArea();
	afx_msg void OnInteractAlllakeExtract();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnInsertlal();
	afx_msg void OnGetsrc();
	afx_msg void OnGetlal();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnContrast();
	afx_msg void OnExtend();
	afx_msg void OnReload();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CeXianYouHuaView.cpp
inline CCeXianYouHuaDoc* CCeXianYouHuaView::GetDocument()
   { return (CCeXianYouHuaDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CEXIANYOUHUAVIEW_H__38266BAC_F85B_43E3_A71D_207444F25D97__INCLUDED_)
