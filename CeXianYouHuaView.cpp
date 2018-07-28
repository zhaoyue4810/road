// CeXianYouHuaView.cpp : implementation of the CCeXianYouHuaView class
//

#include "stdafx.h"
#include "CeXianYouHua.h"

#include "DibObject.h"
#include "Show_Contrast.h"
#include "CeXianYouHuaDoc.h"
#include "CeXianYouHuaView.h"
#include "ShowCoordinary.h"
#include "GeoDialog.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaView

IMPLEMENT_DYNCREATE(CCeXianYouHuaView, CView)

BEGIN_MESSAGE_MAP(CCeXianYouHuaView, CView)
	//{{AFX_MSG_MAP(CCeXianYouHuaView)
	ON_COMMAND(ID_INTERACT_ROAD_EXTRACT, OnInteractRoadExtract)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_INTERACT_LAKE_EXTRACT, OnInteractLakeExtract)
	ON_COMMAND(ID_DRAW_POLYGON, OnDrawPolygon)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_EARSER, OnEarser)
	ON_UPDATE_COMMAND_UI(ID_EARSER, OnUpdateEarser)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_DRAW_LINE, OnDrawLine)
	ON_COMMAND(ID_ERASE_AREA, OnEraseArea)
	ON_COMMAND(ID_INTERACT_ALLLAKE_EXTRACT, OnInteractAlllakeExtract)
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_INSERTLAL, OnInsertlal)
	ON_COMMAND(ID_GETSRC, OnGetsrc)
	ON_COMMAND(ID_GETLAL, OnGetlal)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_CONTRAST, OnContrast)
	ON_COMMAND(ID_EXTEND, OnExtend)
	ON_COMMAND(ID_RELOAD, OnReload)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaView construction/destruction

CCeXianYouHuaView::CCeXianYouHuaView()
{
	// TODO: add construction code here
	m_extractType = 0;
	m_shiftDown = FALSE;
	m_lButtonDown = FALSE;
	m_onlyDrawExtractResult = TRUE;
	m_pointArray.RemoveAll();
	Point1.x=0;
	Point1.y=0;
	s_Point.x=0;
	s_Point.y=0;
	m_Scale_x=0;
	m_Scale_y=0;
	m_jingdu=0;
	m_weidu=0;
	m_ImgVScrollPos=0;
	 m_ImgHScrollPos=0;
}

CCeXianYouHuaView::~CCeXianYouHuaView()
{
}

BOOL CCeXianYouHuaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaView drawing

void CCeXianYouHuaView::OnDraw(CDC* pDC)
{
	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
	if( !pDoc->m_bImageLoaded)
	{
		pDoc->LoadImageToDocument();
		return;
	}

	//¹ö¶¯´°¿Ú
	CSize sizeTotal;
	sizeTotal.cx = pDoc->m_pDibObject->GetWidth()*1.5;
	sizeTotal.cy = pDoc->m_pDibObject->GetHeight();
	SetScrollSizes (MM_TEXT, sizeTotal);

	if( GetFocus() == this )
		pDoc->m_pDibObject->SetPalette(pDC);

	if(!m_onlyDrawExtractResult)
	pDoc->m_pSubDibObject->Draw(pDC, pDoc->m_moveCX, pDoc->m_moveCY);
//	pDoc->m_pDibObject->Draw(pDC, pDoc->m_moveCX, pDoc->m_moveCY);	
//	pDoc->m_pGrayDibObject->Draw(pDC, pDoc->m_moveCX, pDoc->m_moveCY);
	pDoc->m_pExtractDibObject->DrawPixel(pDC, pDoc->m_moveCX, pDoc->m_moveCY);
  

	CPoint point;
	if(m_extractType==10 && m_pointArray.GetSize()==1)////½»»¥µÀÂ·ÌáÈ¡
	{
		point = m_pointArray.GetAt(0);
		point.x += pDoc->m_moveCX;
		point.y += pDoc->m_moveCY;
		pDC->SetPixel(point, 0);
	}
	else if(m_extractType==23 && m_pointArray.GetSize()==1)////²Á³ýÇøÓò
	{
		point = m_pointArray.GetAt(0);
		point.x += pDoc->m_moveCX;
		point.y += pDoc->m_moveCY;
		pDC->SetPixel(point, 0);
	}
	else if(m_extractType == 20)//»­ÇøÓò
	{
		int pointNum = m_pointArray.GetSize();
		if(pointNum)
		{
			point = m_pointArray.GetAt(0);
			point.x += pDoc->m_moveCX;
			point.y += pDoc->m_moveCY;
			pDC->MoveTo(point);
		}
		for(int i=1;i<pointNum;i++)
		{
			point = m_pointArray.GetAt(i);
			point.x += pDoc->m_moveCX;
			point.y += pDoc->m_moveCY;
			pDC->LineTo(point);
		}
	}
	else if(m_extractType == 22)//»­Ïß
	{
		int pointNum = m_pointArray.GetSize();
		if(pointNum)
		{
			point = m_pointArray.GetAt(0);
			point.x += pDoc->m_moveCX;
			point.y += pDoc->m_moveCY;
			pDC->MoveTo(point);
		}
		for(int i=1;i<pointNum;i++)
		{
			point = m_pointArray.GetAt(i);
			point.x += pDoc->m_moveCX;
			point.y += pDoc->m_moveCY;
			pDC->LineTo(point);
		}
	}
}

