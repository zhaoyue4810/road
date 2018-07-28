// CeXianYouHuaDoc.cpp : implementation of the CCeXianYouHuaDoc class
//

#include "stdafx.h"
#include "CeXianYouHua.h"

#include "Show_Contrast.h"
#include "CeXianYouHuaDoc.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ZoomDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaDoc

IMPLEMENT_DYNCREATE(CCeXianYouHuaDoc, CDocument)

BEGIN_MESSAGE_MAP(CCeXianYouHuaDoc, CDocument)
	//{{AFX_MSG_MAP(CCeXianYouHuaDoc)
	ON_COMMAND(ID_AUTO_ROAD_EXTRACT, OnAutoRoadExtract)
	ON_COMMAND(ID_AUTO_LAKE_EXTRACT, OnAutoLakeExtract)
	ON_COMMAND(ID_SCALE_SMALL, OnScaleSmall)
	ON_COMMAND(ID_SCALE_BIG, OnScaleBig)
	ON_COMMAND(ID_CANNY, OnCanny)
	ON_COMMAND(ID_GuassFilter, OnGuassFilter)
	ON_COMMAND(ID_Robert, OnRobert)
	ON_COMMAND(ID_Prewitt, OnPrewitt)
	ON_COMMAND(ID_Sobel, OnSobel)
	ON_COMMAND(ID_Laplacian, OnLaplacian)
	ON_COMMAND(ID_ZOOM, OnZoom)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_MIXED_CHANNEL,ID_BLUE_CHANNEL,OnChannelChange)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MIXED_CHANNEL,ID_BLUE_CHANNEL,OnUpdateChannelChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaDoc construction/destruction

CCeXianYouHuaDoc::CCeXianYouHuaDoc()
{
	// TODO: add one-time construction code here
	m_bImageLoaded = FALSE;
	m_pDibObject = NULL;
	m_pSubDibObject = NULL;
	m_nChannelIndex = 0;
	m_pGrayDibObject = NULL;
	m_pExtractDibObject = NULL;
	m_onlyDrawExtractResult = FALSE;
	m_moveCX = 0;
	m_moveCY = 0;
	m_startPoint = 0;
	m_jieQuSize.cx = 800;
	m_jieQuSize.cy = 600;
	m_scale = 1;
}

CCeXianYouHuaDoc::~CCeXianYouHuaDoc()
{
	if(m_pDibObject != NULL)
	{
		delete m_pDibObject;
		m_pDibObject = NULL;
	}

	if(m_pSubDibObject != NULL)
	{
		delete m_pSubDibObject;
		m_pSubDibObject = NULL;
	}

	if(m_pGrayDibObject != NULL)
	{
		delete m_pGrayDibObject;
		m_pGrayDibObject = NULL;
	}

	if(m_pExtractDibObject != NULL)
	{
		delete m_pExtractDibObject;
		m_pExtractDibObject = NULL;
	}
}

BOOL CCeXianYouHuaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaDoc serialization

void CCeXianYouHuaDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaDoc diagnostics

#ifdef _DEBUG
void CCeXianYouHuaDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCeXianYouHuaDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCeXianYouHuaDoc commands