void CCeXianYouHuaView::OnInitialUpdate()
{
CScrollView::OnInitialUpdate();
 CClientDC dc(this);
 int m_nCellWidth = dc.GetDeviceCaps(LOGPIXELSX);
 int m_nCellHeigh = dc.GetDeviceCaps(LOGPIXELSY) / 4;
 int m_nRibbonWidth = m_nCellWidth / 2;
 int nWidth = (26 * m_nCellWidth) + m_nRibbonWidth;
 int nHeight = m_nCellHeigh * 100;
 SetScrollSizes(MM_TEXT,CSize(nWidth,nHeight));


	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	//SetScrollSizes(MM_TEXT, sizeTotal);
	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc->m_onlyDrawExtractResult)
	{
		m_onlyDrawExtractResult = FALSE;
		pDoc->m_onlyDrawExtractResult = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaView printing

BOOL CCeXianYouHuaView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCeXianYouHuaView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCeXianYouHuaView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaView diagnostics

#ifdef _DEBUG
void CCeXianYouHuaView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CCeXianYouHuaView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CCeXianYouHuaDoc* CCeXianYouHuaView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCeXianYouHuaDoc)));
	return (CCeXianYouHuaDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaView message handlers

void CCeXianYouHuaView::OnInteractRoadExtract() 
{
	// TODO: Add your command handler code here
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor);
	m_extractType = 10;
	m_pointArray.RemoveAll();
}

void CCeXianYouHuaView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(m_extractType == 0)
	{
		m_pointArray.RemoveAll();
		m_pointArray.Add(point);
	}
	else if(m_extractType ==10)//½»»¥µÀÂ·ÌáÈ¡
	{
		if(m_pointArray.GetSize() == 0)
		{
			m_pointArray.Add(CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
			pDoc->UpdateAllViews(NULL);
		}
		else if(m_pointArray.GetSize() == 1)
		{
			m_extractType = 0;
			pDoc->InteractRoadExtract(m_pointArray.GetAt(0), CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
			m_pointArray.RemoveAll();
		}
	}
	else if(m_extractType == 23)//²Á³ýÇøÓò
	{
		if(m_pointArray.GetSize() == 0)
		{
			m_pointArray.Add(CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
			pDoc->UpdateAllViews(NULL);
		}
		else if(m_pointArray.GetSize() == 1)
		{
			m_extractType = 0;
			pDoc->EraseArea(m_pointArray.GetAt(0), CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
			m_pointArray.RemoveAll();
		}
	}
	else if(m_extractType == 11)//½»»¥Ë®ÌåÌáÈ¡
	{
		m_extractType = 0;
		pDoc->InteractLakeExtract(CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
		m_pointArray.RemoveAll();
	}
	else if(m_extractType == 12)//ÌáÈ¡È«²¿Ë®Ìå
	{
		m_extractType = 0;
		pDoc->InteractAllLakeExtract(CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
		m_pointArray.RemoveAll();
	}
	else if(m_extractType == 20)//»­ÇøÓò
	{
		if(m_shiftDown)
		{
			m_pointArray.Add(CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
			pDoc->UpdateAllViews(NULL);
		}
	}
	else if(m_extractType == 21)//ÏðÆ¤²Á
	{
		m_lButtonDown = TRUE;
	}
	else if(m_extractType == 22)//»­Ïß
	{
		if(m_shiftDown)
		{
			m_pointArray.Add(CPoint(point.x - pDoc->m_moveCX, point.y - pDoc->m_moveCY));
			pDoc->UpdateAllViews(NULL);
		}
	}
	else if(m_extractType == 1)
	{
		m_pointArray.RemoveAll();
		m_pointArray.Add(point);
		pDoc->UpdateAllViews(NULL);
	}
	CScrollView::OnLButtonDown(nFlags, point);
}

void CCeXianYouHuaView::OnInteractLakeExtract() 
{
	// TODO: Add your command handler code here
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor);
	m_extractType = 11;
}

void CCeXianYouHuaView::OnDrawPolygon() 
{
	// TODO: Add your command handler code here
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor);
	m_extractType = 20;
	m_pointArray.RemoveAll();
}

void CCeXianYouHuaView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == VK_SHIFT)
	{
		m_shiftDown = TRUE;
	}

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCeXianYouHuaView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == VK_SHIFT)
	{
		m_shiftDown = FALSE;
		CCeXianYouHuaDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if(m_extractType == 20)//»­ÇøÓò
		{
			pDoc->DrawPolygon(m_pointArray);
		//	pDoc->m_pExtractDibObject->DrawPolygon(m_pointArray);
		}
		else if(m_extractType ==22)//»­Ïß
		{
			pDoc->DrawLines(m_pointArray);
		}
		m_extractType = 0;
		pDoc->UpdateAllViews(NULL);
	}

	CScrollView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CCeXianYouHuaView::OnEarser() 
{
	// TODO: Add your command handler code here
	if(m_extractType != 21)
	{
		m_extractType = 21;
		m_hCursor = LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_EARSER));
		::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor); 
	}
	else
	{
		m_extractType = 0;
		m_hCursor = LoadCursor(NULL, IDC_ARROW);
		::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor); 
	}
}

void CCeXianYouHuaView::OnUpdateEarser(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_extractType == 21);
}

void CCeXianYouHuaView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(m_extractType == 1)
	{
		if(m_pointArray.GetSize() == 1)
		{
			CPoint priPoint = m_pointArray.GetAt(0);
			//pDoc->m_moveCX += point.x - priPoint.x;
			//pDoc->m_moveCY += point.y - priPoint.y;
			pDoc->m_startPoint.x += priPoint.x - point.x;
			pDoc->m_startPoint.y += point.y - priPoint.y;
			pDoc->LoadImageToDocument();
			pDoc->UpdateAllViews(NULL);
			m_extractType = 0;
		}
	}
	else if(m_extractType == 21)
	{
		m_lButtonDown = FALSE;
	}
	CScrollView::OnLButtonUp(nFlags, point);
}

void CCeXianYouHuaView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: Add your message handler code here and/or call default
	if(m_lButtonDown)
	{
		pDoc->Earser(CPoint(point.x+3-pDoc->m_moveCX, point.y+3-pDoc->m_moveCY));
		pDoc->UpdateAllViews(NULL);
	}

	CScrollView::OnMouseMove(nFlags, point);
}

void CCeXianYouHuaView::OnDrawLine() 
{
	// TODO: Add your command handler code here
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor);
	m_extractType = 22;
	m_pointArray.RemoveAll();
}

void CCeXianYouHuaView::OnEraseArea() 
{
	// TODO: Add your command handler code here
	m_extractType = 23;
	m_pointArray.RemoveAll();
}

void CCeXianYouHuaView::OnInteractAlllakeExtract() 
{
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor);
	m_extractType = 12;
}