BOOL CCeXianYouHuaDoc::LoadImageToDocument()
{
	CString strPathName = GetPathName();

	//设置等待光标
	BeginWaitCursor();
	if(m_pDibObject)
		delete m_pDibObject;
	m_pDibObject = new CDibObject(strPathName.GetBuffer(3), m_startPoint, m_jieQuSize, m_scale);
	//取消等待光标
	EndWaitCursor();

	//读入图像文件失败
	if( m_pDibObject == NULL )
	{
		AfxMessageBox("无法创建图像类对象！");
		//返回FALSE
		return(FALSE);
	}
	
	//调试
/*	float re_ratio;
	float ratio;
	int WidthIn=m_pDibObject->m_nWidth;
	int HeightIn=m_pDibObject->m_nHeight;
	if(WidthIn>=1000 && HeightIn>=600)
	{
		re_ratio=(WidthIn/1000) > (HeightIn/600) ? (WidthIn/1000) : (HeightIn/600);
		ratio=1.0/re_ratio;
		m_pDibObject->Zoom(ratio,ratio);
	}*/
	//调试

	//获取主框架窗口指针
	CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
	//获取子框架窗口指针
	CChildFrame *pChild = (CChildFrame *) pFrame->MDIGetActive();
	pChild->m_nWidth = m_pDibObject->GetWidth(); 
	pChild->m_nHeight = m_pDibObject->GetHeight();
	pChild->SetWindowPos( NULL, 0, 0, pChild->m_nWidth + 12, 
		pChild->m_nHeight + 38, SWP_NOZORDER | SWP_NOMOVE );

	//读入图像文件成功，设置相应变量
	m_bImageLoaded = TRUE;

	// 用图像m_pDibObject初始化分割后的图像m_pSubDibObject
	if(m_pSubDibObject)
		delete m_pSubDibObject;
	m_pSubDibObject = new CDibObject(m_pDibObject);

	// 由图像m_pSubDibObject获取灰度图像m_pGrayDibObject
	GetGrayDibObject();

	// 获取与子图像m_pSubDibObject相匹配的空白图像m_pExtractDibObject
	ResetExtractDibObject();

	//返回TRUE
	return TRUE;
}

// 由图像m_pSubDibObject获取灰度图像m_pGrayDibObject
BOOL CCeXianYouHuaDoc::GetGrayDibObject()
{
	BOOL success = FALSE;

	// 更改光标形状
	BeginWaitCursor();
	
	switch(m_nChannelIndex)
	{
	case 0:
		success = m_pSubDibObject->GetMixedChannel(m_pGrayDibObject);
		break;
	case 1:
		success = m_pSubDibObject->GetRedChannel(m_pGrayDibObject);
		break;
	case 2:
		success = m_pSubDibObject->GetGreenChannel(m_pGrayDibObject);
		break;
	default:
		success = m_pSubDibObject->GetBlueChannel(m_pGrayDibObject);
		break;
	}

	// 恢复光标
	EndWaitCursor();

	return success;
}

void CCeXianYouHuaDoc::OnUpdateChannelChange(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_nChannelIndex == (pCmdUI->m_nID - ID_MIXED_CHANNEL));
}

void CCeXianYouHuaDoc::OnChannelChange(WORD nID)
{
	m_nChannelIndex = nID-ID_MIXED_CHANNEL;

	// 获取灰度图像存放于变量m_pGrayDibObject中
	GetGrayDibObject();
	
	UpdateAllViews(NULL);
}

// 获取与子图像m_pSubDibObject相匹配的空白图像m_pExtractDibObject
BOOL CCeXianYouHuaDoc::ResetExtractDibObject()
{
	unsigned char *pBits;  // 图像数据指针
	HGLOBAL hGlobal;       // 图像文件指针
	BITMAPFILEHEADER *pBitmapHeader;  // 文件头指针
	int width, height;     // 图像宽和高
	int x, y;  // 中间变量

	// 清除图像m_pExtractDibOjbect
	if(m_pExtractDibObject != NULL)
	{
		delete m_pExtractDibObject;
	}

	// 为图像m_pExtractDibOjbect赋值
	m_pExtractDibObject = new CDibObject(m_pGrayDibObject);

	// 获取图像文件指针
	hGlobal = m_pExtractDibObject->GetDib();
	pBits = (unsigned char *)::GlobalLock(hGlobal);

	// 获取图像文件头指针
	pBitmapHeader = (BITMAPFILEHEADER *)pBits;

	// 获取图像数据指针
	pBits += pBitmapHeader->bfOffBits;
	
	// 获取图像的宽和高
	width = m_pExtractDibObject->GetWidthBytes();
	height = m_pExtractDibObject->GetHeight();

	// 清除图像数据
	for(x=0; x<width; x++)
	{
		for(y=0;y<height;y++)
		{
			pBits[x+y*width]=255;
		}
	}
	::GlobalUnlock(hGlobal);

	return TRUE;
}

// 获取分割后的图像存放于m_pSubDibObject中
BOOL CCeXianYouHuaDoc::GetSubDibObject(CRect &rect)
{
	int top, bottom;

	// rect坐标转换
	top = rect.top;
	bottom = rect.bottom;
	rect.top = m_pDibObject->GetHeight() - 1 - bottom;
	rect.bottom = m_pDibObject->GetHeight() - 1 - top;

	m_pDibObject->GetSubDibObject(rect, m_pSubDibObject);
	GetGrayDibObject();
	ResetExtractDibObject();

	// rect坐标转换
	top = rect.top;
	bottom = rect.bottom;
	rect.top = m_pDibObject->GetHeight() - 1 - bottom;
	rect.bottom = m_pDibObject->GetHeight() - 1 - top;

	return TRUE;
}

void CCeXianYouHuaDoc::OnAutoRoadExtract() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->AutoRoadExtract(m_pExtractDibObject);
	UpdateAllViews(NULL);
}

void CCeXianYouHuaDoc::OnAutoLakeExtract() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->AutoLakeExtract(m_pExtractDibObject);
	UpdateAllViews(NULL);
}

// 道路交互提取
void CCeXianYouHuaDoc::InteractRoadExtract(CPoint roadSeed1, CPoint roadSeed2) 
{
	// TODO: Add your command handler code here
	// 对种子点的坐标变换
	roadSeed1.y = m_pGrayDibObject->GetHeight() - 1 - roadSeed1.y;
	roadSeed2.y = m_pGrayDibObject->GetHeight() - 1 - roadSeed2.y;

	m_pGrayDibObject->InteractRoadExtract(roadSeed1, roadSeed2, m_pExtractDibObject);
	UpdateAllViews(NULL);
}

// 水体交互提取
void CCeXianYouHuaDoc::InteractLakeExtract(CPoint lakeSeed)
{
	lakeSeed.y = m_pGrayDibObject->GetHeight() - 1 - lakeSeed.y;
	m_pGrayDibObject->InteractLakeExtract(lakeSeed, m_pExtractDibObject);
	UpdateAllViews(NULL);
}

// 水体交互提取
void CCeXianYouHuaDoc::InteractAllLakeExtract(CPoint lakeSeed)
{
	lakeSeed.y = m_pGrayDibObject->GetHeight() - 1 - lakeSeed.y;
	m_pGrayDibObject->InteractAllLakeExtract(lakeSeed, m_pExtractDibObject);
	UpdateAllViews(NULL);
}

// 画多边形
void CCeXianYouHuaDoc::DrawPolygon(CArray<CPoint,CPoint> &polygonPoints)
{
	int pointNum;  // 点的个数
	int i;  // 循环变量
	int height;  // 存放提取结果的图像的高度(-1)
	CPoint point;  // 点变量

	// polygonPoints坐标变换
	pointNum = polygonPoints.GetSize();
	height = m_pExtractDibObject->GetHeight() - 1;
	for(i=0; i<pointNum; i++)
	{
		point = polygonPoints.GetAt(i);
		point.y = height - point.y;
		polygonPoints.SetAt(i,point);
	}
	m_pExtractDibObject->DrawPolygon(polygonPoints);

}

// 擦除
void CCeXianYouHuaDoc::Earser(CPoint point)
{
	point.y = m_pExtractDibObject->GetHeight() - 1 - point.y;
	m_pExtractDibObject->Earser(point);
}

// 擦除选中的区域
void CCeXianYouHuaDoc::EraseArea(CPoint point1, CPoint point2)
{
	point1.y = m_pExtractDibObject->GetHeight() - 1 - point1.y;
	point2.y = m_pExtractDibObject->GetHeight() - 1 - point2.y;
	m_pExtractDibObject->EarseArea(point1, point2);
	UpdateAllViews(NULL);
}