void CCeXianYouHuaView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	Point1.x=point.x;
	Point1.y=point.y;

	CMenu menu,*pSubMenu;//¶¨ÒåÏÂÃæÒªÓÃµ½µÄcmenu¶ÔÏó 
    menu.LoadMenu(IDR_POPMENU);//×°ÔØ×Ô¶¨ÒåµÄÓÒ¼ü²Ëµ¥ 
    pSubMenu=menu.GetSubMenu(0);//»ñÈ¡µÚÒ»¸öµ¯³ö²Ëµ¥£¬ËùÒÔµÚÒ»¸ö²Ëµ¥±ØÐëÓÐ×Ó²Ëµ¥ 
    CPoint oPoint;//¶¨ÒåÒ»¸öÓÃÓÚÈ·¶¨¹â±êÎ»ÖÃµÄÎ»ÖÃ 
    GetCursorPos(&oPoint);//»ñÈ¡µ±Ç°¹â±êµÄÎ»ÖÃ£¬ÒÔ±ãÊ¹µÃ²Ëµ¥¿ÉÒÔ¸úËæ¹â±ê 
    pSubMenu->TrackPopupMenu(TPM_LEFTALIGN,oPoint.x,oPoint.y,this);   

	CScrollView::OnRButtonUp(nFlags, point);
}

void CCeXianYouHuaView::OnInsertlal() 
{
	// TODO: Add your command handler code here
	CGeoDialog cg;
	cg.DoModal();
	UpdateData(TRUE);


	if(0==m_jingdu&&0==m_weidu)
	{
		m_jingdu=cg.m_Log;
		m_weidu=cg.m_Lat;
		s_Point.x=Point1.x;
		s_Point.y=Point1.y;
	}
	else
	{
		if(cg.m_Log==m_jingdu || Point1.x==s_Point.x || cg.m_Lat==m_weidu || Point1.y==s_Point.y)
		{
			AfxMessageBox("ÇëÖØÐÂÊäÈë");
		}
		else
		{
			m_Scale_x=(cg.m_Log-m_jingdu)/(Point1.x-s_Point.x);
			m_Scale_y=(cg.m_Lat-m_weidu)/(Point1.y-s_Point.y);
		}
	}
}

void CCeXianYouHuaView::OnGetsrc() 
{
	// TODO: Add your command handler code here
	CShowCoordinary csc;
	csc.m_Width_Show=Point1.x;
	csc.m_Height_Show=Point1.y;
	csc.DoModal();
}

void CCeXianYouHuaView::OnGetlal() 
{
	// TODO: Add your command handler code here
	if(m_Scale_x==0 || m_Scale_y==0)
	{
		AfxMessageBox("±ØÐëÏÈÊäÈëÁ½¸öµãµÄ¾­Î³¶È×ø±ê²ÅÄÜ½øÐÐ²åÖµ¼ÆËã!");
		return;
	}
	else
	{
		CGeoDialog cg;
		cg.m_Log=m_jingdu+(Point1.x-s_Point.x)*m_Scale_x;
		cg.m_Lat=m_weidu+(Point1.y-s_Point.y)*m_Scale_y;
		UpdateData(FALSE);
		cg.DoModal();

	}
}

void CCeXianYouHuaView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	SCROLLINFO si ; 

    GetScrollInfo(SB_VERT,&si,SIF_ALL); 
    m_ImgVScrollPos = si.nPos; 
    Invalidate (TRUE); 

	 CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CCeXianYouHuaView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	SCROLLINFO si ; 

    GetScrollInfo(SB_HORZ,&si,SIF_ALL); 
    m_ImgHScrollPos = si.nPos; 
    Invalidate (TRUE); 
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCeXianYouHuaView::OnContrast() 
{
	// TODO: Add your command handler code here
//	CShow_Contrast *csc=new CShow_Contrast;
//	csc->Create(IDD_DIALOG1,NULL);
//	csc->ShowWindow(SW_SHOW);


//	CWnd *pWnd=GetDlgItem(IDD_DIALOG1);
//	CRect rect;
//	pWnd->GetClientRect(&rect);
//	CDC *pDC=pWnd->GetDC();


//	CShow_Contrast *dlg=new CShow_Contrast();
//	dlg.DoModal();

//	CShow_Contrast *pDlg=new CShow_Contrast();
//	pDlg->DoModal();

	CShow_Contrast *pDlg=new CShow_Contrast();	
	pDlg->Create(IDD_DIALOG1,this);
	pDlg->ShowWindow(SW_SHOW);

	CDC *pDC=pDlg->GetDC();

	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	//µ÷ÊÔ
	pDlg->MoveWindow(0,0,pDoc->m_pDibObject->GetWidth(),pDoc->m_pDibObject->GetHeight());
//	pDlg->MoveWindow(0,0,pDoc->m_pExtractDibObject->m_nWidth,pDoc->m_pExtractDibObject->m_nHeight);
	//µ÷ÊÔ
	
//	SetClassLong(pDlg->m_hWnd,GCL_HBRBACKGROUND,(LONG)GetStockObject(WHITE_BRUSH));

//	pDoc->m_pExtractDibObject->DrawWhite(pDC, pDoc->m_moveCX, pDoc->m_moveCY);

//	pDoc->m_pSubDibObject->Draw(pDC, pDoc->m_moveCX, pDoc->m_moveCY);
	pDoc->m_pExtractDibObject->DrawPixel(pDC, pDoc->m_moveCX, pDoc->m_moveCY);
	
}