// 画线
void CCeXianYouHuaDoc::DrawLines(CArray<CPoint,CPoint> &linePoints)
{
	int pointNum;  // 点的个数
	int i;  // 循环变量
	int height;  // 存放提取结果的图像的高度(-1)
	CPoint point;  // 点变量

	// polygonPoints坐标变换
	pointNum = linePoints.GetSize();
	height = m_pExtractDibObject->GetHeight() - 1;
	for(i=0; i<pointNum; i++)
	{
		point = linePoints.GetAt(i);
		point.y = height - point.y;
		linePoints.SetAt(i,point);
	}
	m_pExtractDibObject->DrawLines(linePoints);//这里的linePoints中的点已经从屏幕坐标转换成图像数据坐标

	CArray<CPoint,CPoint> pointArray;
	m_pExtractDibObject->GetMultiPoints(linePoints,pointArray);
	m_pExtractDibObject->RecordPoints(pointArray);

//	m_pExtractDibObject->RecordPoints(linePoints);//获得手工提取道路的点存放于txt文件中
}

void CCeXianYouHuaDoc::OnScaleSmall() 
{
	// TODO: Add your command handler code here
	m_scale *= 2;
	LoadImageToDocument();
	UpdateAllViews(NULL);
}

void CCeXianYouHuaDoc::OnScaleBig() 
{
	// TODO: Add your command handler code here
	m_scale /= 2;
	LoadImageToDocument();
	UpdateAllViews(NULL);
}

void CCeXianYouHuaDoc::OnCanny() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->Canny(m_pExtractDibObject);
	UpdateAllViews(NULL);
}

void CCeXianYouHuaDoc::OnGuassFilter() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->GaussFilter(m_pExtractDibObject);
	UpdateAllViews(NULL);
}

void CCeXianYouHuaDoc::OnRobert() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->Robert(m_pExtractDibObject);
//	m_pGrayDibObject->Robert(m_pGrayDibObject);
	UpdateAllViews(NULL);
}

void CCeXianYouHuaDoc::OnPrewitt() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->Prewitt(m_pExtractDibObject);
	UpdateAllViews(NULL);	
}

void CCeXianYouHuaDoc::OnSobel() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->Sobel(m_pExtractDibObject);
	UpdateAllViews(NULL);	
}

void CCeXianYouHuaDoc::OnLaplacian() 
{
	// TODO: Add your command handler code here
	m_pGrayDibObject->Laplacian(m_pExtractDibObject);
	UpdateAllViews(NULL);
}

/*void CCeXianYouHuaDoc::OnContrast() 
{
	// TODO: Add your command handler code here
	CShow_Contrast *csc=new CShow_Contrast;
	csc->Create(IDD_DIALOG1,NULL);
	csc->ShowWindow(SW_SHOW);

	HWND hWnd; 
	CWnd *pWnd=GetDlgItem(hWnd,IDD_DIALOG1);
	CRect rect;
	pWnd->GetClientRect(&rect);
	CDC *pDC=pWnd->GetDC();

	csc->UpdateWindow();


	//csc.DoModal();
}
*/

void CCeXianYouHuaDoc::OnZoom() 
{
	
	ZoomDlg dlg;
	dlg.DoModal();
	if(dlg.m_ratioX>1 || dlg.m_ratioY>1)
	{
		AfxMessageBox("该功能只针对缩小图像");
		return;
	}
	// TODO: Add your command handler code here
	m_pDibObject->Zoom(dlg.m_ratioX,dlg.m_ratioY);

	//这里改变m_pSubDibObject的内容
	if(m_pSubDibObject)
		delete m_pSubDibObject;
	m_pSubDibObject = new CDibObject(m_pDibObject);

	// 由图像m_pSubDibObject获取灰度图像m_pGrayDibObject
	GetGrayDibObject();

	// 获取与子图像m_pSubDibObject相匹配的空白图像m_pExtractDibObject
	ResetExtractDibObject();

	UpdateAllViews(NULL);

}