void CCeXianYouHuaView::OnExtend() 
{
	// TODO: Add your command handler code here
	CCeXianYouHuaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int pointNum = m_pointArray.GetSize();

	if(0==pointNum)
	{
		AfxMessageBox("Error!ÇëÏÈÌáÈ¡µÀÂ·!");
		return;
	}
	CPoint A=m_pointArray.GetAt(0);
	CPoint B=m_pointArray.GetAt(m_pointArray.GetSize()-1);

	CPoint A1,A2,B1,B2;
/*	A1.x=A.x+8;
	A1.y=A.y-8*(B.x-A.x)/(B.y-A.y);

	A2.x=A.x-8;
	A2.y=A.y+8*(B.x-A.x)/(B.y-A.y);

	B1.x=B.x+8;
	B1.y=B.y-8*(B.x-A.x)/(B.y-A.y);

	B2.x=B.x-8;
	B2.y=B.y+8*(B.x-A.x)/(B.y-A.y);

	m_pointArray.RemoveAll();
*/
/*	int k=((B.x-A.x)/(B.y-A.y))*((B.x-A.x)/(B.y-A.y));
	A1.x=A.x+sqrt(64/(k+1));
	A1.y=A.y-(B.x-A.x)*(A1.x-A.x)/(B.y-A.y);

	A2.x=A.x-sqrt(64/(k+1));
	A2.y=A.y-(B.x-A.x)*(A2.x-A.x)/(B.y-A.y);

	B1.x=B.x+sqrt(64/(k+1));
	B1.y=B.y-(A.x-B.x)*(B1.x-B.x)/(A.y-B.y);

	B2.x=B.x-sqrt(64/(k+1));
	B2.y=B.y-(A.x-B.x)*(B2.x-B.x)/(A.y-B.y);

	m_pointArray.RemoveAll();
*/
	float k=(((float)B.x-(float)A.x)/((float)B.y-(float)A.y))*(((float)B.x-(float)A.x)/((float)B.y-(float)A.y));
	A1.x=A.x+sqrt(64.0/(k+1));
	A1.y=A.y-(B.x-A.x)*(A1.x-A.x)/(B.y-A.y);

	A2.x=A.x-sqrt(64.0/(k+1));
	A2.y=A.y-(B.x-A.x)*(A2.x-A.x)/(B.y-A.y);

	B1.x=B.x+sqrt(64.0/(k+1));
	B1.y=B.y-(A.x-B.x)*(B1.x-B.x)/(A.y-B.y);

	B2.x=B.x-sqrt(64.0/(k+1));
	B2.y=B.y-(A.x-B.x)*(B2.x-B.x)/(A.y-B.y);

	m_pointArray.RemoveAll();
	m_pointArray.Add(A2);
	m_pointArray.Add(A1);
	m_pointArray.Add(B1);
	m_pointArray.Add(B2);
//	pDoc->DrawPolygon(m_pointArray);
	pDoc->m_pExtractDibObject->DrawPolygon(m_pointArray);
	m_extractType = 0;
	pDoc->UpdateAllViews(NULL);
}

void CCeXianYouHuaView::OnReload() 
{
	// TODO: Add your command handler code here
	m_hCursor = LoadCursor(NULL, IDC_ARROW);
	::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, (LONG)m_hCursor);
	m_extractType = 1;
	m_pointArray.RemoveAll();
}
