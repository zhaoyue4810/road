#include "StdAfx.h"
#include "DibObject.h"
#include "ImageLoad.h"
#include "ImageErrors.h"
#include <math.h>
#include <gdal_priv.h>

#define PI 3.1415927

// 调试用
unsigned char *pgUnchEdge;
int ngWidth;
// 调试完
//static int m_Record_Count_r;
//static int m_Record_Count_w;
//static int m_Record_Count_weh;
//static int m_Record_Count_woh;

static int m_I_r;
static int m_I_we;
static int m_I_wb;
static int m_H_r;
static int m_H_we;
static int m_H_wb;
////////////////////////////////////////////////////////////////////////
//CDibObject类的构造函数		    
//----------------------------------------------------------------------
//基本功能：这是一个无参数的构造函数。它简单地创建一个CDibObject对象并
//			初始化其内部变量。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：无
////////////////////////////////////////////////////////////////////////
CDibObject::CDibObject()
{
	//调用辅助函数初始化其内部变量。
	InitVars();
}

////////////////////////////////////////////////////////////////////////
//CDibObject类的构造函数		    
//----------------------------------------------------------------------
//基本功能：构造一个CDibObject对象。唯一的要求的一个参数是文件名。如果
//			给出了CDC设备上下文参数，图像加载后会立即在该设备上下文中显示
//			出来。如果给出了nX或nY参数，图像会显示在该坐标指定的位置，否
//			则，图像总是显示在坐标为（0, 0）的位置。
//----------------------------------------------------------------------
//参数说明：const char *pszFilename
//			CDC *pDC，默认为NULL
//			int nX，默认为-1
//			int nY，默认为-1
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
CDibObject::CDibObject( const char *pszFilename, CPoint &startPoint, CSize &jieQuSize,
					   float scale, CDC *pDC, int nX, int nY )
{
	InitVars();
	Load( pszFilename, startPoint, jieQuSize, scale, pDC, nX, nY );
}

////////////////////////////////////////////////////////////////////////
//CDibObject类的构造函数		    
//----------------------------------------------------------------------
//基本功能：用一个CDibObject对象通过复制操作来创建一个CDibObject对象。
//----------------------------------------------------------------------
//参数说明：CDibObject *pDibObject
//----------------------------------------------------------------------
//返 回 值：无
////////////////////////////////////////////////////////////////////////
CDibObject::CDibObject(CDibObject *pDibObject)
{
	InitVars();

	KillImage();
	
	m_nWidth = pDibObject->m_nWidth;
	m_nHeight = pDibObject->m_nHeight;
	m_nPlanes = pDibObject->m_nPlanes;
	m_nBits = pDibObject->m_nBits;
	m_nColors = pDibObject->m_nColors;
	m_nImageType = pDibObject->m_nImageType;
	m_nX = pDibObject->m_nX;
	m_nY = pDibObject->m_nY;
	m_nLastError = pDibObject->m_nLastError;
	m_nScreenPlanes = pDibObject->m_nScreenPlanes;
	m_nScreenBits = pDibObject->m_nScreenBits;
	m_nPaletteBytes = pDibObject->m_nPaletteBytes;
	m_nQuality = pDibObject->m_nQuality;
	m_nPaletteCreationType = pDibObject->m_nPaletteCreationType;
	
	int nNumColors = m_nColors;
	int nWidthBytes = WidthBytes( m_nBits, m_nWidth );
	
	if( pDibObject->m_hDib != NULL )
	{
		DWORD dwSize = ::GlobalSize( pDibObject->m_hDib );
		char *pData = (char *) ::GlobalLock( pDibObject->m_hDib );
		if( pData != NULL )
		{
			HGLOBAL hGlobal = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize );
			if( hGlobal != NULL ){
				
				char *pDestData = (char *) ::GlobalLock( hGlobal );
				if( pDestData != NULL )
				{
					memcpy( pDestData, pData, dwSize );
					::GlobalUnlock( hGlobal );
					m_hDib = hGlobal;
				}
				else ::GlobalFree( hGlobal );
			}
			::GlobalUnlock( pDibObject->m_hDib );
		}
	}
}

////////////////////////////////////////////////////////////////////////
//InitVars()函数		    
//----------------------------------------------------------------------
//基本功能：初始化CDibObject类内部变量
//----------------------------------------------------------------------
//参数说明：BOOL bFlag	标记类中是否有图像打开，默认值为FALSE
////////////////////////////////////////////////////////////////////////
void CDibObject::InitVars( BOOL bFlag )
{
	m_nWidth = m_nHeight = m_nBits = m_nColors = m_nImageType = 0;
	m_nX = m_nY = 0;
	m_nLastError = 0;
	m_hDib = NULL;
	m_nPaletteCreationType = FIXED_PALETTE;
	if( !bFlag )
	{
		m_pszFilename = NULL;
	}
	m_nQuality = 50;
}

////////////////////////////////////////////////////////////////////////
//BOOL Load()		    
//----------------------------------------------------------------------
//基本功能：本函数把一个图像文件载入CDibObject类。必需的一个参数是文件
//			名。如果给出了CDC设备上下文参数，图像一加载就被绘制。如果给
//			出了nX或nY参数，图像将显示在该坐标指定的位置。否则，图像总是
//			显示在坐标为（0,0）的位置。
//----------------------------------------------------------------------
//参数说明：const char *pszFilename
//			CDC *pDC，默认为NULL
//			int nX，默认为-1
//			int nY，默认为-1
//----------------------------------------------------------------------
//返 回 值：BOOL：成功返回TRUE，失败返回FALSE
////////////////////////////////////////////////////////////////////////
BOOL CDibObject::Load( const char *pszFilename,	CPoint &startPoint,
					  CSize &jieQuSize, float scale, CDC *pDC, int nX, int nY )
{

	//获取图像文件类型
	m_nImageType = FileType( pszFilename );
	if( m_nImageType == 0 )
	{
		m_nLastError = IMAGELIB_UNSUPPORTED_FILETYPE;
		return( FALSE );
	}

	//删除已存在的图像
	KillImage();

	m_pszFilename = new char [strlen(pszFilename)+1];
	if( m_pszFilename != NULL ) strcpy( m_pszFilename, pszFilename );

	//根据文件类型调用ImageLoad.dll动态链接库中的相应函数打开图像文件
	switch( m_nImageType )
	{
	case IMAGETYPE_BMP:
		m_hDib = ::LoadBMP( pszFilename );
		if( m_hDib == NULL ){
			m_nLastError = ::GetLastPicLibError();
			return( FALSE );
		}
		break;
	case IMAGETYPE_GIF:
		m_hDib = ::LoadGIF( pszFilename );
		if( m_hDib == NULL ){
			m_nLastError = ::GetLastPicLibError();
			return( FALSE );
		}
		break;
	case IMAGETYPE_JPG:
		m_hDib = ::LoadJPG( pszFilename );
		if( m_hDib == NULL ){
			m_nLastError = ::GetLastPicLibError();
			return( FALSE );
		}
		break;
	case IMAGETYPE_PCX:
		m_hDib = ::LoadPCX( pszFilename );
		if( m_hDib == NULL ){
			m_nLastError = ::GetLastPicLibError();
			return( FALSE );
		}
		break;
	case IMAGETYPE_TGA:
		m_hDib = ::LoadTGA( pszFilename );
		if( m_hDib == NULL ){
			m_nLastError = ::GetLastPicLibError();
			return( FALSE );
		}
		break;
	case IMAGETYPE_TIF:
		m_hDib = ::LoadTIF( pszFilename );
		if( m_hDib == NULL ){
			m_hDib = LoadGetTif(pszFilename, startPoint, jieQuSize, scale);
			if(m_hDib == NULL)
				return( FALSE );
		}
		break;
	}

	//处理图像信息头
	ProcessImageHeader();
	//处理调色板
	ProcessPalette();

	//若传入了设备上下文指针，就在指定的设备上下文中绘制图像
	if( pDC != NULL ) Draw( pDC, nX, nY );

	return( TRUE );
}

// 载入Geotiff格式图像
HGLOBAL CDibObject::LoadGetTif(const char *pszFilename, CPoint &startPoint, CSize &jieQuSize, float scale)
{
	GDALDataset *poDataset; //GDAL数据集
	int nImgSizeCX, nImgSizeCY;  // 原图像大小
	int length;  // 中间变量
	int widthBytes;  // 新图像位图字节宽度
	int bits = 8;  // 新图像位图每像素位数
	DWORD dwSize;  //新图像总字节数
	int colorNums = 256;  // 新图像位图颜色数
	HGLOBAL hGlobal;  // 为新图像位图分配内存
	unsigned char *pBits;  // 新图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 新图像位图文件头指针
	BITMAPINFOHEADER *pBIH;  // 新图像位图信息头指针
	RGBQUAD *pRGBPalette;  // 新图像位图调色板指针
	int i, j;  // 循环变量
	int nBandCount;  // 波段数量

	GDALAllRegister();  //注册驱动
	poDataset = (GDALDataset *) GDALOpen(pszFilename, GA_ReadOnly ); // 获取数据集
	if( poDataset == NULL )
	{
		AfxMessageBox("文件打开失败!!!");
		return NULL;
	}

	// 获取原图像大小
	nImgSizeCX = poDataset->GetRasterXSize();
	nImgSizeCY = poDataset->GetRasterYSize();
	
	// 获取缩放后的图像大小
	nImgSizeCX /= scale;
	nImgSizeCY /= scale;

	jieQuSize.cx = jieQuSize.cx < nImgSizeCX? jieQuSize.cx: nImgSizeCX;
	jieQuSize.cy = jieQuSize.cy < nImgSizeCY? jieQuSize.cy: nImgSizeCY;

	// 调整图像截取的左上点
	startPoint.x = 0 > startPoint.x? 0: startPoint.x;
	startPoint.y = 0 > startPoint.y? 0: startPoint.y;
	length = nImgSizeCX - jieQuSize.cx;
	startPoint.x = startPoint.x < length? startPoint.x: length;
	length = nImgSizeCY - jieQuSize.cy;
	startPoint.y = startPoint.y < length? startPoint.y: length;

	// 为新图像分配内存
	widthBytes = WidthBytes(bits, jieQuSize.cx);
	dwSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) +
			colorNums * sizeof( RGBQUAD ) + jieQuSize.cy * widthBytes;
	hGlobal = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize );
	if( hGlobal == NULL )
	{
		::GlobalUnlock( m_hDib );
		return( NULL );
	}

	pBits = (unsigned char *) ::GlobalLock( hGlobal );
	if( pBits == NULL )
	{
		::GlobalFree( hGlobal );
		return( NULL );
	}

	// 设置新图像的文件头指针
	pBFH = (BITMAPFILEHEADER *) pBits;
	pBFH->bfType = 19778;
	pBFH->bfOffBits = sizeof( BITMAPFILEHEADER ) + 
							sizeof( BITMAPINFOHEADER ) + 
							colorNums * sizeof( RGBQUAD );
	pBFH->bfSize = pBFH->bfOffBits + widthBytes * jieQuSize.cy;
	pBFH->bfReserved1 = 0;
	pBFH->bfReserved2 = 0;

	// 设置新图像的信息头指针
	pBIH = (BITMAPINFOHEADER *) &pBits[sizeof(BITMAPFILEHEADER)];
	pBIH->biSize = sizeof(BITMAPINFOHEADER);
	pBIH->biWidth = jieQuSize.cx;
	pBIH->biHeight = jieQuSize.cy;
	pBIH->biPlanes = 1;
	pBIH->biBitCount = 8;
	pBIH->biCompression = BI_RGB;
	pBIH->biSizeImage = widthBytes * jieQuSize.cy;
	pBIH->biXPelsPerMeter = 0;
	pBIH->biYPelsPerMeter = 0;
	pBIH->biClrUsed = colorNums;
	pBIH->biClrImportant = 0;

	//获得新图像的调色板指针
	pRGBPalette = (RGBQUAD *) &pBits[sizeof(BITMAPFILEHEADER)
						+sizeof(BITMAPINFOHEADER)];

	//创建256色灰度调色板
	for( i = 0; i < colorNums; i++ )
	{
		pRGBPalette[i].rgbRed = i;
		pRGBPalette[i].rgbGreen = i;
		pRGBPalette[i].rgbBlue = i;
	}

	pBits += pBFH->bfOffBits;

	// 获取波段数量
	nBandCount=poDataset->GetRasterCount();
	switch(nBandCount)
	{
	case 1:
		GDALRasterBand *poBand;  // 遥感的一个波段
		BYTE *pafScanblock, *origonPafScanblock;  // 缓冲区空间指针
		poBand = poDataset->GetRasterBand(1);
		pafScanblock = (BYTE *) CPLMalloc(sizeof(BYTE)*(jieQuSize.cx)*(jieQuSize.cy));  // 分配缓冲区空间
		origonPafScanblock = pafScanblock;
		poBand->RasterIO( GF_Read, startPoint.x*scale, startPoint.y*scale, jieQuSize.cx*scale,
			jieQuSize.cy*scale, pafScanblock, jieQuSize.cx,jieQuSize.cy, GDT_Byte,0, 0 );
		for (i=0; i<jieQuSize.cy; i++)
		{
			for (j=0; j<jieQuSize.cx; j++)
			{
				pBits[j + i*widthBytes] = *pafScanblock;
				pafScanblock++;
			}	
		}
		CPLFree(origonPafScanblock);  // 释放缓冲区空间
		break;
	case 3:
		GDALRasterBand *poBand1, *poBand2, *poBand3;
		BYTE *pafScanblock1, *origonPafScanblock1;
		BYTE *pafScanblock2, *origonPafScanblock2;
		BYTE *pafScanblock3, *origonPafScanblock3;

		poBand1 = poDataset->GetRasterBand(1);
		poBand2 = poDataset->GetRasterBand(2);
		poBand3 = poDataset->GetRasterBand(3);
		pafScanblock1 = (BYTE *) CPLMalloc(sizeof(BYTE)*(jieQuSize.cx)*(jieQuSize.cy));
		origonPafScanblock1 = pafScanblock1;
		pafScanblock2 = (BYTE *) CPLMalloc(sizeof(BYTE)*(jieQuSize.cx)*(jieQuSize.cy));
		origonPafScanblock2 = pafScanblock2;
		pafScanblock3 = (BYTE *) CPLMalloc(sizeof(BYTE)*(jieQuSize.cx)*(jieQuSize.cy));
		origonPafScanblock3 = pafScanblock3;
		poBand1->RasterIO( GF_Read, startPoint.x*scale, startPoint.y*scale, jieQuSize.cx*scale,
			jieQuSize.cy*scale, pafScanblock1, jieQuSize.cx,jieQuSize.cy, GDT_Byte,0, 0 );
		poBand2->RasterIO( GF_Read, startPoint.x*scale, startPoint.y*scale, jieQuSize.cx*scale,
			jieQuSize.cy*scale, pafScanblock2, jieQuSize.cx,jieQuSize.cy, GDT_Byte,0, 0 );
		poBand3->RasterIO( GF_Read, startPoint.x*scale, startPoint.y*scale, jieQuSize.cx*scale,
			jieQuSize.cy*scale, pafScanblock3, jieQuSize.cx,jieQuSize.cy, GDT_Byte,0, 0 );
		for (i=0; i<jieQuSize.cy; i++)
		{
			for (j=0; j<jieQuSize.cx; j++)
			{
				pBits[j + i*widthBytes] = (*pafScanblock1*30 + *pafScanblock2*59 + *pafScanblock3*11)/100;
				pafScanblock1++;
				pafScanblock2++;
				pafScanblock3++;
			}
		}
		CPLFree(origonPafScanblock1);
		CPLFree(origonPafScanblock2);
		CPLFree(origonPafScanblock3);
		break;
	}

	::GlobalUnlock( hGlobal );
	return pBFH;
}

////////////////////////////////////////////////////////////////////////
//void KillImage()		    
//----------------------------------------------------------------------
//基本功能：清除当前CDibObject类中的图像对象。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：无
////////////////////////////////////////////////////////////////////////
void CDibObject::KillImage( void )
{

	if( m_hDib ) ::GlobalFree( m_hDib );
	m_hDib = NULL;

	if( m_pszFilename != NULL ) delete [] m_pszFilename;
	m_pszFilename = NULL;

	m_Palette.DeleteObject();
}

////////////////////////////////////////////////////////////////////////
//void ProcessImageHeader()		    
//----------------------------------------------------------------------
//基本功能：本函数处理图像信息头各项内容。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：无
////////////////////////////////////////////////////////////////////////
void CDibObject::ProcessImageHeader( void )
{
	m_nLastError = IMAGELIB_HDIB_NULL;
	if( m_hDib == NULL ) return;

	char *pTemp;
	BITMAPINFOHEADER *pBIH;
	pTemp = (char *) ::GlobalLock( m_hDib );
	m_nLastError = IMAGELIB_MEMORY_LOCK_ERROR;
	if( pTemp == NULL ) return;
	
	//得到图像信息头指针
	pBIH = (BITMAPINFOHEADER *) &pTemp[sizeof(BITMAPFILEHEADER)];
	m_nWidth = pBIH->biWidth;
	m_nHeight = pBIH->biHeight;
	m_nPlanes = pBIH->biPlanes;
	m_nBits = pBIH->biBitCount;
	m_nColors = 1 << m_nBits;
	if( m_nPlanes > 1 ) m_nColors <<= ( m_nPlanes - 1 );
	if( m_nBits >= 16 ) m_nColors = 0;

		
	int nWidthBytes = WidthBytes(m_nBits, m_nWidth);
	pBIH->biSizeImage = nWidthBytes * m_nHeight;

	::GlobalUnlock( m_hDib );

	m_nLastError = IMAGELIB_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
//void ProcessPalette()		    
//----------------------------------------------------------------------
//基本功能：处理体调色板。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：无
////////////////////////////////////////////////////////////////////////
void CDibObject::ProcessPalette( void )
{
	m_nLastError = IMAGELIB_HDIB_NULL;
	if( m_hDib == NULL ) return;

	CWindowDC WindowDC( NULL );
	m_nScreenPlanes = WindowDC.GetDeviceCaps( PLANES );
	m_nScreenBits = WindowDC.GetDeviceCaps( BITSPIXEL );

	m_nPaletteBytes = 0;
	m_Palette.DeleteObject();

	if( m_nBits <= 8 ) m_nPaletteBytes = m_nColors * sizeof( RGBQUAD );
	if( m_nScreenBits >= 16 ) return;

	char *pTemp;
	pTemp = (char *) ::GlobalLock( m_hDib );
	m_nLastError = IMAGELIB_MEMORY_LOCK_ERROR;
	if( pTemp == NULL ) return;

	if( m_nBits <= 8 ){
		RGBQUAD *pRGBPalette;
		pRGBPalette = (RGBQUAD *) &pTemp[sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)];
		LOGPALETTE *pPalette;
		pPalette = CreateLogPalette( pRGBPalette, m_nColors );
		if( pPalette == NULL ){
			m_nLastError = IMAGELIB_LOGICAL_PALETTE_CREATION_ERROR;
			goto ProcessPaletteExit;
			}
		m_Palette.CreatePalette( pPalette );
		delete [] pPalette;
		}

	m_nLastError = IMAGELIB_SUCCESS;

ProcessPaletteExit:
	::GlobalUnlock( m_hDib );
}

////////////////////////////////////////////////////////////////////////
//BOOL Draw( CDC *pDC, int nX, int nY )		    
//----------------------------------------------------------------------
//基本功能：本函数是在设备描述表上绘制图像。如果X和Y坐标没有给出，图像会
//			被画在(0,0)坐标或上次传入的有效坐标处。
//----------------------------------------------------------------------
//参数说明：CDC *pDC
//			int nX，默认为-1
//			int nY，默认为-1
//----------------------------------------------------------------------
//返 回 值：BOOL：成功返回TRUE，失败返回FALSE
////////////////////////////////////////////////////////////////////////
BOOL CDibObject::Draw( CDC *pDC, int nX, int nY )
{
	if( nX != -1 ) m_nX = nX;
	if( nY != -1 ) m_nY = nY;

	m_nLastError = IMAGELIB_HDIB_NULL;

	//没有打开图像
	if( m_hDib == NULL ) return( FALSE );

	char *pTemp;
	//锁定图像对象句柄
	pTemp = (char *) ::GlobalLock( m_hDib );
	m_nLastError = IMAGELIB_MEMORY_LOCK_ERROR;
	if( pTemp == NULL ) return( NULL );


//	CRect rcWin;
//	GetWindowRect(pDC->GetWindow()->m_hWnd,&rcWin);
	//图像信息头
	BITMAPINFOHEADER *pBIH;
	pBIH = (BITMAPINFOHEADER *) &pTemp[sizeof(BITMAPFILEHEADER)];

	int nRet = ::StretchDIBits( pDC->m_hDC, m_nX, m_nY, m_nWidth, m_nHeight, 0, 0, 
								m_nWidth, m_nHeight, 
								(const void FAR *) &pTemp[sizeof(BITMAPFILEHEADER)+
														  sizeof(BITMAPINFOHEADER)+
														  m_nPaletteBytes], 
								(BITMAPINFO *) pBIH, DIB_RGB_COLORS, SRCCOPY );

	::GlobalUnlock( m_hDib );
	m_nLastError = IMAGELIB_STRETCHDIBITS_ERROR;
	if( nRet != m_nHeight ) return( FALSE );
	m_nLastError = IMAGELIB_SUCCESS;

	return( TRUE );
}

// 绘制图像
BOOL CDibObject::DrawPixel( CDC *pDC, int nX, int nY)
{
	unsigned char *pTemp;
	int nWidthBytes;
	int x, y;

	if( nX != -1 ) m_nX = nX;
	if( nY != -1 ) m_nY = nY;

	m_nLastError = IMAGELIB_HDIB_NULL;

	//没有打开图像
	if( m_hDib == NULL ) return( FALSE );

	//锁定图像对象句柄
	pTemp = (unsigned char *) ::GlobalLock( m_hDib );
	m_nLastError = IMAGELIB_MEMORY_LOCK_ERROR;
	if( pTemp == NULL ) return( NULL );

	//图像信息头
	BITMAPFILEHEADER *pBFH;
	pBFH = (BITMAPFILEHEADER *) pTemp;

	pTemp += pBFH->bfOffBits;
	nWidthBytes = WidthBytes(m_nBits, m_nWidth);

	for(x=0; x<m_nWidth; x++)
	{
		for(y=0; y<m_nHeight; y++)
		{
			if(pTemp[x + y*nWidthBytes] <= 3)
			{
				pDC->SetPixel(x + m_nX, (m_nHeight - 1 - y) + m_nY, RGB(0, 0, 0));
			}
		}
	}

	::GlobalUnlock( m_hDib );
	m_nLastError = IMAGELIB_SUCCESS;

	return( TRUE );
}

BOOL CDibObject::DrawWhite( CDC *pDC, int nX, int nY)
{
	unsigned char *pTemp;
	int nWidthBytes;
	int x, y;

	if( nX != -1 ) m_nX = nX;
	if( nY != -1 ) m_nY = nY;

	m_nLastError = IMAGELIB_HDIB_NULL;

	//没有打开图像
	if( m_hDib == NULL ) return( FALSE );

	//锁定图像对象句柄
	pTemp = (unsigned char *) ::GlobalLock( m_hDib );
	m_nLastError = IMAGELIB_MEMORY_LOCK_ERROR;
	if( pTemp == NULL ) return( NULL );

	//图像信息头
	BITMAPFILEHEADER *pBFH;
	pBFH = (BITMAPFILEHEADER *) pTemp;

	pTemp += pBFH->bfOffBits;
	nWidthBytes = WidthBytes(m_nBits, m_nWidth);

	for(x=0; x<m_nWidth; x++)
	{
		for(y=0; y<m_nHeight; y++)
		{
			pDC->SetPixel(x,y, RGB(255,255,255));
		}
	}

	::GlobalUnlock( m_hDib );
	m_nLastError = IMAGELIB_SUCCESS;

	return( TRUE );
}

////////////////////////////////////////////////////////////////////////
//int WidthBytes()		    
//----------------------------------------------------------------------
//基本功能：本函数根据传入的像素位数及图像宽度（像素）计算其字节宽度。
//----------------------------------------------------------------------
//参数说明：int nBits	每像素位数
//			int nWidth	图像的宽度
//----------------------------------------------------------------------
//返 回 值：int	图像的字节宽度
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
int CDibObject::WidthBytes( int nBits, int nWidth )
{
	int nWidthBytes;

	nWidthBytes = nWidth;
	if( nBits == 1 ) nWidthBytes = ( nWidth + 7 ) / 8;
	else if( nBits == 4 ) nWidthBytes = ( nWidth + 1 ) / 2;
	else if( nBits == 16 ) nWidthBytes = nWidth * 2;
	else if( nBits == 24 ) nWidthBytes = nWidth * 3;
	else if( nBits == 32 ) nWidthBytes = nWidth * 4;

	//一个数的后两位为零，则该数为4的整数倍
	while( ( nWidthBytes & 3 ) != 0 ) nWidthBytes++;

	return( nWidthBytes );
}

////////////////////////////////////////////////////////////////////////
//BOOL CreateLogPalette()		    
//----------------------------------------------------------------------
//基本功能：本函数根据指定的调色板指针及颜色数创建一个逻辑调色板。
//----------------------------------------------------------------------
//参数说明：CDC *pDC
//			int nX，默认为-1
//			int nY，默认为-1
//----------------------------------------------------------------------
//返 回 值：创建的逻辑调色板指针
////////////////////////////////////////////////////////////////////////
LOGPALETTE *CDibObject::CreateLogPalette( RGBQUAD *pPalette, int nNumColors )
{
	LOGPALETTE *pLogPal;
	int i;

	if( pPalette == NULL ) return( NULL );

	pLogPal = (LOGPALETTE *) new char [sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY)];
	if( pLogPal == NULL ) return( NULL );

	//指定调色板版本号
	pLogPal->palVersion = 0x300;
	//指定调色板颜色数
	pLogPal->palNumEntries = (unsigned short) nNumColors;
	
	//复制调色板各颜色项
	for( i=0; i<nNumColors; i++ )
	{
		pLogPal->palPalEntry[i].peRed = pPalette[i].rgbRed;
		pLogPal->palPalEntry[i].peGreen = pPalette[i].rgbGreen;
		pLogPal->palPalEntry[i].peBlue = pPalette[i].rgbBlue;
		pLogPal->palPalEntry[i].peFlags = 0;
	}

	return( pLogPal );
}

////////////////////////////////////////////////////////////////////////
//int GetWidth()
//----------------------------------------------------------------------
//基本功能：本函数返回图像的像素宽度数。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：int nWidth
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
int CDibObject::GetWidth( void )
{
	return( m_nWidth );
}

////////////////////////////////////////////////////////////////////////
//int GetHeight( void )
//----------------------------------------------------------------------
//基本功能：本函数返回图像的像素高度数。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：int nHeight
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
int CDibObject::GetHeight( void )
{
	return( m_nHeight );
}

// 得到字节宽度
int CDibObject::GetWidthBytes( void )
{
	return WidthBytes(m_nBits, m_nWidth);
}

////////////////////////////////////////////////////////////////////////
//BOOL SetPalette( CDC *pDC )		    
//----------------------------------------------------------------------
//基本功能：本函数将该设备描述表的调色板分配给相应的图像。当一个图像被载
//			入时，它的调色板信息便被存入CDibObject对象中。
//----------------------------------------------------------------------
//参数说明：CDC *pDC
//----------------------------------------------------------------------
//返 回 值：BOOL：成功返回TRUE，失败返回FALSE
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
BOOL CDibObject::SetPalette( CDC *pDC )
{
	m_nLastError = IMAGELIB_HDIB_NULL;
	if( m_hDib == NULL ) return( FALSE );

	pDC->SelectPalette( &m_Palette, FALSE );
	pDC->RealizePalette();

	m_nLastError = IMAGELIB_SUCCESS;

	return( TRUE );
}

////////////////////////////////////////////////////////////////////////
//HGLOBALBOOL GetDib( void )	    
//----------------------------------------------------------------------
//基本功能：本函数返回当前载入的图像Dib句柄。
//----------------------------------------------------------------------
//参数说明：无
//----------------------------------------------------------------------
//返 回 值：HGLOBALBOOL hDib
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
HGLOBAL CDibObject::GetDib( void )
{
	return( m_hDib );
}

//获取合成通道
BOOL CDibObject::GetMixedChannel(CDibObject *&pMixed)
{
	// 清除pMixed图像
	if(pMixed !=NULL)
	{
		delete pMixed;
	}

	// 已是256色灰度图像不用再转换
	if(IsGray256())
	{
		pMixed = new CDibObject(this);
		return TRUE;
	}

	MakeGray256(0, pMixed);

	return TRUE;
}

//获取红色通道
BOOL CDibObject::GetRedChannel(CDibObject *&pMixed)
{
	// 清除pMixed图像
	if(pMixed !=NULL)
	{
		delete pMixed;
	}

	// 已是256色灰度图像不用再转换
	if(IsGray256())
	{
		pMixed = new CDibObject(this);
		return TRUE;
	}

	MakeGray256(1, pMixed);

	return TRUE;
}

//获取绿色通道
BOOL CDibObject::GetGreenChannel(CDibObject *&pMixed)
{
	// 清除pMixed图像
	if(pMixed !=NULL)
	{
		delete pMixed;
	}

	// 已是256色灰度图像不用再转换
	if(IsGray256())
	{
		pMixed = new CDibObject(this);
		return TRUE;
	}

	MakeGray256(2, pMixed);

	return TRUE;
}

//获取蓝色通道
BOOL CDibObject::GetBlueChannel(CDibObject *&pMixed)
{
	// 清除pMixed图像
	if(pMixed !=NULL)
	{
		delete pMixed;
	}

	// 已是256色灰度图像不用再转换
	if(IsGray256())
	{
		pMixed = new CDibObject(this);
		return TRUE;
	}

	MakeGray256(3, pMixed);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
//BOOL IsGray256(CDibObject *pDibObject)
//----------------------------------------------------------------------
//基本功能：本函数判断传入的CDibObject对象中的图像是否为256级灰度图像。
//----------------------------------------------------------------------
//参数说明：CDibObject *pDibObject, 默认为NULL
//----------------------------------------------------------------------
//返回：BOOL：成功返回TRUE，失败返回FALSE。
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
BOOL CDibObject::IsGray256()
{

	//不是8位图像，必不是256级灰度图像，不处理返回FALSE
	if( m_nBits != 8 ) return( FALSE );

	//定义变量
	unsigned char *pBuffer;
	RGBQUAD *pPalette;
	int i;

	//pBuffer: 获得位图数据指针
	pBuffer = (unsigned char *)::GlobalLock(m_hDib);
	if( pBuffer == NULL ) return( FALSE );

	//pPalette：获得调色板数据地址
	pPalette = (RGBQUAD *) &pBuffer[sizeof(BITMAPFILEHEADER)+
									sizeof(BITMAPINFOHEADER)];

	//检查是否为256色灰度调色板
	for( i=0; i<256; i++ )
	{
		if( pPalette[i].rgbRed != pPalette[i].rgbGreen 
			|| pPalette[i].rgbRed != pPalette[i].rgbBlue
			|| pPalette[i].rgbGreen != pPalette[i].rgbBlue )
			return( FALSE);
	}
	::GlobalUnlock( m_hDib);

	return( TRUE );
}

////////////////////////////////////////////////////////////////////////
//BOOL MakeGray256(BYTE mGrayType, CDibObject *pDibObject)   
//----------------------------------------------------------------------
//基本功能：本函数将图像从彩色转换为灰度图像并保存于参数图像pDibObject中。
//			如果进行此调整之前没有指定一个CDibObject对象指针，则必须在
//			调整时加以指定。
//----------------------------------------------------------------------
//参数说明：BYTE mGrayType  0：Y=0.3R+0.59G+0.11B 
//							1: Y=R 
//							2: Y=G
//							3: Y=B					
//			CDibObject *pDibObject, 默认为NULL。
//----------------------------------------------------------------------
//返    回：成功返回TRUE，失败返回FALSE。
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
BOOL CDibObject::MakeGray256(BYTE mGrayType, 
									CDibObject *&pDibObject )
{
	//低于8位的图像不进行处理
	if( m_nBits < 8 ) return( FALSE );

	unsigned char *pOldBits, *pNewBits;  // 原图像位图数据指针
	BITMAPFILEHEADER *pOldBFH, *pNewBFH;  // 图像位图文件头指针
	BITMAPINFOHEADER *pOldBIH, *pNewBIH;  // 图像位图信息头指针
	RGBQUAD *pOldRGBPalette, *pNewRGBPalette;  // 图像位图调色板指针
	int nOldWidthBytes, nNewWidthBytes;  // 图像位图字节宽度
	int nBits = 8;  // 新图像位图每像素位数
	int nNumColors = 256;  // 新图像位图颜色数
	DWORD dwSize;  //新图像总字节数
	HGLOBAL hGlobal;  // 为新图像位图分配内存
	int x, y;  //循环变量定义
	unsigned char *pLookup;	//调色板查找表
	DWORD dwGray;	//灰度级别
	unsigned char *pTemp, *pNewTemp;

	pOldBits = (unsigned char *)::GlobalLock(m_hDib);
	if( pOldBits == NULL ) return( FALSE );
	// 获取原图像位图文件头指针
	pOldBFH = (BITMAPFILEHEADER *) pOldBits;
	// 获取原图像位图信息头指针
	pOldBIH = (BITMAPINFOHEADER *) &pOldBits[sizeof(BITMAPFILEHEADER)];
	// 获取原图像位图调色板指针
	pOldRGBPalette = (RGBQUAD *) &pOldBits[sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)];
	// 获取原图像位图数据指针
	pOldBits += pOldBFH->bfOffBits;
	nOldWidthBytes = WidthBytes(m_nBits, m_nWidth);

	pDibObject = new CDibObject(this);
	nNewWidthBytes = WidthBytes(nBits, m_nWidth);
	dwSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) +
			nNumColors * sizeof( RGBQUAD ) + m_nHeight * nNewWidthBytes;
	hGlobal = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize );
	if( hGlobal == NULL )
	{
		::GlobalUnlock( m_hDib );
		return( FALSE );
	}

	pNewBits = (unsigned char *) ::GlobalLock( hGlobal );
	if( pNewBits == NULL )
	{
		::GlobalFree( hGlobal );
		::GlobalUnlock( m_hDib );
		return( FALSE );
	}

	//获得新图像的文件头指针
	pNewBFH = (BITMAPFILEHEADER *) pNewBits;
	//获得新图像的信息头指针
	pNewBIH = (BITMAPINFOHEADER *) &pNewBits[sizeof(BITMAPFILEHEADER)];
	//获得新图像的调色板指针
	pNewRGBPalette = (RGBQUAD *) &pNewBits[sizeof(BITMAPFILEHEADER)
						+sizeof(BITMAPINFOHEADER)];
	//复制原图像位图文件头数据、信息头数据到新图像位图
	memcpy(pNewBFH, pOldBFH, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

	pNewBIH->biBitCount = nBits;
	pNewBIH->biSizeImage = nNewWidthBytes * m_nHeight;
	pNewBIH->biClrUsed = nNumColors;
	pNewBFH->bfOffBits =	sizeof( BITMAPFILEHEADER ) + 
							sizeof( BITMAPINFOHEADER ) + 
							nNumColors * sizeof( RGBQUAD );
	pNewBFH->bfSize = pNewBFH->bfOffBits + pNewBIH->biSizeImage;
	pNewBits += pNewBFH->bfOffBits;
	pDibObject->SetPaletteBytes( nNumColors * sizeof( RGBQUAD ));
	
	//创建256色灰度调色板
	for( x = 0; x < nNumColors; x++ )
	{
		pNewRGBPalette[x].rgbRed = x;
		pNewRGBPalette[x].rgbGreen = x;
		pNewRGBPalette[x].rgbBlue = x;
	}

	switch( m_nBits )
	{
	case 8:		//256色图像
		pLookup = new unsigned char [256];
		if( pLookup == NULL ) break;
		memset( pLookup, 0, 256 );	//调色板查找表清0(256项)
		switch( mGrayType)
		{
		case 0:	//按亮度Y=0.3R+0.59G+0.11B将彩色图像转换为灰度图像
			for( x=0; x<256; x++ )
			{
				dwGray = ( (DWORD) pOldRGBPalette[x].rgbRed * 30 +
					(DWORD) pOldRGBPalette[x].rgbGreen * 59 +
					(DWORD) pOldRGBPalette[x].rgbBlue * 11 ) / 100;
				pLookup[x] = (unsigned char) dwGray;
			}
			break;
		case 1:	//按亮度Y=R将彩色图像转换为灰度图像
			for( x=0; x<256; x++ )
			{
				dwGray =  (DWORD) pOldRGBPalette[x].rgbRed;
				pLookup[x] = (unsigned char) dwGray;
			}
			break;
		case 2:	//按亮度Y=G将彩色图像转换为灰度图像
			for( x=0; x<256; x++ )
			{
				dwGray = (DWORD) pOldRGBPalette[x].rgbGreen;
				pLookup[x] = (unsigned char) dwGray;
			}
			break;
		case 3:	//按亮度Y=B将彩色图像转换为灰度图像
			for( x=0; x<256; x++ )
			{
				dwGray =(DWORD) pOldRGBPalette[x].rgbBlue;
				pLookup[x] = (unsigned char) dwGray;
			}
			break;
		}
		for( y = 0; y < m_nHeight; y++ )
		{
			pTemp = pOldBits;	//位图数据起始指针
			pTemp += y * nOldWidthBytes;	//位图数据下一行起始指针
			//转换成灰度索引
			for( x = 0; x < pOldBIH->biWidth; x++ ) pTemp[x] = pLookup[pTemp[x]];
		}
		delete [] pLookup;	//释放pLookup查找表所占内存
		
		memcpy( pNewBits, pOldBits, nNewWidthBytes * m_nHeight);
		
		break;
	case 16:	//16位色真彩色图像
		unsigned char ucRed, ucGreen, ucBlue;
		for( y=0; y<pOldBIH->biHeight; y++ )
		{
			//位图数据起始指针
			pTemp = pOldBits;	
			pNewTemp = pNewBits;
			//位图数据下一行起始指针
			pTemp += y * nOldWidthBytes;	
			pNewTemp += y * nNewWidthBytes;
			switch( mGrayType )
			{
			case 0:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					GETRGB555( ucRed, ucGreen, ucBlue, &pTemp[x*2] );
					//按亮度Y=0.3R+0.59G+0.11B将彩色图像转换为灰度图像
					dwGray = (ucRed * 30 + ucGreen * 59 +ucBlue * 11) / 100;
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 1:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					GETRGB555( ucRed, ucGreen, ucBlue, &pTemp[x*2] );
					//按亮度Y=R将彩色图像转换为灰度图像
					dwGray = ucRed;
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 2:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					GETRGB555( ucRed, ucGreen, ucBlue, &pTemp[x*2] );
					//按亮度Y=G将彩色图像转换为灰度图像
					dwGray = ucGreen;
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 3:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					GETRGB555( ucRed, ucGreen, ucBlue, &pTemp[x*2] );
					//按亮度Y=B将彩色图像转换为灰度图像
					dwGray = ucBlue;
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			}
		}
		break;
	case 24:	//24位真彩色图像
		for( y=0; y<pOldBIH->biHeight; y++ )
		{
			//位图数据起始指针
			pTemp = pOldBits;	
			pNewTemp = pNewBits;
			//位图数据下一行起始指针
			pTemp += y * nOldWidthBytes;	
			pNewTemp += y * nNewWidthBytes;
			switch( mGrayType )
			{
			case 0:	//按亮度Y=0.3R+0.59G+0.11B将彩色图像转换为灰度图像
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					dwGray = ( (DWORD) pTemp[x*3+2] * 30	//红色
						+(DWORD) pTemp[x*3+1] * 59		//绿色
						+(DWORD) pTemp[x*3] * 11		//兰色
						) / 100;
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 1:	//按亮度Y=R将彩色图像转换为灰度图像
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					dwGray =  (DWORD) pTemp[x*3+2]; //红色
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 2:	//按亮度Y=G将彩色图像转换为灰度图像
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					dwGray = (DWORD) pTemp[x*3+1] ;	//绿色
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 3:	//按亮度Y=B将彩色图像转换为灰度图像
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					dwGray =(DWORD) pTemp[x*3];	//兰色
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			}
		}
		break;
	case 32:	//32位真彩色图像
		for( y=0; y<pOldBIH->biHeight; y++ )
		{
			//位图数据起始指针
			pTemp = pOldBits;	
			pNewTemp = pNewBits;
			//位图数据下一行起始指针
			pTemp += y * nOldWidthBytes;	
			pNewTemp += y * nNewWidthBytes;
			switch( mGrayType )
			{
			case 0:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					//按亮度Y=0.3R+0.59G+0.11B将彩色图像转换为灰度图像
					dwGray = ( (DWORD) pTemp[x*4] * 30 //红色
						+(DWORD) pTemp[x*4+1] * 59	//绿色
						+(DWORD) pTemp[x*4+2] * 11 //兰色
						) / 100;
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 1:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					//按亮度Y=R将彩色图像转换为灰度图像
					dwGray =  (DWORD) pTemp[x*4];//红色
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 2:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					//按亮度Y=G将彩色图像转换为灰度图像
					dwGray = (DWORD) pTemp[x*4+1] ;	//绿色
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			case 3:
				for( x=0; x<pOldBIH->biWidth; x++ )
				{
					//按亮度Y=B将彩色图像转换为灰度图像
					dwGray =(DWORD) pTemp[x*4+2] ; //兰色
					//给新图像数据赋值
					pNewTemp[x] = (unsigned char)dwGray;
				}
				break;
			}
		}
		break;
	}
			
	::GlobalUnlock( m_hDib );
	::GlobalUnlock( hGlobal );

	pDibObject->SetDib( hGlobal );

	pDibObject->ProcessImageHeader();

	pDibObject->m_nLastError = IMAGELIB_SUCCESS;

	return( TRUE );
}

// 获取分割后的图像存放于参数pDibObject中
BOOL CDibObject::GetSubDibObject(CRect &rect, CDibObject *&pDibObject)
{
	//LONG top, bottom;                     // 变量rect原先值
	unsigned char *pOldBits, *pNewBits;   // 图像位图数据起始指针
	BITMAPFILEHEADER *pOldBFH, *pNewBFH;  // 图像位图文件头指针
	DWORD bfOffBits;                      // 位图位图数据距文件头的偏移量
	int nOldWidthBytes, nNewWidthBytes;   // 原图像位图字节宽度和分割后图像字节宽度
	LONG biWidth, biHeight;               // 分割后图像位图的宽和高
	DWORD bfSize;                         // 分割后图像位图文件大小
	BITMAPINFOHEADER *pNewBIH;            // 图像位图信息头指针
	RGBQUAD *pNewRGBPalette;              // 图像位图调色板指针
	HGLOBAL hGlobal;    	              // 为新图像位图所分配的内存
	int startIndex, endIndex;             // 位图数据分割的起始点、终止点及两点间距离
	int i;                                // 循环变量
	LOGPALETTE *pLogPalette;

	if(!CheckRect(rect))
	{
		return FALSE;
	}

	// 清除图像
	if(pDibObject != NULL)
	{
		delete pDibObject;
	}

	m_nLastError = IMAGELIB_HDIB_NULL;
	if( m_hDib == NULL ) return( FALSE );

	pOldBits = (unsigned char *)::GlobalLock( m_hDib );
	m_nLastError = IMAGELIB_MEMORY_LOCK_ERROR;
	if( pOldBits == NULL ) return( FALSE );

	// 获取分割后的图像文件头指针
	pOldBFH = (BITMAPFILEHEADER *) pOldBits;
	bfOffBits = pOldBFH->bfOffBits;

	// 获取原图像字节宽度
	nOldWidthBytes = WidthBytes(m_nBits, m_nWidth);

	// 获取分割后图像的宽和高
	biWidth = rect.Width() + 1;
	biHeight = rect.Height() + 1;

	// 获取分割后图像的字节宽度
	nNewWidthBytes = WidthBytes(m_nBits, biWidth);

	// 获取分割后图像文件大小
	bfSize = bfOffBits + biHeight * nNewWidthBytes;

	//为分割后图像分配内存
	hGlobal = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, bfSize );

	if( hGlobal == NULL )
	{
		::GlobalUnlock( m_hDib );
		return( FALSE );
	}

	pNewBits = (unsigned char *) ::GlobalLock( hGlobal );
	if( pNewBits == NULL )
	{
		::GlobalFree( hGlobal );
		::GlobalUnlock( m_hDib );
		return( FALSE );
	}

	// 从原图像拷贝图像文件头、图像信息头和图像调色板至分割后的图像
	memcpy(pNewBits, pOldBits, bfOffBits);

	// 获取分割后的图像文件头指针
	pNewBFH = (BITMAPFILEHEADER *) pNewBits;
	// 获取分割后的图像信息头指针
	pNewBIH = (BITMAPINFOHEADER *) &pNewBits[sizeof(BITMAPFILEHEADER)];
	// 获取分割后的图像调色板指针
	pNewRGBPalette = (RGBQUAD *) &pNewBits[sizeof(BITMAPFILEHEADER)+
									sizeof(BITMAPINFOHEADER)];

	pNewBIH->biWidth = biWidth;
	pNewBIH->biHeight = biHeight;
	pNewBIH->biSizeImage = nNewWidthBytes * biHeight;
	pNewBFH->bfSize = bfSize;
	
	// 使pOldBits、pNewBits指向位图数据起始位置
	pOldBits += bfOffBits;
	pNewBits += bfOffBits;

	// 获取位图数据分割的起始点和终止点
	switch(m_nBits)
	{
	case 1:
		startIndex = rect.left/8;
		endIndex = rect.right/8;
		break;
	case 4:
		startIndex = rect.left/2;
		endIndex = rect.right/2;
		break;
	case 8:
		startIndex = rect.left;
		endIndex = rect.right;
		break;
	case 16:
		startIndex = rect.left * 2;
		endIndex = rect.right * 2 + 1;
		break;
	case 24:
		startIndex = rect.left * 3;
		endIndex = rect.right * 3 + 2;
		break;
	case 32:
		startIndex = rect.left * 4;
		endIndex = rect.right * 4 + 3;
	}
	
	// 拷贝位图数据文件
	pOldBits += rect.top * nOldWidthBytes + startIndex;
	for(i=0; i<biHeight; i++)
	{
		memcpy(pNewBits, pOldBits, nNewWidthBytes);
		pOldBits += nOldWidthBytes;
		pNewBits += nNewWidthBytes;
	}
	m_nLastError = IMAGELIB_SUCCESS;

	// 获取分割后的图像
	pDibObject = new CDibObject();
	if( m_nBits <= 8 )
	{
		pLogPalette = CreateLogPalette( pNewRGBPalette, m_nColors );
		if( pLogPalette == NULL )
		{
			pDibObject->m_nLastError = IMAGELIB_LOGICAL_PALETTE_CREATION_ERROR;
			::GlobalUnlock(hGlobal);
			::GlobalUnlock(m_hDib);
			return FALSE;
		}
		pDibObject->m_Palette.CreatePalette( pLogPalette );
		delete [] pLogPalette;
	}
	::GlobalUnlock(hGlobal);
	::GlobalUnlock(m_hDib);
	//rect.top = top;
	//rect.bottom = bottom;

	pDibObject->m_hDib = hGlobal;
	pDibObject->m_nBits = m_nBits;
	pDibObject->m_nColors = m_nColors;
	pDibObject->m_nHeight = biHeight;
	pDibObject->m_nImageType = m_nImageType;
	pDibObject->m_nLastError = m_nLastError;
	pDibObject->m_nPaletteBytes = m_nPaletteBytes;
	pDibObject->m_nPaletteCreationType = m_nPaletteCreationType;
	pDibObject->m_nPlanes = m_nPlanes;
	pDibObject->m_nQuality = m_nQuality;
	pDibObject->m_nScreenBits = m_nScreenBits;
	pDibObject->m_nScreenPlanes = m_nScreenPlanes;
	pDibObject->m_nWidth = biWidth;
	pDibObject->m_nX = m_nX;
	pDibObject->m_nY = m_nY;
	
	return TRUE;
}

// 验证矩形所示区域是否是有效区域
BOOL CDibObject::CheckRect(CRect &rect)
{
	if(rect.left < 0)
	{
		rect.left = 0;
	}
	while(rect.left%8 !=0) rect.left++;
	if(rect.right < 0)
	{
		return FALSE;
	}
	if(rect.right >= m_nWidth)
	{
		rect.right = m_nWidth -1;
	}
	if(rect.left >= rect.right)
	{
		return FALSE;
	}
	if(rect.top < 0)
	{
		rect.top = 0;
	}
	if(rect.top >= m_nHeight)
	{
		return FALSE;
	}
	if(rect.bottom < 0)
	{
		return FALSE;
	}
	if(rect.bottom >= m_nHeight)
	{
		rect.bottom = m_nHeight -1;
	}
	if(rect.top >= rect.bottom)
	{
		return FALSE;
	}
	
	return TRUE;
}

// 自动道路提取
BOOL CDibObject::AutoRoadExtract(CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度
	CArray<CLine*,CLine*> roadEdgeLineArray;  // 道路边界线数组
	CGroup *pGroups = NULL;  // 分组数组
	int groupNum;  // 线段数量,即初始分组数
	CArray<CLine*,CLine*> roadArray;  // 道路数组

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];
	// 调试用
	pGUnchEdge = pUnchEdge;
	// 调试完

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];
	// 对原图象进行高斯滤波
	GaussianSmooth(pUnchEdge, m_nWidth, m_nHeight, sigma, pUnchSmooth);

	// 调试用
	//CopyBitmap(pBits, widthBytes, m_nHeight, pUnchSmooth, m_nWidth, m_nHeight);
	// 调试完

	pnGradX = new int[m_nWidth * m_nHeight];
	pnGradY = new int[m_nWidth * m_nHeight];
	// 计算方向导数
	DirGrad(pUnchSmooth, m_nWidth, m_nHeight, pnGradX, pnGradY);

	pnGradMag = new int[m_nWidth * m_nHeight];
	// 计算梯度的幅度
	GradMagnitude(pnGradX, pnGradY, m_nWidth, m_nHeight, pnGradMag);

	// 应用non-maximum 抑制
	NonmaxSuppress(pnGradMag, pnGradX, pnGradY, m_nWidth, m_nHeight, pUnchEdge);

	// 应用Hysteresis，找到所有的边界
	Hysteresis(pnGradMag, m_nWidth, m_nHeight, dRatioLow, dRatioHigh, pUnchEdge);

	// 将pBits所指向位图的位图数据复制到pUnchSmooth所指向的位图
	CopyBitmap(pUnchSmooth, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	// 获取道路边界记录于变量roadEdgeLineArray中	
	GetRoadEdgeLine(pUnchEdge, m_nWidth, m_nHeight, roadEdgeLineArray, pUnchSmooth, pnGradMag);

	// 初始化分组数组
	InitGroups(pGroups, groupNum, roadEdgeLineArray);

	// 层次记号编组
	CengCiBianZu(roadEdgeLineArray, pGroups, groupNum);

	// 计算置信度
	CalBeliefDegree(pGroups, groupNum);

	// 获取道路边界点
	GetRoadEdgePoint(pGroups, groupNum, roadEdgeLineArray, pnGradMag, pUnchSmooth, m_nWidth, m_nHeight);

	// 调试用
	// 将边界线信息记录于文本中
//	RecordRoadEdgeLineToText(roadEdgeLineArray);
	// 将编组信息记录于文本中
//	RecordGroupsToText(pGroups, groupNum);
	// 画出所有道路边界线
//	PaintAllRoadEdgeLine(pUnchEdge,m_nWidth,roadEdgeLineArray);
	
	
	// 画出单条道路边界线
	//PaintOneRoadEdgeLine(pUnchEdge,m_nWidth,roadEdgeLineArray,12);
	//PaintOneRoadEdgeLine(pUnchEdge,m_nWidth,roadEdgeLineArray,12);
	//PaintOneRoadEdgeLine(pUnchEdge,m_nWidth,roadEdgeLineArray,3);
	// 调试完

	// 根据道路种子点跟踪道路
	TrackRoadByGroup(pGroups, groupNum, pUnchSmooth, m_nWidth, m_nHeight, roadArray);

	// 知识推理减少虚警
	FindRoad(roadArray, m_nWidth, m_nHeight);

	// 清除位图pUnchEdge中数据
	ClearBitmap(pUnchEdge, m_nWidth, m_nHeight);

	// 画出所有道路
	PaintAllRoad(pUnchEdge, m_nWidth, roadArray);

	// 调试用
	// 将边界线信息记录于文本中
	RecordRoadEdgeLineToText(roadArray);
	// 调试完

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	// 调试用
	// 画出所有道路边界线
	//PaintAllRoadEdgeLine(pBits, widthBytes, roadEdgeLineArray);
	//for(int i=0; i<groupNum; i++)
	//{
		//if(pGroups[i].m_believed)
		//{
			//pBits[pGroups[i].m_seedPoint1.x + pGroups[i].m_seedPoint1.y * widthBytes] = 0;
			//pBits[pGroups[i].m_seedPoint2.x + pGroups[i].m_seedPoint2.y * widthBytes] = 0;
		//}
	//}
	// 调试完
	
	CopyBitmapPixel(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);

	::GlobalUnlock(pDibObject->m_hDib);

	// 清除roadArray数组
	ClearLineArray(roadArray);

	if(pGroups != NULL)
	{
		delete []pGroups;
	}

	// 清除roadEdgeLineArray数组
	ClearLineArray(roadEdgeLineArray);

	delete []pnGradMag;
	delete []pnGradY;
	delete []pnGradX;
	delete []pUnchSmooth;
	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
}

// 图像位图数据复制
void CDibObject::CopyBitmap(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight)
{
	int width, height;  // 宽度
	int x, y;  // 循环变量

	width = destWidth < srcWidth? destWidth: srcWidth;
	height = destHeight < srcHeight? destHeight: srcHeight;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			pDestBits[x + y*destWidth] = pSrcBits[x + y*srcWidth];
		 	//pDestBits[x + y*destWidth] = 255;
		}
	}
}

// 图像位图数据复制，只复制灰度小于8的点
void CDibObject::CopyBitmapPixel(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight)
{
	int width, height;  // 宽度
	int x, y;  // 循环变量

	width = destWidth < srcWidth? destWidth: srcWidth;
	height = destHeight < srcHeight? destHeight: srcHeight;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			if(pSrcBits[x + y*srcWidth] < 8)
			{
				pDestBits[x + y*destWidth] = pSrcBits[x + y*srcWidth];
			}
		}
	}
}

/*void CDibObject::CopyPixel(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight)
{
	int width, height;  // 宽度
	int x, y;  // 循环变量

	width = destWidth < srcWidth? destWidth: srcWidth;
	height = destHeight < srcHeight? destHeight: srcHeight;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{	
			pDestBits[x + y*destWidth] = pSrcBits[x + y*srcWidth];	
		}
	}
}
*/

/*************************************************************************
 *
 * \函数名称：
 *   GaussianSmooth()
 *
 * \输入参数:
 *   unsigned char * pUnchImg				- 指向图象数据的指针
 *   int nWidth								- 图象数据宽度
 *   int nHeight							- 图象数据高度
 *   double dSigma							- 高斯函数的标准差
 *   unsigned char * pUnchSmthdImg	        - 指向经过平滑之后的图象数据
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   为了抑止噪声，采用高斯滤波对图象进行滤波，滤波先对x方向进行，然后对
 *   y方向进行。
 *   
 *************************************************************************
 */
void CDibObject::GaussianSmooth(unsigned char *pUnchImg, int nWidth, int nHeight, 
										double sigma, unsigned char * pUnchSmthdImg)
{
	// 循环控制变量
	int y;
	int x;
	int i;
	
	// 高斯滤波器的数组长度
	int nWindowSize;
	
	//  窗口长度的1/2
	int	nHalfLen;   
	
	// 一维高斯数据滤波器
	double *pdKernel ;
	
	// 高斯系数与图象数据的点乘
	double  dDotMul     ;
	
	// 高斯滤波系数的总和
	double  dWeightSum     ;          
  
	// 中间变量
	double * pdTmp ;
	
	// 分配内存
	pdTmp = new double[nWidth*nHeight];
	
	// 产生一维高斯数据滤波器
	MakeGauss(sigma, &pdKernel, &nWindowSize) ;
	
	// MakeGauss返回窗口的长度，利用此变量计算窗口的半长
	nHalfLen = nWindowSize / 2;
	
	// x方向进行滤波
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			dDotMul		= 0;
			dWeightSum = 0;
			for(i=(-nHalfLen); i<=nHalfLen; i++)
			{
				// 判断是否在图象内部
				if( (i+x) >= 0  && (i+x) < nWidth )
				{
					dDotMul += (double)pUnchImg[y*nWidth + (i+x)] * pdKernel[nHalfLen+i];
					dWeightSum += pdKernel[nHalfLen+i];
				}
			}
			pdTmp[y*nWidth + x] = dDotMul/dWeightSum ;
		}
	}
	
	// y方向进行滤波
	for(x=0; x<nWidth; x++)
	{
		for(y=0; y<nHeight; y++)
		{
			dDotMul		= 0;
			dWeightSum = 0;
			for(i=(-nHalfLen); i<=nHalfLen; i++)
			{
				// 判断是否在图象内部
				if( (i+y) >= 0  && (i+y) < nHeight )
				{
					dDotMul += (double)pdTmp[(y+i)*nWidth + x] * pdKernel[nHalfLen+i];
					dWeightSum += pdKernel[nHalfLen+i];
				}
			}
			pUnchSmthdImg[y*nWidth + x] = (unsigned char)(int)(dDotMul/dWeightSum);
		}
	}

	// 释放内存
	delete []pdKernel;
	pdKernel = NULL ;
	
	delete []pdTmp;
	pdTmp = NULL;
}

/*************************************************************************
 *
 * \函数名称：
 *   MakeGauss()
 *
 * \输入参数:
 *   double sigma									        - 高斯函数的标准差
 *   double **pdKernel										- 指向高斯数据数组的指针
 *   int *pnWindowSize										- 数据的长度
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   这个函数可以生成一个一维的高斯函数的数字数据，理论上高斯数据的长度应
 *   该是无限长的，但是为了计算的简单和速度，实际的高斯数据只能是有限长的
 *   pnWindowSize就是数据长度
 *   
 *************************************************************************
 */
void CDibObject::MakeGauss(double sigma, double **pdKernel, int *pnWindowSize)
{
	// 循环控制变量
	int i   ;
	
	// 数组的中心点
	int nCenter;

	// 数组的某一点到中心点的距离
	double  dDis  ; 

	// 中间变量
	double  dValue; 
	double  dSum  ;
	dSum = 0 ; 
	
	// 数组长度，根据概率论的知识，选取[-3*sigma, 3*sigma]以内的数据。
	// 这些数据会覆盖绝大部分的滤波系数
	*pnWindowSize = int(1 + 2 * ceil(3 * sigma));
	
	// 中心
	nCenter = (*pnWindowSize) / 2;
	
	// 分配内存
	*pdKernel = new double[*pnWindowSize] ;
	
	for(i=0; i< (*pnWindowSize); i++)
	{
		dDis = (double)(i - nCenter);
		dValue = exp(-(1/2)*dDis*dDis/(sigma*sigma)) / (sqrt(2 * PI) * sigma );
		(*pdKernel)[i] = dValue ;
		dSum += dValue;
	}
	
	// 归一化
	for(i=0; i<(*pnWindowSize) ; i++)
	{
		(*pdKernel)[i] /= dSum;
	}
}

/*************************************************************************
 *
 * \函数名称：
 *   DirGrad()
 *
 * \输入参数:
 *   unsigned char *pUnchSmthdImg					- 经过高斯滤波后的图象
 *   int nWidht														- 图象宽度
 *   int nHeight      										- 图象高度
 *   int *pnGradX                         - x方向的方向导数
 *   int *pnGradY                         - y方向的方向导数
 * \返回值:
 *   无
 *
 * \说明:
 *   这个函数计算方向倒数，采用的微分算子是(-1 0 1) 和 (-1 0 1)'(转置)
 *   计算的时候对边界象素采用了特殊处理
 *   
 *   
 *************************************************************************
 */
void CDibObject::DirGrad(unsigned char *pUnchSmthdImg, int nWidth, int nHeight,
										 int *pnGradX , int *pnGradY)
{
	// 循环控制变量
	int y ;
	int x ;
	
	// 计算x方向的方向导数，在边界出进行了处理，防止要访问的象素出界
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			pnGradX[y*nWidth+x] = (int) ( pUnchSmthdImg[y*nWidth+min(nWidth-1,x+1)]  
													          - pUnchSmthdImg[y*nWidth+max(0,x-1)]     );
		}
	}

	// 计算y方向的方向导数，在边界出进行了处理，防止要访问的象素出界
	for(x=0; x<nWidth; x++)
	{
		for(y=0; y<nHeight; y++)
		{
			pnGradY[y*nWidth+x] = (int) ( pUnchSmthdImg[min(nHeight-1,y+1)*nWidth + x]  
																		- pUnchSmthdImg[max(0,y-1)*nWidth+ x ]     );
		}
	}
}

/*************************************************************************
 *
 * \函数名称：
 *   GradMagnitude()
 *
 * \输入参数:
 *   int *pnGradX                         - x方向的方向导数
 *   int *pnGradY                         - y方向的方向导数
 *   int nWidht														- 图象宽度
 *   int nHeight      										- 图象高度
 *   int *pnMag                           - 梯度幅度   
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   这个函数利用方向倒数计算梯度幅度，方向倒数是DirGrad函数计算的结果
 *   
 *************************************************************************
 */
void CDibObject::GradMagnitude(int *pnGradX, int *pnGradY, int nWidth, int nHeight, int *pnMag)
{
	
	// 循环控制变量
	int y ;
	int x ;

	// 中间变量
	double dSqtOne;
	double dSqtTwo;
	
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			dSqtOne = pnGradX[y*nWidth + x] * pnGradX[y*nWidth + x];
			dSqtTwo = pnGradY[y*nWidth + x] * pnGradY[y*nWidth + x];
			pnMag[y*nWidth + x] = (int)(sqrt(dSqtOne + dSqtTwo) + 0.5);
		}
	}
}

/*************************************************************************
 *
 * \函数名称：
 *   NonmaxSuppress()
 *
 * \输入参数:
 *   int *pnMag                - 梯度图
 *   int *pnGradX							 - x方向的方向导数	
 *   int *pnGradY              - y方向的方向导数
 *   int nWidth                - 图象数据宽度
 *   int nHeight               - 图象数据高度
 *   unsigned char *pUnchRst   - 经过NonmaxSuppress处理后的结果
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   抑止梯度图中非局部极值点的象素。
 *   
 *************************************************************************
 */
void CDibObject::NonmaxSuppress(int *pnMag, int *pnGradX, int *pnGradY, int nWidth, 
										int nHeight,	unsigned char *pUnchRst)
{
	// 循环控制变量
	int y ;
	int x ;
	int nPos;

	// x方向梯度分量
	int gx  ;
	int gy  ;

	// 临时变量
	int g1, g2, g3, g4 ;
	double weight  ;
	double dTmp1   ;
	double dTmp2   ;
	double dTmp    ;
	
	// 设置图象边缘部分为不可能的边界点
	for(x=0; x<nWidth; x++)		
	{
		pUnchRst[x] = 255;
		pUnchRst[(nHeight-1)*nWidth+x] = 255;
	}
	for(y=0; y<nHeight; y++)		
	{
		pUnchRst[y*nWidth] = 255;
		pUnchRst[y*nWidth + nWidth-1] = 255;
	}

	for(y=1; y<nHeight-1; y++)
	{
		for(x=1; x<nWidth-1; x++)
		{
			nPos = y*nWidth + x ;
			
			// 如果当前象素的梯度幅度为0，则不是边界点
			if(pnMag[nPos] == 0 )
			{
				pUnchRst[nPos] = 255 ;
			}
			else
			{
				// 当前象素的梯度幅度
				dTmp = pnMag[nPos] ;
				
				// x，y方向导数
				gx = pnGradX[nPos]  ;
				gy = pnGradY[nPos]  ;

				// 如果方向导数y分量比x分量大，说明导数的方向更加“趋向”于y分量。
				if (abs(gy) > abs(gx)) 
				{
					// 计算插值的比例
					weight = fabs(gx)/fabs(gy); 

					g2 = pnMag[nPos-nWidth] ; 
					g4 = pnMag[nPos+nWidth] ;
					
					// 如果x，y两个方向的方向导数的符号相同
					// C是当前象素，与g1-g4的位置关系为：
					//	g1 g2 
					//		 C         
					//		 g4 g3 
					if (gx*gy > 0) 
					{ 					
						g1 = pnMag[nPos-nWidth-1] ;
						g3 = pnMag[nPos+nWidth+1] ;
					} 

					// 如果x，y两个方向的方向导数的符号相反
					// C是当前象素，与g1-g4的位置关系为：
					//	   g2 g1
					//		 C         
					//	g3 g4  
					else 
					{ 
						g1 = pnMag[nPos-nWidth+1] ;
						g3 = pnMag[nPos+nWidth-1] ;
					} 
				}
				
				// 如果方向导数x分量比y分量大，说明导数的方向更加“趋向”于x分量
				// 这个判断语句包含了x分量和y分量相等的情况
				else
				{
					// 计算插值的比例
					weight = fabs(gy)/fabs(gx); 
					
					g2 = pnMag[nPos+1] ; 
					g4 = pnMag[nPos-1] ;
					
					// 如果x，y两个方向的方向导数的符号相同
					// C是当前象素，与g1-g4的位置关系为：
					//	g3   
					//	g4 C g2       
					//       g1
					if (gx*gy > 0) 
					{				
						g1 = pnMag[nPos+nWidth+1] ;
						g3 = pnMag[nPos-nWidth-1] ;
					} 
					// 如果x，y两个方向的方向导数的符号相反
					// C是当前象素，与g1-g4的位置关系为：
					//	     g1
					//	g4 C g2       
					//  g3     
					else 
					{ 
						g1 = pnMag[nPos-nWidth+1] ;
						g3 = pnMag[nPos+nWidth-1] ;
					}
				}

				// 下面利用g1-g4对梯度进行插值
				{
					dTmp1 = weight*g1 + (1-weight)*g2 ;
					dTmp2 = weight*g3 + (1-weight)*g4 ;
					
					// 当前象素的梯度是局部的最大值
					// 该点可能是个边界点
					if(dTmp>=dTmp1 && dTmp>=dTmp2)
					{
						pUnchRst[nPos] = 128 ;
					}
					else
					{
						// 不可能是边界点
						pUnchRst[nPos] = 255 ;
					}
				}
			} //else
		} // for
	}
} 

/*************************************************************************
 *
 * \函数名称：
 *   Hysteresis()
 *
 * \输入参数:
 *   int *pnMag               - 梯度幅度图
 *	 int nWidth               - 图象数据宽度
 *	 int nHeight              - 图象数据高度
 *	 double dRatioLow         - 低阈值和高阈值之间的比例
 *	 double dRatioHigh        - 高阈值占图象象素总数的比例
 *   unsigned char *pUnchEdge - 记录边界点的缓冲区
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   本函数实现类似“磁滞现象”的一个功能，也就是，先调用EstimateThreshold
 *   函数对经过non-maximum处理后的数据pUnchSpr估计一个高阈值，然后判断
 *   pUnchSpr中可能的边界象素(=128)的梯度是不是大于高阈值nThdHigh，如果比
 *   该阈值大，该点将作为一个边界的起点，调用TraceEdge函数，把对应该边界
 *   的所有象素找出来。最后，当整个搜索完毕时，如果还有象素没有被标志成
 *   边界点，那么就一定不是边界点。
 *   
 *************************************************************************
 */
 void CDibObject::Hysteresis(int *pnMag, int nWidth, int nHeight, double dRatioLow, 
								double dRatioHigh, unsigned char *pUnchEdge)
{
	// 循环控制变量
	int y;
	int x;

	int nThdHigh ;
	int nThdLow  ;

	int nPos;

	// 估计TraceEdge需要的低阈值，以及Hysteresis函数使用的高阈值
	EstimateThreshold(pnMag, nWidth, nHeight, &nThdHigh, 
		               &nThdLow, pUnchEdge,dRatioHigh, dRatioLow);

  // 这个循环用来寻找大于nThdHigh的点，这些点被用来当作边界点，然后用
	// TraceEdge函数来跟踪该点对应的边界
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			nPos = y*nWidth + x ; 
			
			// 如果该象素是可能的边界点，并且梯度大于高阈值，该象素作为
			// 一个边界的起点
			if((pUnchEdge[nPos] == 128) && (pnMag[nPos] >= nThdHigh))
			{
				// 设置该点为边界点
				pUnchEdge[nPos] = 0;
				TraceEdge(x, y, nThdLow, pUnchEdge, pnMag, nWidth);
			}
		}
	}

	 // 那些还没有被设置为边界点的象素已经不可能成为边界点
   for(y=0; y<nHeight; y++)
	 {
		 for(x=0; x<nWidth; x++)
		 {
			 nPos = y*nWidth + x ; 
			 if(pUnchEdge[nPos] != 0)
			 {
				 // 设置为非边界点
				 pUnchEdge[nPos] = 255 ;
			 }
		 }
	 }
}

 /*************************************************************************
 *
 * \函数名称：
 *   EstimateThreshold()
 *
 * \输入参数:
 *   int *pnMag               - 梯度幅度图
 *	 int nWidth               - 图象数据宽度
 *	 int nHeight              - 图象数据高度
 *   int *pnThdHigh           - 高阈值
 *   int *pnThdLow            - 低阈值
 *	 double dRatioLow         - 低阈值和高阈值之间的比例
 *	 double dRatioHigh        - 高阈值占图象象素总数的比例
 *   unsigned char *pUnchEdge - 经过non-maximum处理后的数据
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   经过non-maximum处理后的数据pUnchEdge，统计pnMag的直方图，确定阈值。
 *   本函数中只是统计pUnchEdge中可能为边界点的那些象素。然后利用直方图，
 *   根据dRatioHigh设置高阈值，存储到pnThdHigh。利用dRationLow和高阈值，
 *   设置低阈值，存储到*pnThdLow。dRatioHigh是一种比例：表明梯度小于
 *   *pnThdHigh的象素数目占象素总数目的比例。dRationLow表明*pnThdHigh
 *   和*pnThdLow的比例，这个比例在canny算法的原文里，作者给出了一个区间。
 *
 *************************************************************************
 */
 void CDibObject::EstimateThreshold(int *pnMag, int nWidth, int nHeight, int *pnThdHigh,int *pnThdLow, 
											 unsigned char * pUnchEdge, double dRatioHigh, double dRationLow) 
{ 
	// 循环控制变量
	int y;
	int x;
	int k;
	
	// 该数组的大小和梯度值的范围有关，如果采用本程序的算法，那么梯度的范围不会超过362(255*sqrt(2))
	int nHist[362] ;

	// 可能的边界数目
	int nEdgeNb     ;

	// 最大梯度值
	int nMaxMag     ;

	int nHighCount  ;

	nMaxMag = 0     ; 
	
	// 初始化
	for(k=0; k<362; k++) 
	{
		nHist[k] = 0; 
	}

	// 统计直方图，然后利用直方图计算阈值
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			// 只是统计那些可能是边界点，并且还没有处理过的象素
			if(pUnchEdge[y*nWidth+x]==128)
			{
				nHist[ pnMag[y*nWidth+x] ]++;
			}
		}
	}

	nEdgeNb = nHist[0]  ;
	nMaxMag = 0         ;
	// 统计经过“非最大值抑止(non-maximum suppression)”后有多少象素
	for(k=1; k<362; k++)
	{
		if(nHist[k] != 0)
		{
			// 最大梯度值
			nMaxMag = k;
		}
		
		// 梯度为0的点是不可能为边界点的
		// 经过non-maximum suppression后有多少象素
		nEdgeNb += nHist[k];
	}

	// 梯度比高阈值*pnThdHigh小的象素点总数目
	nHighCount = (int)(dRatioHigh * nEdgeNb +0.5);
	
	k = 1;
	nEdgeNb = nHist[1];
	
	// 计算高阈值
	while( (k<(nMaxMag-1)) && (nEdgeNb < nHighCount) )
	{
		k++;
		nEdgeNb += nHist[k];
	}

	// 设置高阈值
	*pnThdHigh = k ;

	// 设置低阈值
	*pnThdLow  = (int)((*pnThdHigh) * dRationLow+ 0.5);
}

 /*************************************************************************
 *
 * \函数名称：
 *   TraceEdge()
 *
 * \输入参数:
 *   int    x									- 跟踪起点的x坐标 
 *   int    y									- 跟踪起点的y坐标
 *   int nLowThd							- 判断一个点是否为边界点的低阈值
 *   unsigned char *pUnchEdge - 记录边界点的缓冲区
 *   int *pnMag               - 梯度幅度图
 *   int nWidth               - 图象数据宽度
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   递归调用  
 *   从(x,y)坐标出发，进行边界点的跟踪，跟踪只考虑pUnchEdge中没有处理并且
 *   可能是边界点的那些象素(=128),象素值为0表明该点不可能是边界点，象素值
 *   为255表明该点已经被设置为边界点，不必再考虑
 *   
 *   
 *************************************************************************
 */
void CDibObject::TraceEdge (int x, int y, int nLowThd, unsigned char *pUnchEdge, int *pnMag, int nWidth) 
{ 
	// 对8邻域象素进行查询
	static int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1} ;
	static int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1} ;

	CArray<CPoint,CPoint> pointArray;  // 记录边界点堆栈
	CPoint point;  // 点变量
	int xx, yy;  // 坐标变量	
	int k;  // 循环变量
	
	pointArray.Add(CPoint(x,y));

	while(pointArray.GetSize())
	{
		point = pointArray.GetAt(0);
		pointArray.RemoveAt(0);

		for(k=0; k<8; k++)
		{
			xx = point.x + xNb[k];
			yy = point.y + yNb[k];
			// 如果该象素为可能的边界点，又没有处理过
			// 并且梯度大于阈值
			if(pUnchEdge[yy*nWidth+xx] == 128  && pnMag[yy*nWidth+xx]>=nLowThd)
			{
				// 把该点设置成为边界点
				pUnchEdge[yy*nWidth+xx] = 0 ;
				pointArray.Add(CPoint(xx,yy));
			}
		}
	}
}

// 获取道路边界记录于参数roadLineArray中
void CDibObject::GetRoadEdgeLine(unsigned char *pUnchEdge, int nWidth, int nHeight,
		CArray<CLine*,CLine*> &roadEdgeLineArray,unsigned char *pUnchSmooth,int *pnGradMag)
{
	// 对8邻域象素进行查询
	static int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1};
	static int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1};
	int k;  // 循环变量
	int x, y, xx, yy;  // 坐标变量
	int nPos;  // 一般变量
	CArray<CPoint,CPoint> pointArray;  // 点数组
	BOOL finish;  // 搜寻是否结束
	CPoint point;  // 点变量

	// 清除数组roadEdgeLineArray中数据
	ClearLineArray(roadEdgeLineArray);
	CLine::groupID=0;

	for(x=0; x<nWidth; x++)
	{
		for(y=0; y<nHeight; y++)
		{
			nPos = x + y*nWidth;
			if(pUnchEdge[nPos] == 0)
			{
				pUnchEdge[nPos] = 255;
				finish = FALSE;
				pointArray.RemoveAll();
				pointArray.Add(CPoint(x,y));
				while(!finish)
				{
					finish = TRUE;
					point = pointArray.GetAt(0);
					for(k=0; k<8; k++)
					{
						xx = point.x + xNb[k];
						yy = point.y + yNb[k];
						nPos = xx + yy*nWidth;
						if(pUnchEdge[nPos] == 0)
						{
							pUnchEdge[nPos] = 255;
							finish = FALSE;
							pointArray.InsertAt(0,CPoint(xx,yy));
							break;
						}
					}
				}
				// 检查数组pointArray中存储的线中是否有道路边界线，如果有截取后将其加入roadEdgeLineArray中
				ExtractRoadEdgeLine(pointArray, roadEdgeLineArray, pUnchSmooth, pnGradMag, nWidth, nHeight);
			}
		}
	}
}

// 初始化分组数组
void CDibObject::InitGroups(CGroup *&pGroups, int &groupNum, CArray<CLine*,CLine*> &lineArray)
{
	int lineNum;  // 线段数量
	int i;  // 循环变量
	CLine *pLine;

	lineNum = lineArray.GetSize();
	pGroups = new CGroup[lineNum];
	groupNum = lineNum;
	for(i=0; i<lineNum; i++)
	{
		pLine = lineArray.GetAt(i);
		pGroups[i].m_groupID = pLine->m_groupID;
		pGroups[i].m_lineLength = pLine->m_lineLength;
		pGroups[i].m_parallels = pLine->m_parallels;
		pGroups[i].m_edgeLines = pLine->m_edgeLines;
	}
}

// 检查数组pointArray中存储的线中是否有道路边界线，如果有截取后将其加入roadEdgeLineArray中
void CDibObject::ExtractRoadEdgeLine(CArray<CPoint,CPoint> &pointArray,CArray<CLine*,CLine*> &roadEdgeLineArray,
									 unsigned char *pUnchSmooth,int *pnGradMag,int nWidth,int nHeight)
{
	static int lowPointNum = 30;  // 线段应该包含的点的最小数目
	static int startPointNum = 7;  // 线段两端平滑验证开始点的位置，应与成员函数GetRoadSeedPoint
	                               // 中的一致
	static double lowDirectionCha=155;  // 线段前后端方向差最大值
	int nPointNum;  // 点的数量
	CArray<CPoint,CPoint> subPointArray;  // 截取的比较平滑的线段
	int i,k;  // 循环变量
	CPoint startPoint,point,endPoint;  // 点变量
	double dirCha;  // 两矢量方向差

	// 去除所含点数小于lowPointNum的线段
	nPointNum = pointArray.GetSize();
	if(nPointNum < lowPointNum)
	{
		return;
	}

	// 在线段pointArray中截取一定长度且比较平滑的线段
	for(i=0; i<startPointNum; i++)
	{
		subPointArray.Add(pointArray.GetAt(i));
	}

	nPointNum -= startPointNum;
	for(i=startPointNum; i<nPointNum; i++)
	{
		startPoint = pointArray.GetAt(i - startPointNum);
		point = pointArray.GetAt(i);
		endPoint = pointArray.GetAt(i + startPointNum);
		dirCha = GetDirCha(startPoint.x - point.x, startPoint.y - point.y,
			endPoint.x - point.x, endPoint.y - point.y)*180/PI;
		
		// 如果point点处线段比较平滑，则将该点加入线段subPointArray
		if(dirCha > lowDirectionCha)
		{
			subPointArray.Add(point);
		}
		// 如果point点处线段不够平滑，则检查已截取的线段subPointArray是否是道路边界并开始新的截取
		else
		{
			CheckRoadEdgeLine(subPointArray, roadEdgeLineArray, pUnchSmooth, pnGradMag, nWidth, nHeight);
			// 重新设置线段subPointArray
			subPointArray.RemoveAll();
			for(k=startPointNum-1;k>=0;k--)
			{
				subPointArray.Add(pointArray.GetAt(i-k));
			}
		}
	}

	nPointNum += startPointNum;
	for(;i<nPointNum;i++)
	{
		subPointArray.Add(pointArray.GetAt(i));
	}
	CheckRoadEdgeLine(subPointArray, roadEdgeLineArray, pUnchSmooth, pnGradMag, nWidth, nHeight);
}

// 计算两矢量夹角，lx1和ly1为矢量1，lx2和ly2为矢量2
double CDibObject::GetDirCha(double lx1,double ly1,double lx2,double ly2)
{
	double length1,length2;      // 矢量长度
	double cosResult;            // 余弦值

	length1=sqrt(lx1*lx1+ly1*ly1);
	length2=sqrt(lx2*lx2+ly2*ly2);
	cosResult=(lx1*lx2+ly1*ly2)/(length1*length2);
	if(cosResult<-1)
		cosResult=-1;
	else if(cosResult>1)
		cosResult=1;
	return acos(cosResult);
}

// 检查已截取的线段subPointArray是否是道路边界，如果是则将其加入数组roadEdgeLineArray中
void CDibObject::CheckRoadEdgeLine(CArray<CPoint,CPoint> &pointArray,CArray<CLine*,CLine*> &roadEdgeLineArray,
								   unsigned char *pUnchSmooth,int *pnGradMag,int nWidth,int nHeight)
{
	static int lowPointNum=30;  // 线段应该包含的点的最小数目
	static double lowLineLength=30;  // 线段最小长度
	int pointNum;  // 线段包含的点的数目
	int i;  // 循环变量
	CPoint point1,point2;  // 中间变量
	double lx,ly;  // 中间变量
	double lineLength=0;  // 记录线段长度
	int nPos;  // 中间变量
	double aveHuiDu=0;  // 记录线段平均灰度
	double aveGradMag=0;  // 记录线段平均梯度
	int edgeLines=0;  // 边界点数量
	CLine *pLine;  // 线段指针

	// 去除所含点数小于lowPointNum的线段
	pointNum=pointArray.GetSize();
	if(pointNum<lowPointNum )
		return;

	point1=pointArray.GetAt(0);
	if(IsEdgePoint(point1,nWidth,nHeight))
		edgeLines++;

	// 取得线段的长度、平均灰度和梯度
	pointNum-=2;
	for(i=0;i<pointNum;i++)
	{
		point1=pointArray.GetAt(i);
		point2=pointArray.GetAt(i+2);
		lx=point2.x-point1.x;
		ly=point2.y-point1.y;
		lineLength+=sqrt(lx*lx+ly*ly);
		nPos=point1.x+point1.y*nWidth;
		aveHuiDu+=pUnchSmooth[nPos];
		aveGradMag+=pnGradMag[nPos];
	}
	point1=pointArray.GetAt(pointNum++);
	nPos=point1.x+point1.y*nWidth;
	aveHuiDu+=pUnchSmooth[nPos];
	aveGradMag+=pnGradMag[nPos];

	point1=pointArray.GetAt(pointNum++);
	nPos=point1.x+point1.y*nWidth;
	aveHuiDu+=pUnchSmooth[nPos];
	aveGradMag+=pnGradMag[nPos];

	if(IsEdgePoint(point1,nWidth,nHeight))
		edgeLines++;

	aveHuiDu/=pointNum;
	aveGradMag/=pointNum;
	lineLength/=2;

	// 去除长度小于lowLineLength的线段
	if(lineLength<lowLineLength)
		return;

	// 将该线段记录入数组roadEdgeLineArray中
	pLine=new CLine;
	for(i=0;i<pointNum;i++)
	{
		pLine->m_pointArray.Add(pointArray.GetAt(i));
	}
	pLine->m_lineLength=lineLength;
	pLine->m_aveHuiDu=aveHuiDu;
	pLine->m_aveGradMag=aveGradMag;
	pLine->m_edgeLines=edgeLines;
	roadEdgeLineArray.Add(pLine);
}

// 检查指定点是否是边界点
BOOL CDibObject::IsEdgePoint(CPoint point1,int nWidth,int nHeight)
{
	static int highBianJu=11;  // 距离边界最大距离
	int x,y;                   // 被检查点的x,y坐标
	BOOL isEdgePoint=FALSE;
	x=point1.x;
	y=point1.y;
	if(x<highBianJu || (nWidth-1-x)<highBianJu)
		isEdgePoint=TRUE;
	if(y<highBianJu || (nHeight-1-y)<highBianJu)
		isEdgePoint=TRUE;
	return isEdgePoint;
}

// 层次记号编组
void CDibObject::CengCiBianZu(CArray<CLine*,CLine*> &roadEdgeLineArray, CGroup *pGroups, int groupNum)
{
	int lineNum;  // 线段数量
	BOOL finish = FALSE;  // 是否完成
	int i, j, k;  // 循环变量
	CLine *pLine1, *pLine2;  // 线段指针
	BOOL parallel;  // 是否平行
	int groupID;  // 线段编组号
	
	lineNum = roadEdgeLineArray.GetSize();
	while(!finish)
	{
		finish = TRUE;
		for(i=0; i<lineNum; i++)
		{
			pLine1 = roadEdgeLineArray.GetAt(i);
			for(j=0; j<lineNum; j++)
			{
				pLine2 = roadEdgeLineArray.GetAt(j);
				// 如果编组成功则合并相应组
				if(pLine1->m_groupID != pLine2->m_groupID && CanBianZu(pLine1, pLine2, parallel))
				{
					// 将所有编组号为pLine2->m_groupID的线段的编组号改为pLine1->m_groupID
					groupID = pLine2->m_groupID;
					for(k=0; k<lineNum; k++)
					{
						pLine2 = roadEdgeLineArray.GetAt(k);
						if(pLine2->m_groupID == groupID)
						{
							pLine2->m_groupID = pLine1->m_groupID;
						}
					}
					// 合并线段组
					HeBingGroup(pGroups, groupNum, pLine1->m_groupID, groupID, parallel);
					finish = FALSE;
				}
			}
		}
	}
}

// 检查两线段是否可以编为一组
BOOL CDibObject::CanBianZu(CLine *pLine1, CLine *pLine2, BOOL &parallel)
{
	static int fenGePos = 10;  // 前端、中端、后端分类标准，如前fenGePos点为前端点
	static double fanLowDirCha = 165;  // 方向相反时认为共线最小方向差
	static double highDirCha = 15;  // 方向相同时认为共线最大方向差
	static double highJuLi = 15;  // 延长方向上最大距离
	CPoint startPoint, endPoint, point1, point2;  // 点变量
	int pos;  // 位置变量
	double dirCha;  // 两线段方向差
	int pointNum1, pointNum2; // 点数量

	parallel = FALSE;
	
	startPoint = pLine1->m_pointArray.GetAt(0);
	// 如果线段pLine1前端点与线段pLine2相邻
	if(XiangLin(startPoint, pLine2, 6, pos))
	{
		endPoint = pLine1->m_pointArray.GetAt(fenGePos);
		// 线段pLine1的前端点与线段pLine2的前端点相邻
		if(pos < fenGePos)
		{
			point1 = pLine2->m_pointArray.GetAt(0);
			point2 = pLine2->m_pointArray.GetAt(fenGePos);
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point1.x - point2.x, point1.y - point2.y) * 180/PI;
		
			// 线段pLine1的前端与线段pLine2的前端方向相同
			if(dirCha < highDirCha)
			{
				pointNum1 = pLine1->m_pointArray.GetSize();
				pointNum2 = pLine2->m_pointArray.GetSize() - pos;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >= fenGePos? pointNum2 + pos -1: pointNum2 + pos - fenGePos;
					point2 = pLine2->m_pointArray.GetAt(pos);

					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point2, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? pointNum1 - 1: pointNum1 - fenGePos;
					endPoint = pLine1->m_pointArray.GetAt(pos);

					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(endPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}

			// 线段pLine1的前端与线段pLine2的前端方向相反，则认为共线
			else if(dirCha > fanLowDirCha)
			{
				return TRUE;
			}
		}

		// 线段pLine1的前端点与线段pLine2的后端点相邻
		else if(pos > pLine2->m_pointArray.GetSize() - fenGePos -1)
		{
			pointNum2 = pLine2->m_pointArray.GetSize();
			point1 = pLine2->m_pointArray.GetAt(pointNum2 - 1 - fenGePos);
			point2 = pLine2->m_pointArray.GetAt(pointNum2 - 1);
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point1.x - point2.x, point1.y - point2.y) * 180/PI;

			// 线段pLine1的前端点与线段pLine2的后端点方向相同，则认为共线
			if(dirCha < highDirCha)
			{
				return TRUE;
			}
			// 线段pLine1的前端点与线段pLine2的后端点方向相反
			else if(dirCha > fanLowDirCha)
			{
				pointNum1 = pLine1->m_pointArray.GetSize();
				pointNum2 = pos + 1;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >=fenGePos? 0: fenGePos - 1;
					point1 = pLine2->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point1, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? pointNum1 - 1: pointNum1 - fenGePos;
					endPoint = pLine1->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(endPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}
		}
		// 线段pLine1的前端点与线段pLine2的中端点相邻
		else
		{
			point1 = pLine2->m_pointArray.GetAt(pos - fenGePos);
			point2 = pLine2->m_pointArray.GetAt(pos + fenGePos);
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point1.x - point2.x, point1.y - point2.y) * 180/PI;
			
			// 线段pLine1的前端点与线段pLine2的中端点方向相同
			if(dirCha < highDirCha)
			{
				pointNum1 = pLine1->m_pointArray.GetSize();
				pointNum2 = pLine2->m_pointArray.GetSize() - pos;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >= fenGePos? pointNum2 + pos -1: pointNum2 + pos - fenGePos;
					point2 = pLine2->m_pointArray.GetAt(pos);

					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point2, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? pointNum1 - 1: pointNum1 - fenGePos;
					endPoint = pLine1->m_pointArray.GetAt(pos);

					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(endPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}
			// 线段pLine1的前端点与线段pLine2的中端点方向相反
			else if(dirCha > fanLowDirCha)
			{
				pointNum1 = pLine1->m_pointArray.GetSize();
				pointNum2 = pos + 1;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >=fenGePos? 0: fenGePos - 1;
					point1 = pLine2->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point1, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? pointNum1 - 1: pointNum1 - fenGePos;
					endPoint = pLine1->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(endPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}
		}
	}

	pointNum1 = pLine1->m_pointArray.GetSize();
	endPoint = pLine1->m_pointArray.GetAt(pointNum1 - 1);
	// 如果线段pLine1的后端点与线段pLine2相邻
	if(XiangLin(endPoint, pLine2, 6, pos))
	{
		startPoint = pLine1->m_pointArray.GetAt(pointNum1 - 1 - fenGePos);
		pointNum2 = pLine2->m_pointArray.GetSize();
		// 线段pLine1的后端点与线段pLine2的后端点相邻 
		if(pos > pointNum2 - fenGePos -1)
		{
			point1 = pLine2->m_pointArray.GetAt(pointNum2 - 1 - fenGePos);
			point2 = pLine2->m_pointArray.GetAt(pointNum2 - 1);
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point1.x - point2.x, point1.y - point2.y);

			// 线段pLine1的后端点与线段pLine2的后端点方向相同
			if(dirCha < highDirCha)
			{
				pointNum2 = pos + 1;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >= fenGePos? 0: fenGePos - 1;
					point1 = pLine2->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point1, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}

				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? 0: fenGePos - 1;
					startPoint = pLine1->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(startPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}

			// 线段pLine1的后端点与线段pLine2的后端点方向相反，则认为两线段共线
			else if(dirCha > fanLowDirCha)
			{
				return TRUE;
			}
		}

		// 如果线段pLine1后端点与线段pLine2中端点相邻
		else if(pos > fenGePos - 1)
		{
			point1 = pLine2->m_pointArray.GetAt(pos - fenGePos);
			point2 = pLine2->m_pointArray.GetAt(pos + fenGePos);
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point1.x - point2.x, point1.y - point2.y);
			// 线段pLine1的后端点与线段pLine2的中端点方向相同
			if(dirCha < highDirCha)
			{
				pointNum2 = pos + 1;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >= fenGePos? 0: fenGePos - 1;
					point1 = pLine2->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point1, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}

				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? 0: fenGePos - 1;
					startPoint = pLine1->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(startPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}

			// 线段pLine1的后端点与线段pLine2的中端点方向相反
			else if(dirCha > fanLowDirCha)
			{
				pointNum2 = pointNum2 - pos;
				// 线段pLine2在同方向上剩余的点较少
				if(pointNum1 > pointNum2)
				{
					pos = pointNum1 - pointNum2 >= fenGePos? pointNum2 + pos -1: pointNum2 + pos -fenGePos;
					point2 = pLine2->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(point2, pLine1, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}

				// 线段pLine1在同方向上剩余的点较少
				else
				{
					pos = pointNum2 - pointNum1 >= fenGePos? 0: pos - 1;
					startPoint = pLine1->m_pointArray.GetAt(pos);
					// 较短线段的另一端点亦与较长线段相临，则认为两线段平行
					if(XiangLin(startPoint, pLine2, 6, pos))
					{
						parallel = TRUE;
						return TRUE;
					}
				}
			}
		}
	}

	startPoint = pLine1->m_pointArray.GetAt(0);
	point1 = pLine2->m_pointArray.GetAt(0);
	// 两线段前端点在延长范围内
	if(GetJuLi(startPoint, point1) <= highJuLi)
	{
		endPoint = pLine1->m_pointArray.GetAt(fenGePos);
		point2 = pLine2->m_pointArray.GetAt(fenGePos);
		dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
			point1.x - point2.x, point1.y - point2.y);
		// 两线段在前端点处方向相反
		if(dirCha > fanLowDirCha)
		{
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point1.x - startPoint.x, point1.y - startPoint.y);
			// 前端点连线方向与线段pLine1前端方向一致
			if(dirCha < highDirCha)
			{
				dirCha = GetDirCha(point1.x - point2.x, point1.y - point2.y,
					point1.x - startPoint.x, point1.y - startPoint.y);	
				// 前端点连线方向与线段pLine2前端方向亦一致，则认为共线
				if(dirCha > fanLowDirCha)
				{
					return TRUE;
				}
			}
		}
	}

	pointNum2 = pLine2->m_pointArray.GetSize();
	point2 = pLine2->m_pointArray.GetAt(pointNum2 - 1);
	// 线段pLine1前端点与线段pLine2后端点在延长范围内
	if(GetJuLi(startPoint, point2) <= highJuLi)
	{
		endPoint = pLine1->m_pointArray.GetAt(fenGePos);
		point1 = pLine2->m_pointArray.GetAt(pointNum2 - 1 - fenGePos);
		dirCha = GetDirCha(startPoint.x - endPoint.x,startPoint.y - endPoint.y,
			point1.x - point2.x, point1.y - point2.y);
		// 两线段前后端方向相同
		if(dirCha < highDirCha)
		{
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point2.x - startPoint.x, point2.y - startPoint.y);
			// 前后端点连线方向与线段pLine1前端方向一致
			if(dirCha < highDirCha)
			{
				dirCha = GetDirCha(point1.x - point2.x, point1.y - point2.y,
					point2.x - startPoint.x, point2.y - startPoint.y);
				// 前后端点连线方向与线段pLine2后端方向一致，则认为共线
				if(dirCha < highDirCha)
				{
					return TRUE;
				}
			}
		}
	}

	pointNum1 = pLine1->m_pointArray.GetSize();
	endPoint = pLine1->m_pointArray.GetAt(pointNum1 - 1);
	// 两线段后端点在连线范围内
	if(GetJuLi(endPoint, point2) <= highJuLi)
	{
		startPoint = pLine1->m_pointArray.GetAt(pointNum1 - 1 - fenGePos);
		point1 = pLine2->m_pointArray.GetAt(pointNum2 - 1 - fenGePos);
		dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
			point1.x - point2.x, point1.y - point2.y);
		// 两线段后端点方向相反
		if(dirCha > fanLowDirCha)
		{
			dirCha = GetDirCha(startPoint.x - endPoint.x, startPoint.y - endPoint.y,
				point2.x - endPoint.x, point2.y - endPoint.y);
			// 后端点连线方向与线段pLine1后端点方向一致
			if(dirCha > fanLowDirCha)
			{
				dirCha = GetDirCha(point1.x - point2.x, point1.y - point2.y,
					point2.x - endPoint.x, point2.y - endPoint.y);
				// 后端点连线方向与线段pLine2后端点方向一致
				if(dirCha < highDirCha)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

// 检查指定点是否与指定线段相邻，并通过参数pos返回线段上的相邻点的索引
BOOL CDibObject::XiangLin(CPoint point, CLine *pLine, double highJuLi, int &pos)
{
	int pointNum;  // 点数量
	int i;  // 循环变量
	CPoint point1;  // 点变量

	pointNum = pLine->m_pointArray.GetSize();
	for(i=0; i<pointNum; i++)
	{
		point1 = pLine->m_pointArray.GetAt(i);
		if(GetJuLi(point, point1) <= highJuLi)
		{
			pos = i;
			return TRUE;
		}
	}

	return FALSE;
}

// 得到两点之间的距离
double CDibObject::GetJuLi(CPoint point1,CPoint point2)
{
	double lx,ly;       // 方向矢量

	lx=point1.x-point2.x;
	ly=point1.y-point2.y;
	return sqrt(lx*lx+ly*ly);
}

// 合并线段组
void CDibObject::HeBingGroup(CGroup *pGroups, int groupNum, int group1ID, int group2ID, BOOL parallel)
{
	int i;  // 循环变量
	int group1Index, group2Index;  // 组索引

	for(i=0; i<groupNum; i++)
	{
		if(pGroups[i].m_groupID == group1ID)
		{
			group1Index = i;
		}
		if(pGroups[i].m_groupID == group2ID)
		{
			group2Index = i;
		}
	}

	pGroups[group2Index].m_groupID = -1;
	pGroups[group1Index].m_lineLength += pGroups[group2Index].m_lineLength;
	pGroups[group1Index].m_parallels += pGroups[group2Index].m_parallels;
	if(parallel)
	{
		pGroups[group1Index].m_parallels++;
	}
	pGroups[group1Index].m_edgeLines += pGroups[group2Index].m_edgeLines;
}

// 计算置信度
void CDibObject::CalBeliefDegree(CGroup *pGroups, int groupNum)
{
	int i;  // 循环变量

	for(i=0; i<groupNum; i++)
	{
		if(pGroups[i].m_groupID >= 0)
		{
			pGroups[i].CalBeliefDegree();
		}
	}
}

// 获取道路种子点
BOOL CDibObject::GetRoadEdgePoint(CGroup *pGroups, int groupNum, CArray<CLine*,CLine*> &roadEdgeLineArray,
								  int *pnGradMag,unsigned char *pUnchSmooth, int width, int height)
{
	static int startPointNum = 7; // 线段两端梯度最大值搜寻点开始的位置，应与
	                              // 成员函数ExtractRoadEdgeLine中的一致
	static int seedPointJu = 10;  // 两种子点相距的最小距离
	static int N = 7;             // N领域范围半径，与TrackRoadByGroup中一致
	static int M = 3 * N;         // M领域范围半径
	int lineNum;  // 线段数量
	int i, j;  // 循环变量
	int indexOfMaxValue;  // 最大值在数组中的索引
	double maxLineLen;  // 最大线段长度
	double maxGradMag;  // 最大灰度值
	CLine *pLine;  // 线段指针
	int pointNum;  // 点数量
	CPoint point1, point2;  // 点变量
	// 线段两端点M、N领域内的平均灰度
	double point1AveGrayN, point1AveGrayM, point2AveGrayN, point2AveGrayM;
	
	lineNum = roadEdgeLineArray.GetSize();
	for(i=0; i<groupNum; i++)
	{
		if(pGroups[i].m_believed)
		{
			// 找出该组中长度最大的线段
			maxLineLen = 0;
			for(j=0; j<lineNum; j++)
			{
				pLine = roadEdgeLineArray.GetAt(j);
				if(pLine->m_groupID == pGroups[i].m_groupID && pLine->m_lineLength > maxLineLen)
				{
					indexOfMaxValue = j;
					maxLineLen = pLine->m_lineLength;
				}
			}

			// 找出长度最大线段中梯度最大的点
			pLine = roadEdgeLineArray.GetAt(indexOfMaxValue);
			pointNum = pLine->m_pointArray.GetSize() - startPointNum;
			maxGradMag = 0;
			for(j=startPointNum; j<pointNum; j++)
			{
				point1 = pLine->m_pointArray.GetAt(j);
				if(pnGradMag[point1.x + point1.y*width] > maxGradMag)
				{
					indexOfMaxValue = j;
					maxGradMag = pnGradMag[point1.x + point1.y*width];
				}
			}

			// 找出梯度最大点两侧的道路种子点
			j = indexOfMaxValue - seedPointJu/2;
			if(j < 0)
				j = 0;
			pointNum += startPointNum - seedPointJu;
			for(; j<pointNum; j++)
			{
				point1 = pLine->m_pointArray.GetAt(j);
				point2 = pLine->m_pointArray.GetAt(j + seedPointJu);

				// 计算领域内平均灰度
				point1AveGrayN=GetAveGray(point1, pUnchSmooth, width, height, N);
				point1AveGrayM=GetAveGray(point1, pUnchSmooth, width, height, M);
				point2AveGrayN=GetAveGray(point2, pUnchSmooth, width, height, N);
				point2AveGrayM=GetAveGray(point2, pUnchSmooth, width, height, M);
				
				// 找到道路种子点
				if(point1AveGrayM > point1AveGrayN && point2AveGrayM > point2AveGrayN ||
					point1AveGrayM < point1AveGrayN && point2AveGrayM < point2AveGrayN)
				{
					pGroups[i].m_seedPoint1 = point1;
					pGroups[i].m_seedPoint2 = point2;
					break;
				}
			}
		}
	}
	return TRUE;
}

// 计算指定点指定领域内的灰度
double CDibObject::GetAveGray(CPoint point, unsigned char *pUnchSmooth,
							  int width, int height, int length)
{
	int x, y, X, Y;  // 循环变量，标记坐标
	int pointNum = 0;  // 符合要求的点的个数
	double aveGray = 0;

	X=point.x+length+1;
	Y=point.y+length+1;

	// 计算所有符合要求的点的总灰度
	for(x=point.x-length; x<X; x++)
	{
		for(y=point.y-length; y<Y; y++)
		{
			if(x>-1 && x<width && y>-1 && y<height && GetJuLi(CPoint(x,y),point) <= length)
			{
				aveGray += pUnchSmooth[x + y*width];
				pointNum++;
			}
		}
	}

	return aveGray /= pointNum;
}

// 清除数组
void CDibObject::ClearLineArray(CArray<CLine*, CLine*> &array)
{
	int arraySize;  // 数组大小
	int i;  // 循环变量

	if(array.GetSize() > 0)
	{
		arraySize = array.GetSize();
		for(i=0; i<arraySize; i++)
		{
			delete (CLine *)array.GetAt(i);
		}

		array.RemoveAll();
	}
}

// 根据道路种子点跟踪道路
void CDibObject::TrackRoadByGroup(CGroup* pGroups, int groupNum, unsigned char *pUnchSmooth,
							int width, int height, CArray<CLine*, CLine*> &roadArray)
{
	int i, j;  // 循环变量
	CPoint edgePoint1, edgePoint2;  // 点变量
	static int N = 7;               // N领域范围半径，与GetRoadEdgePoint中一致
	static int M = 3 * N;           // M领域范围半径
	// 线段两端点M、N领域内的平均灰度
	double point1AveGrayN, point1AveGrayM, point2AveGrayN, point2AveGrayM;
	CArray<CPoint, CPoint> roadPointArray;  // 记录获得的道路点
	CLine *pLine;  // 线段指针变量
	int pointNum;  // 线段点的个数

	// 清除roadArray数组
	ClearLineArray(roadArray);

	for(i=0; i<groupNum; i++)
	{
		if(pGroups[i].m_believed)
		{
			edgePoint1 = pGroups[i].m_seedPoint1;
			edgePoint2 = pGroups[i].m_seedPoint2;

			// 计算领域内平均灰度
			point1AveGrayN=GetAveGray(edgePoint1, pUnchSmooth, width, height, N);
			point1AveGrayM=GetAveGray(edgePoint1, pUnchSmooth, width, height, M);
			point2AveGrayN=GetAveGray(edgePoint2, pUnchSmooth, width, height, N);
			point2AveGrayM=GetAveGray(edgePoint2, pUnchSmooth, width, height, M);

			if(point1AveGrayN > point1AveGrayM && point2AveGrayN > point2AveGrayM)
			{
				TrackGrayRoadByEdgePoint(edgePoint1, edgePoint2, pUnchSmooth, width, height, 48, roadPointArray);
			}
			else
			{
				TrackWhiteRoadByEdgePoint(edgePoint1, edgePoint2, pUnchSmooth, width, height, 48, roadPointArray);
			}

			// 新建道路
			pLine = new CLine;
			pointNum = roadPointArray.GetSize();
			if(pointNum > 0)
			{
				for(j=0; j<pointNum; j++)
				{
					pLine->m_pointArray.Add(roadPointArray.GetAt(j));
				}
				roadArray.Add(pLine);
			}
		}
	}
}

// 根据两个点向两个方向延伸道路
void CDibObject::TrackGrayRoadByEdgePoint(CPoint point1, CPoint point2, unsigned char *pUnchSmooth, int width,
		int height, double highCost, CArray<CPoint, CPoint> &roadPointArray)
{
	CPoint grayestPoint1,grayerPoint1,grayestPoint2,grayerPoint2;  // 两个方向延伸的可能起始点 
	CArray<CPoint,CPoint> linePoints;    // 记录线段上的点
	int pointNum,halfPointNum;           // 线段所包含点数
	int i;                               // 循环变量
	BOOL isGrayestPoint1 = TRUE;           // 是否是从isGrayestPoint1点延伸
	CPoint point;                        // 点变量
	double lineLength = 0;                 // 记录线段长

	roadPointArray.RemoveAll();
	GetGrayestPoint(point1, point2, pUnchSmooth, width, height, grayestPoint1, grayerPoint1);
	GetGrayestPoint(point2, point1, pUnchSmooth, width, height, grayestPoint2, grayerPoint2);

	// 线段从grayestPoint2向grayestPoint1方向延伸
	TrackRoadBySeedPoint(grayestPoint2,grayestPoint1,grayerPoint1,pUnchSmooth,width,height,highCost,roadPointArray);

	// 从线段roadPointArray中去除grayestPoint2与grayestPoint1(或grayerPoint1)之间的点
	if(roadPointArray.GetSize() > 0)
	{
		// 判断是否是从点grayestPoint1延伸
		GetLinePoints(grayestPoint2,grayestPoint1,linePoints);
		pointNum=linePoints.GetSize();
		for(i=0;i<pointNum;i++)
		{
			if(((CPoint)linePoints.GetAt(i))!=((CPoint)roadPointArray.GetAt(i)))
			{
				isGrayestPoint1=FALSE;
				break;
			}
		}

		// 如果从grayestPoint1点延伸，则从roadPointArray中去除线段(grayestPoint2,grayestPoint1)上的点
		if(isGrayestPoint1)
		{
			roadPointArray.RemoveAt(0,pointNum);
		}

		// 从点grayerPoint1延伸
		else
		{
			GetLinePoints(grayestPoint2,grayerPoint1,linePoints);
			pointNum=linePoints.GetSize();
			roadPointArray.RemoveAt(0,pointNum);
			point=grayestPoint1;
			grayestPoint1=grayerPoint1;
			grayerPoint1=point;
		}

		// 将线段linePoints中的点反转
		pointNum=roadPointArray.GetSize();
		halfPointNum=pointNum/2;
		pointNum--;
		for(i=0;i<halfPointNum;i++)
		{
			point=roadPointArray.GetAt(i);
			roadPointArray.SetAt(i,roadPointArray.GetAt(pointNum-i));
			roadPointArray.SetAt(pointNum-i,point);
		}
	}

	// 线段从grayestPoint1向grayestPoint2方向延伸
	TrackRoadBySeedPoint(grayestPoint1,grayestPoint2,grayerPoint2,pUnchSmooth,width,height,highCost,roadPointArray);	
}

// 根据两个点向两个方向延伸道路
void CDibObject::TrackWhiteRoadByEdgePoint(CPoint point1, CPoint point2, unsigned char *pUnchSmooth, int width,
		int height, double highCost, CArray<CPoint, CPoint> &roadPointArray)
{
	CPoint whitestPoint1,whiterPoint1,whitestPoint2,whiterPoint2;  // 两个方向延伸的可能起始点 
	CArray<CPoint,CPoint> linePoints;    // 记录线段上的点
	int pointNum,halfPointNum;           // 线段所包含点数
	int i;                               // 循环变量
	BOOL isWhitestPoint1=TRUE;           // 是否是从isWhitestPoint1点延伸
	CPoint point;                        // 点变量
	double lineLength=0;                 // 记录线段长

	roadPointArray.RemoveAll();
	GetWhitestPoint(point1,point2,pUnchSmooth,width,height,whitestPoint1,whiterPoint1);
	GetWhitestPoint(point2,point1,pUnchSmooth,width,height,whitestPoint2,whiterPoint2);

	// 线段从whitestPoint2向whitestPoint1方向延伸
	TrackRoadBySeedPoint(whitestPoint2,whitestPoint1,whiterPoint1,pUnchSmooth,width,height,highCost,roadPointArray);

	// 从线段roadPointArray中去除whitestPoint2与whitestPoint1(或whiterPoint1)之间的点
	if(roadPointArray.GetSize()>0)
	{
		// 判断是否是从点whitestPoint1延伸
		GetLinePoints(whitestPoint2,whitestPoint1,linePoints);
		pointNum=linePoints.GetSize();
		for(i=0;i<pointNum;i++)
		{
			if(((CPoint)linePoints.GetAt(i))!=((CPoint)roadPointArray.GetAt(i)))
			{
				isWhitestPoint1=FALSE;
				break;
			}
		}

		// 如果从whitestPoint1点延伸，则从roadPointArray中去除线段(whitestPoint2,whitestPoint1)上的点
		if(isWhitestPoint1)
		{
			roadPointArray.RemoveAt(0,pointNum);
		}

		// 从点whiterPoint1延伸
		else
		{
			GetLinePoints(whitestPoint2,whiterPoint1,linePoints);
			pointNum=linePoints.GetSize();
			roadPointArray.RemoveAt(0,pointNum);
			point=whitestPoint1;
			whitestPoint1=whiterPoint1;
			whiterPoint1=point;
		}
		
		// 将线段linePoints中的点反转
		pointNum=roadPointArray.GetSize();
		halfPointNum=pointNum/2;
		pointNum--;
		for(i=0;i<pointNum;i++)
		{
			point=roadPointArray.GetAt(i);
			roadPointArray.SetAt(i,roadPointArray.GetAt(pointNum-i));
			roadPointArray.SetAt(pointNum-i,point);
		}
	}

	// 线段从whitestPoint1向whitestPoint2方向延伸
	TrackRoadBySeedPoint(whitestPoint1,whitestPoint2,whiterPoint2,pUnchSmooth,width,height,highCost,roadPointArray);
}

// 在靠近pointA点处得到灰度最大点、灰度次大点
void CDibObject::GetGrayestPoint(CPoint pointA,CPoint pointB,unsigned char *pUnchSmooth,int width,int height,CPoint &grayestPoint,CPoint &grayerPoint)
{
	static int extPointNum = 5;           // 左右扩展点个数
	static int len = 2;                   // 计算灰度时指定领域的范围半径
	CArray<CPoint,CPoint> grayPoints;     // 参与比较灰度的各个点
	double grayest;                       // 最大灰度值
	double gray;                          // 灰度值
	int pointNum;                         // grayPoints中点的数量
	int i;                                // 循环变量
	CPoint point;                         // 变量

	GetParePoints(pointA, pointB, extPointNum, width, height, grayPoints);
	grayestPoint = grayerPoint = grayPoints.GetAt(0);
	grayest=GetAveGray(grayestPoint, pUnchSmooth, width, height, len);
	pointNum=grayPoints.GetSize();
	for(i=1;i<pointNum;i++)
	{
		point=grayPoints.GetAt(i);
		gray=GetAveGray(point , pUnchSmooth, width, height, len);
		if(grayest<gray)
		{
			grayerPoint=grayestPoint;
			grayestPoint=point;
			grayest=gray;
		}
	}
}

// 在靠近pointA点处得到灰度最小点、灰度次小点
void CDibObject::GetWhitestPoint(CPoint pointA,CPoint pointB,unsigned char *pUnchSmooth,int width,
					 int height,CPoint &whitestPoint,CPoint &whiterPoint)
{
	static int extPointNum=3;             // 左右扩展点个数
	static int len=3;                     // 计算灰度时指定领域的范围半径
	CArray<CPoint,CPoint> whitePoints;     // 参与比较灰度的各个点
	double whitest;                       // 最大灰度值
	double white;                          // 灰度值
	int pointNum;                         // grayPoints中点的数量
	int i;                                // 循环变量
	CPoint point;                         // 变量

	GetParePoints(pointA,pointB,extPointNum,width,height,whitePoints);
	whitestPoint=whiterPoint=whitePoints.GetAt(0);
	whitest=GetAveGray(whitestPoint,pUnchSmooth,width,height,len);
	pointNum=whitePoints.GetSize();
	for(i=1;i<pointNum;i++)
	{
		point=whitePoints.GetAt(i);
		white=GetAveGray(point,pUnchSmooth,width,height,len);
		if(whitest>white)
		{
			whiterPoint=whitestPoint;
			whitestPoint=point;
			whitest=white;
		}
	}
}

// 得到参与比较的各个点，在经过pointA点并且斜率垂直于线段(pointA,pB)的小线段上找
void CDibObject::GetParePoints(CPoint pointA,CPoint pointB,int extPointNum,int width,int height,CArray<CPoint,CPoint> &parePoints)
{
	double lx,ly;           // x、y方向分量
	int x,y,X,Y;            // 点坐标
	double k;               // 线段斜率

	parePoints.RemoveAll();
	x=pointA.x;
	y=pointA.y;
	X=pointA.x+extPointNum+1;
	Y=pointA.y+extPointNum+1;
	lx=pointB.x-pointA.x;
	ly=pointB.y-pointA.y;
	if(lx==0)
	{
		for(x=pointA.x-extPointNum;x<X;x++)
		{
			if(x>-1 && x<width)
			{
				parePoints.Add(CPoint(x,y));
			}
		}
	}

	else if(ly==0)
	{
		for(y=pointA.y-extPointNum;y<Y;y++)
		{
			if(y>-1 && y<height)
			{
				parePoints.Add(CPoint(x,y));
			}
		}
	}

	else
	{
		k=-lx/ly;
		if(fabs(k)>1) 
		{
			for(y=pointA.y-extPointNum;y<Y;y+=1) 
			{ 
				x=int((y-pointA.y)/k+pointA.x+0.5);
				if(x>-1 && x<width && y>-1 && y<height)
					parePoints.Add(CPoint(x,y));
			} 
		} 
		else  
		{ 
			for(x=pointA.x-extPointNum;x<X;x+=1) 
			{ 
				y=int((x-pointA.x)*k+pointA.y+0.5);
				if(x>-1 && x<width && y>-1 && y<height)
					parePoints.Add(CPoint(x,y));
			} 
		} 
	}
}

// 灰色道路跟踪，从第一个点向第二个点方向延伸
void CDibObject::TrackRoadBySeedPoint(CPoint pointA,CPoint pointB,CPoint pointB1,unsigned char *pUnchSmooth,
				   int width,int height, double highCost, CArray<CPoint, CPoint> &roadPointArray)
{
	static int trackLen=16;                     // 延伸长度
	static double V=50;                        // 弯曲度常数
	CArray<CPoint,CPoint> linePoints1;          // 线段上的各个点
	CArray<CPoint,CPoint> linePoints2;          // 线段上的各个点
	int pointNum1,pointNum2;                    // 线段上点的个数
	int i,j;                                    // 循环变量
	CPoint point;                               // 点变量
	double aveGrayAB,aveGrayBC;                 // 平均灰度
	double costGray;                            // 平均灰度差
	double costDgray;                           // 灰度绝对差
	double costBend;                            // 当前线段偏离原直线程度
	double dirCha;                              // 方向差
	double costBC;                              // 线段BC评价值
	CPoint pointC,pointCEst,pointCEr;           // 记录点C，代价最优点记录于pointCEst中，代价次优点记录于pointEr中
	double costBCEst;                           // 代价最优值和次优值

	while(TRUE)
	{
		// 得到线段AB上的各个点
		GetLinePoints(pointA, pointB, linePoints1);
		
		// 得到线段AB的平均灰度
		aveGrayAB = 0;
		pointNum1 = linePoints1.GetSize();
		for(i=0; i<pointNum1; i++)
		{
			point = linePoints1.GetAt(i);
			aveGrayAB += pUnchSmooth[point.x + point.y*width];
		}
		aveGrayAB /= pointNum1;
		
		// 得到下一个跟踪点C
		pointC = GetNextTrackPoint(pointA, pointB, trackLen, width, height);
		if(GetJuLi(pointC,pointB) > trackLen/2)
		{
			// 得到C'的可能点，存储在数组linePoints1中
			GetParePoints(pointC, pointB, trackLen/2, width, height, linePoints1);
			
			// 对每个可能C'点进行评价
			costBCEst = 1.7e+308;
			pointCEst = pointCEr = linePoints1.GetAt(0);
			pointNum1 = linePoints1.GetSize();
			for(i=0; i<pointNum1; i++)
			{
				pointC = linePoints1.GetAt(i);  // 取出每个C'点
				
				// 得到BC'上的所有点，储存在数组linePoints2中
				GetLinePoints(pointC, pointB, linePoints2);
				// 计算BC'的平均灰度
				pointNum2 = linePoints2.GetSize();
				aveGrayBC = 0;
				for(j=0; j<pointNum2; j++)
				{
					point = linePoints2.GetAt(j);
					aveGrayBC += pUnchSmooth[point.x+point.y*width];
				}
				aveGrayBC /= pointNum2;
				
				// 计算线段AB和线段BC'的灰度差
				costGray = fabs(aveGrayAB-aveGrayBC);
				
				// 计算线段BC'的灰度绝对差
				costDgray = 0;
				for(j=0; j<pointNum2; j++)
				{
					point = linePoints2.GetAt(j);
					costDgray += fabs(pUnchSmooth[point.x+point.y*width] - aveGrayBC);
				}
				
				// 计算线段AB和线段BC的方向差
				dirCha = GetDirCha( pointB.x - pointA.x, pointB.y - pointA.y,
					pointC.x - pointB.x, pointC.y - pointB.y);
				
				// 计算线段BC'偏离线段AB程度
				costBend = V*sin(dirCha);
				
				// 线段BC代价
				costBC = costGray + costDgray/pointNum2 + costBend;
				if(costBCEst > costBC)
				{
					pointCEr = pointCEst;
					pointCEst = pointC;
					costBCEst = costBC;
				}
			}
			
			// 如果代价值超过最高阀值则改为AB1线段
			if(costBCEst > highCost)
			{
				pointB = pointB1;
				// 得到线段AB上的各个点
				GetLinePoints(pointA, pointB, linePoints1);
				
				// 得到线段AB的平均灰度
				aveGrayAB = 0;
				pointNum1 = linePoints1.GetSize();
				for(i=0; i<pointNum1; i++)
				{
					point = linePoints1.GetAt(i);
					aveGrayAB += pUnchSmooth[point.x + point.y*width];
				}
				aveGrayAB /= pointNum1;
				
				// 得到下一个跟踪点C
				pointC = GetNextTrackPoint(pointA, pointB, trackLen, width, height);
				if(GetJuLi(pointC,pointB) > trackLen/2)
				{
					// 得到C'的可能点，存储在数组linePoints1中
					GetParePoints(pointC, pointB, trackLen/2, width,height, linePoints1);
					
					// 对每个可能C'点进行评价
					costBCEst = 1.7e+308;
					pointCEst = pointCEr = linePoints1.GetAt(0);
					pointNum1 = linePoints1.GetSize();
					for(i=0; i<pointNum1; i++)
					{
						pointC = linePoints1.GetAt(i);  // 取出每个C'点
						
						// 得到BC'上的所有点，储存在数组linePoints2中
						GetLinePoints(pointC, pointB, linePoints2);
						// 计算BC'的平均灰度
						pointNum2 = linePoints2.GetSize();
						aveGrayBC = 0;
						for(j=0; j<pointNum2; j++)
						{
							point = linePoints2.GetAt(j);
							aveGrayBC += pUnchSmooth[point.x + point.y*width];
						}
						aveGrayBC /= pointNum2;
						
						// 计算线段AB和线段BC'的灰度差
						costGray = fabs(aveGrayAB-aveGrayBC);
						
						// 计算线段BC'的灰度绝对差
						costDgray = 0;
						for(j=0; j<pointNum2; j++)
						{
							point = linePoints2.GetAt(j);
							costDgray += fabs(pUnchSmooth[point.x+point.y*width] - aveGrayBC);
						}
						
						// 计算线段AB和线段BC的方向差
						dirCha = GetDirCha(pointB.x - pointA.x, pointB.y - pointA.y,
							pointC.x - pointB.x, pointC.y - pointB.y);
						
						// 计算线段BC'偏离线段AB程度
						costBend = V*sin(dirCha);
						
						// 线段BC代价
						costBC = costGray + costDgray/pointNum2 + costBend;
						if(costBCEst > costBC)
						{
							pointCEr = pointCEst;
							pointCEst = pointC;
							costBCEst = costBC;
						}
					}
					
					if(costBCEst > highCost)
						break;
				}
				else
				{
					GetLinePoints(pointA, pointC, linePoints1);
					pointNum1 = linePoints1.GetSize();
					for(i=0; i<pointNum1; i++)
					{
						roadPointArray.Add(linePoints1.GetAt(i));
					}
					
					break;
				}
			}

			// 将线段AB上的点记录于linePoints1
			GetLinePoints(pointA, pointB, linePoints1);
			pointNum1 = linePoints1.GetSize();
			for(i=0; i<pointNum1; i++)
			{
				roadPointArray.Add(linePoints1.GetAt(i));
			}

			pointA = pointB;
			pointB = pointCEst;
			pointB1 = pointCEr;
		}
		else
		{
			// 将线段AC上的点记录于linePoints1
			GetLinePoints(pointA, pointC, linePoints1);
			pointNum1 = linePoints1.GetSize();
			for(i=0; i<pointNum1; i++)
			{
				roadPointArray.Add(linePoints1.GetAt(i));
			}
			break;
		}
	}
}

// 得到线段AB上的各个点,记录点从A到B
void CDibObject::GetLinePoints(CPoint pointA,CPoint pointB,CArray<CPoint,CPoint> &linePoints)
{
	double lx,ly;           // x、y方向分量
	int x,y;                // 点坐标
	double k;               // 线段斜率

	linePoints.RemoveAll();

	x=pointA.x;
	y=pointA.y;
	lx=pointB.x-pointA.x;
	ly=pointB.y-pointA.y;
	if(lx==0)
	{
		if(pointA.y<pointB.y)
		{
			for(;y<=pointB.y;y++)
			{
				linePoints.Add(CPoint(pointA.x,y));
			}
		}
		else
		{
			for(;y>=pointB.y;y--)
			{
				linePoints.Add(CPoint(pointA.x,y));
			}
		}
	}

	else
	{
		k=ly/lx;
		if(fabs(k)>1) 
		{
			if(pointA.y<pointB.y)
			{
				for(;y<=pointB.y;y+=1) 
				{ 
					x=int((y-pointB.y)/k+pointB.x+0.5);
					linePoints.Add(CPoint(x,y));
				}
			}

			else
			{
				for(;y>=pointB.y;y-=1) 
				{ 
					x=int((y-pointB.y)/k+pointB.x+0.5);
					linePoints.Add(CPoint(x,y));
				}
			}
		} 
		else  
		{ 
			if(pointA.x<pointB.x)
			{
				for(;x<=pointB.x;x+=1) 
				{ 
					y=int((x-pointB.x)*k+pointB.y+0.5);
					linePoints.Add(CPoint(x,y));
				}
			}

			else
			{
				for(;x>=pointB.x;x-=1) 
				{ 
					y=int((x-pointB.x)*k+pointB.y+0.5);
					linePoints.Add(CPoint(x,y));
				}
			}
		} 
	}
}

// 得到下一个跟踪点，A—B—C
CPoint	CDibObject::GetNextTrackPoint(CPoint pointA,CPoint pointB,int trackLen,int width,int height)
{
	double lx,ly;           // x、y方向分量
	int i;                  // 循环变量
	int x,y;                // 点坐标
	double k;               // 线段斜率

	lx=pointB.x-pointA.x;
	ly=pointB.y-pointA.y;
	if(lx==0)
	{
		if(ly>0)
		{
			for(i=trackLen;i>=0;i--)
			{
				y=pointB.y+i;
				if(y<height)
					return CPoint(pointB.x,y);
			}
		}
		else
		{
			for(i=trackLen;i>=0;i--)
			{	
				y=pointB.y-i;
				if(y>=0)
					return CPoint(pointB.x,y);
			}
		}
	}

	else
	{
		k=(double)ly/lx;
		if(fabs(k)>1) 
		{
			if(ly>0)
			{
				for(i=trackLen;i>=0;i--)
				{
					y=pointB.y+i;
					x=int((y-pointB.y)/k+pointB.x+0.5);
					if(x>=0 && x<width && y>=0 && y<height)
						return CPoint(x,y);
				}
			}
			else
			{
				for(i=trackLen;i>=0;i--)
				{
					y=pointB.y-i;
					x=int((y-pointB.y)/k+pointB.x+0.5);
					if(x>=0 && x<width && y>=0 && y<height)
						return CPoint(x,y);
				}
			}
		} 
		else  
		{ 
			if(lx>0)
			{
				for(i=trackLen;i>=0;i--)
				{
					x=pointB.x+i;
					y=int((x-pointB.x)*k+pointB.y+0.5);
					if(x>=0 && x<width && y>=0 && y<height)
						return CPoint(x,y);
				}
			}
			else
			{
				for(i=trackLen;i>=0;i--)
				{
					x=pointB.x-i;
					y=int((x-pointB.x)*k+pointB.y+0.5);
					if(x>=0 && x<width && y>=0 && y<height)
						return CPoint(x,y);
				}
			}
		} 
	}
	return pointB;
}

// 清除位图数据
void CDibObject::ClearBitmap(unsigned char *pBitmap, int width, int height)
{
	int x, y;  // 循环变量

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			pBitmap[x + y*width] = 255;
		}
	}
}

// 画出所有道路
void CDibObject::PaintAllRoad(unsigned char *pBits, int widthBytes, CArray<CLine*, CLine*> &roadArray)
{
	int roadNum;  // 道路数量
	int i;  // 循环变量
	CLine *pLine;  // 线段指针

	roadNum = roadArray.GetSize();
	for(i=0; i<roadNum; i++)
	{
		pLine = roadArray.GetAt(i);
		if(pLine->m_edgeLines == 2)
		{
			PaintOneRoad(pBits, widthBytes, pLine->m_pointArray);
		}
	}
}

// 画出单条道路
void CDibObject::PaintOneRoad(unsigned char *pBits, int widthBytes, CArray<CPoint, CPoint> &roadPointArray)
{
	int pointNum;  // 点数量
	int i;  // 循环变量
	CPoint point;  // 点变量

	pointNum = roadPointArray.GetSize();
	for(i=0; i<pointNum; i++)
	{
		point = roadPointArray.GetAt(i);
		pBits[point.x + point.y*widthBytes] = 0;
	}
}

// 知识推理减少虚警
void CDibObject::FindRoad(CArray<CLine*,CLine*> &roadArray, int width, int height)
{
	BOOL finish = FALSE;  // 是否完成
	int roadNum;  // 道路数量
	int i, j;  // 循环变量
	CLine *pLine1, *pLine2;  // 道路指针

	// 连接头尾相连的可能中间断掉过的可能道路
	while(!finish)
	{
		finish = TRUE;
		roadNum = roadArray.GetSize();
		for(i=0; i<roadNum; i++)
		{
			pLine1 = roadArray.GetAt(i);
			for(j=0; j<roadNum; j++)
			{
				if(i != j)
				{
					pLine2 = roadArray.GetAt(j);
					if(HeadTailLink(pLine1, pLine2))
					{
						roadArray.RemoveAt(j);
						finish = FALSE;
						break;
					}
				}
			}
			
			if(!finish)
			{
				break;
			}
		}
	}
	
	// 通过边界端点检查是否是真正的道路
	CheckRoadByEdgePoint(roadArray, width, height);

	finish = FALSE;
	roadNum = roadArray.GetSize();
	while(finish)
	{
		finish = TRUE;
		for(i=0; i<roadNum; i++)
		{
			// 索引为i的道路是否是真正的道路
			if(((CLine *)roadArray.GetAt(i))->m_edgeLines == 2)
			{
				for(j=0; j<roadNum; j++)
				{
					if(i != j  && ((CLine *)roadArray.GetAt(j))->m_edgeLines != 2)
					{
						// 索引为j的线段是否是连接到索引为i的道路上的道路
						if(CheckRoadByRoad(roadArray, j, i))
						{
							finish = FALSE;
						}
					}
				}
			}
		}
	}
}

// 连接头尾相连的可能中间断掉过的可能道路
BOOL CDibObject::HeadTailLink(CLine *pLine1, CLine *pLine2)
{
	static double highJuLi = 15;  // 线段两端点相连的最大距离
	CPoint startPoint1, endPoint1, startPoint2, endPoint2;  // 点变量
	CArray<CPoint, CPoint> linePoints;  // 线段上的点
	int pointNum;  // 点数量
	CPoint point;  // 点变量
	int i;  // 循环变量

	startPoint1 = pLine1->m_pointArray.GetAt(0);
	startPoint2 = pLine2->m_pointArray.GetAt(0);

	// 线段pLine1前端与线段pLine2前端可相连
	if(GetJuLi(startPoint1, startPoint2) <= highJuLi)
	{
		// 填补两线段之间的缺口
		GetLinePoints(startPoint1, startPoint2, linePoints);
		pointNum = linePoints.GetSize();
		if(pointNum > 1)
		{
			point = linePoints.GetAt(0);
			if(startPoint1 == point)
			{
				pointNum -= 1;
				for(i=1; i<pointNum; i++)
				{
					point = linePoints.GetAt(i);
					pLine1->m_pointArray.InsertAt(0, point);
				}
			}
			else
			{
				for(i=pointNum-2; i>0; i--)
				{
					point = linePoints.GetAt(i);
					pLine1->m_pointArray.InsertAt(0, point);
				}
			}
		}

		// 将线段pLine2中的点加入pLine1
		pointNum = pLine2->m_pointArray.GetSize();
		for(i=0; i<pointNum; i++)
		{
			point = pLine2->m_pointArray.GetAt(i);
			pLine1->m_pointArray.InsertAt(0, point);
		}

		return TRUE;
	}

	endPoint2 = pLine2->m_pointArray.GetAt(pLine2->m_pointArray.GetSize() - 1);
	// 线段pLine1前端与线段pLine2后端可相连
	if(GetJuLi(startPoint1, endPoint2) <= highJuLi)
	{
		// 填补两线段之间的缺口
		GetLinePoints(startPoint1, endPoint2, linePoints);
		pointNum = linePoints.GetSize();
		if(pointNum > 1)
		{
			point = linePoints.GetAt(0);
			if(startPoint1 == point)
			{
				pointNum -= 1;
				for(i=1; i<pointNum; i++)
				{
					point = linePoints.GetAt(i);
					pLine1->m_pointArray.InsertAt(0, point);
				}
			}
			else
			{
				for(i=pointNum-2; i>0; i--)
				{
					point = linePoints.GetAt(i);
					pLine1->m_pointArray.InsertAt(0, point);
				}
			}
		}

		// 将线段pLine2中的点加入pLine1
		pointNum = pLine2->m_pointArray.GetSize();
		for(i=pointNum-1; i>=0; i--)
		{
			point = pLine2->m_pointArray.GetAt(i);
			pLine1->m_pointArray.InsertAt(0, point);
		}

		return TRUE;
	}

	endPoint1 = pLine1->m_pointArray.GetAt(pLine1->m_pointArray.GetSize() - 1);
	// 线段pLine1后端与线段pLine2后端可相连
	if(GetJuLi(endPoint1, endPoint2) <= highJuLi)
	{
		// 填补两线段之间的缺口
		GetLinePoints(endPoint1, endPoint2, linePoints);
		pointNum = linePoints.GetSize();
		if(pointNum > 1)
		{
			point = linePoints.GetAt(0);
			if(endPoint1 == point)
			{
				pointNum -= 1;
				for(i=1; i<pointNum; i++)
				{
					point = linePoints.GetAt(i);
					pLine1->m_pointArray.Add(point);
				}
			}
			else
			{
				for(i=pointNum-2; i>0; i--)
				{
					point = linePoints.GetAt(i);
					pLine1->m_pointArray.Add(point);
				}
			}
		}

		// 将线段pLine2中的点加入pLine1
		pointNum = pLine2->m_pointArray.GetSize();
		for(i=pointNum-1; i>=0; i--)
		{
			point = pLine2->m_pointArray.GetAt(i);
			pLine1->m_pointArray.Add(point);
		}

		return TRUE;
	}
	
	return FALSE;
}

// 通过边界端点检查是否是真正的道路
void CDibObject::CheckRoadByEdgePoint(CArray<CLine*, CLine*> &roadArray, int width, int height)
{
	int roadNum;  // 线段数量
	int i;  // 循环变量
	CLine *pLine;  // 线段指针
	CPoint point;  // 点变量

	roadNum = roadArray.GetSize();
	for(i=0; i<roadNum; i++)
	{
		pLine = roadArray.GetAt(i);
		point = pLine->m_pointArray.GetAt(0);
		if(IsEdgePoint(point, width, height))
		{
			pLine->m_edgeLines++;
		}

		point = pLine->m_pointArray.GetAt(pLine->m_pointArray.GetSize() - 1);
		if(IsEdgePoint(point, width, height))
		{
			pLine->m_edgeLines++;
		}
	}
}

// 索引为m的线段是否是连接到索引为n的道路上的道路
BOOL CDibObject::CheckRoadByRoad(CArray<CLine*, CLine*> &roadArray, int j, int i)
{
	int roadNum;  // 道路数量
	CLine *pLine1, *pLine2;  // 道路指针
	CPoint startPoint, endPoint;  // 点变量
	int pos;  // 中间变量
	CArray<CPoint, CPoint> linePoints;  // 线段上的点
	int pointNum;  // 点数量
	CPoint point;  // 点变量
	int m;  // 循环变量

	roadNum = roadArray.GetSize();
	
	// 取出线段j
	if(j<0 || j>=roadNum)
	{
		return FALSE;
	}
	pLine1 = roadArray.GetAt(j);
	startPoint = pLine1->m_pointArray.GetAt(0);

	// 取出道路i
	if(i<0 || i>=roadNum)
	{
		return FALSE;
	}
	pLine2 = roadArray.GetAt(i);

	// 线段pLine1前端与线段pLine2相邻
	if(XiangLin(startPoint, pLine2, 15, pos))
	{
		endPoint = pLine2->m_pointArray.GetAt(pos);
		GetLinePoints(startPoint, endPoint, linePoints);
		pointNum = linePoints.GetSize();

		// 线段pLine1是道路
		if(pointNum > 1)
		{
			point = linePoints.GetAt(0);
			if(startPoint == point)
			{
				for(m=1; m<pointNum; m++)
				{
					point = linePoints.GetAt(m);
					pLine1->m_pointArray.InsertAt(0, point);
				}
			}
			else
			{
				for(m=pointNum-2; m>=0; m--)
				{
					point = linePoints.GetAt(m);
					pLine1->m_pointArray.InsertAt(0,point);
				}
			}

			pLine1->m_edgeLines = 2;
			return TRUE;
		}
	}

	endPoint = pLine1->m_pointArray.GetAt(pLine1->m_pointArray.GetSize() - 1);
	// 线段pLine1后端与线段pLine2相邻
	if(XiangLin(endPoint, pLine2, 15, pos))
	{
		startPoint = pLine2->m_pointArray.GetAt(pos);
		GetLinePoints(endPoint, startPoint, linePoints);
		pointNum = linePoints.GetSize();

		// 线段pLine1是道路
		if(pointNum > 1)
		{
			point = linePoints.GetAt(0);
			if(endPoint == point)
			{
				for(m=1; m<pointNum; m++)
				{
					point = linePoints.GetAt(m);
					pLine1->m_pointArray.Add(point);
				}
			}
			else
			{
				for(m=pointNum-2; m>=0; m--)
				{
					point = linePoints.GetAt(m);
					pLine1->m_pointArray.Add(point);
				}
			}

			pLine1->m_edgeLines = 2;
			return TRUE;
		}
	}

	return FALSE;
}

// 自动湖泊提取
BOOL CDibObject::AutoLakeExtract(CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchEdge;  // 存放提取出的边界
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度
	CArray<CLine*,CLine*> edgeLineArray;  // 道路边界线数组
	CGroup *pGroups = NULL;  // 分组数组
	int groupNum;  // 线段数量,即初始分组数

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];
	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];
	// 对原图象进行高斯滤波
	GaussianSmooth(pUnchEdge, m_nWidth, m_nHeight, sigma, pUnchSmooth);

	pnGradX = new int[m_nWidth * m_nHeight];
	pnGradY = new int[m_nWidth * m_nHeight];
	// 计算方向导数
	DirGrad(pUnchSmooth, m_nWidth, m_nHeight, pnGradX, pnGradY);

	pnGradMag = new int[m_nWidth * m_nHeight];
	// 计算梯度的幅度
	GradMagnitude(pnGradX, pnGradY, m_nWidth, m_nHeight, pnGradMag);

	// 应用non-maximum 抑制
	NonmaxSuppress(pnGradMag, pnGradX, pnGradY, m_nWidth, m_nHeight, pUnchEdge);

	// 应用Hysteresis，找到所有的边界
	Hysteresis(pnGradMag, m_nWidth, m_nHeight, dRatioLow, dRatioHigh, pUnchEdge);

	// 获取边界线记录于变量edgeLineArray中
	GetEdgeLine(pUnchEdge, m_nWidth, m_nHeight, 0, edgeLineArray);

	// 对线段两端点进行延伸，以填补小缺口
	ExtendLine(edgeLineArray, pUnchEdge, m_nWidth, m_nHeight);

	// 寻找湖泊
	FindLake(pUnchSmooth, pUnchEdge, m_nWidth, m_nHeight);

	// 去除边界点或通过验证的区域,边界点灰度为0,通过验证的区域点灰度为1,
	// 通过验证的边界点灰度值为2,可自由选择
	RemoveEdge(pUnchEdge, m_nWidth, m_nHeight, 0);

	// 获取湖泊边界线记录于变量edgeLineArray中
	GetEdgeLine(pUnchEdge, m_nWidth, m_nHeight, 2, edgeLineArray);

	// 初始化分组数组
	InitGroups(pGroups, groupNum, edgeLineArray);

	// 将同一区域中的边界线编为一组
	CengCiBianZuLake(edgeLineArray, pGroups, groupNum);

	
	// 将编组信息记录于文本中
	RecordGroupsToText(pGroups, groupNum);

	// 调试用
	// 画出单条道路边界线
	//PaintRoadEdgeLineByGroup(pUnchEdge,m_nWidth,edgeLineArray,10);
	//PaintOneRoadEdgeLine(pUnchEdge,m_nWidth,edgeLineArray,0);
	//pgUnchEdge = pUnchEdge;
	//ngWidth = m_nWidth;

	// 清除位图pUnchEdge
	//for(int i=0; i<m_nWidth; i++)
	//{
		//for(int j=0; j<m_nHeight; j++)
		//{
			//pUnchEdge[i + j*m_nWidth] = 255;
		//}
	//}
	// 调试完

	// 对湖泊区域进行验证
	CheckLake(pGroups, groupNum, edgeLineArray, pUnchEdge, m_nWidth, m_nHeight);

	// 去除边界点或通过验证的区域,边界点灰度为0,通过验证的区域点灰度为1,
	// 通过验证的边界点灰度值为2,可自由选择
	RemoveEdge(pUnchEdge, m_nWidth, m_nHeight, 0);

	// 将边界线信息记录于文本中
	RecordWaterEdgeLineToText(edgeLineArray);
	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	// 将pUnchEdge所指向的位图数据复制到pBits所指向位图的位图
	CopyBitmapPixel(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);
	
	::GlobalUnlock(pDibObject->m_hDib);
	if(pGroups)
	{
		delete []pGroups;
	}
	// 清除edgeLineArray数组
	ClearLineArray(edgeLineArray);
	delete []pnGradMag;
	delete []pnGradY;
	delete []pnGradX;
	delete []pUnchSmooth;
	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);

	return TRUE;
}

// 获取湖泊边界记录于变量edgeLineArray中
void CDibObject::GetEdgeLine(unsigned char *pUnchEdge, int width, int height, int gray, CArray<CLine*,CLine*> &edgeLineArray)
{
	// 对8邻域象素进行查询
	static int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1};
	static int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1};
	int k;  // 循环变量
	int x, y, xx, yy;  // 坐标变量
	int nPos;  // 一般变量
	CArray<CPoint,CPoint> pointArray;  // 点数组
	BOOL finish;  // 搜寻是否结束
	CPoint point, point1;  // 点变量
	CLine *pLine;  // 线段指针
	int pointNum;  // 点的数量
	double lineLen;  // 线段长度

	// 清除数组roadEdgeLineArray中数据
	ClearLineArray(edgeLineArray);
	CLine::groupID=0;

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x + y*width;
			if(pUnchEdge[nPos] == gray)
			{
				pUnchEdge[nPos] = 8;
				finish = FALSE;
				pointArray.RemoveAll();
				pointArray.Add(CPoint(x,y));
				while(!finish)
				{
					finish = TRUE;
					point = pointArray.GetAt(0);
					for(k=0; k<8; k++)
					{
						xx = point.x + xNb[k];
						yy = point.y + yNb[k];
						nPos = xx + yy*width;
						if(xx>=0 && xx<width && yy>=0 && yy<height &&pUnchEdge[nPos] == gray)
						{
							pUnchEdge[nPos] = 8;
							finish = FALSE;
							pointArray.InsertAt(0,CPoint(xx,yy));
							break;
						}
					}
				}

				// 将该线段记录入数组edgeLineArray中
				pLine=new CLine;
				pointNum = pointArray.GetSize();
				for(k=0; k<pointNum; k++)
				{
					pLine->m_pointArray.Add(pointArray.GetAt(k));
				}

				// 计算线段长度
				lineLen = 0;
				pointNum -= 2;
				for(k=0; k<pointNum; k++)
				{
					point = pointArray.GetAt(k);
					point1 = pointArray.GetAt(k+2);
					lineLen += GetJuLi(point, point1);
				}
				pLine->m_lineLength = lineLen/2;
				edgeLineArray.Add(pLine);
			}
		}
	}

	// 还原pUnchEdge为原来的值
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x + y*width;
			if(pUnchEdge[nPos] == 8)
			{
				pUnchEdge[nPos] = gray;
			}
		}
	}
}

// 对线段两端点进行延伸，以填补小缺口
void CDibObject::ExtendLine(CArray<CLine*, CLine*> &edgeLineArray,
		unsigned char *pUnchEdge, int width, int height)
{
	int lineNum;             // 线段数量
	int i;                   // 循环变量
	CLine *pLine;            // 选段变量
	CPoint point;            // 点变量

	lineNum = edgeLineArray.GetSize();
	for(i=0; i<lineNum; i++)
	{
		pLine = edgeLineArray.GetAt(i);    // 取出线段
		point = pLine->m_pointArray.GetAt(0);  // 取出前端点
		ExtendPoint(point, pUnchEdge, width, height);  // 对前端点进行延伸
		point = pLine->m_pointArray.GetAt(pLine->m_pointArray.GetSize()-1);  // 取出后端点
		ExtendPoint(point, pUnchEdge, width, height);  // 对后端点进行延伸
	}
}

/*************************************************************************
以给定点为中心，以2×extendLen为边长构成一个正方形，在正方形的边上寻找边界
点(即灰度值为0或图片边界的点)，找到后连接改点到point点，此步的目的是在给
定点处填补小缺口   
 ************************************************************************/
void CDibObject::ExtendPoint(CPoint point, unsigned char *pUnchEdge, int width, int height)
{
	static int extendLen = 5;        // 延伸长度
	int X,Y;                       // 延伸坐标点上限
	int i;                         // 循环变量

	X = point.x + extendLen;
	Y = point.y - extendLen;

	// 正四方形下边寻找线段点并连接
	if(Y > 0)
	{
		for(i=point.x-extendLen; i<=X; i++)
		{
			if(i>=0 && i<width)
			{
				if(pUnchEdge[i+Y*width] == 0)
				{
					LinkPoints(point, CPoint(i,Y), pUnchEdge, width);
				}
			}
		}
	}
	else
	{
		LinkPoints(point, CPoint(point.x,0), pUnchEdge, width);
	}

	// 正四方形上边寻找线段点并连接
	Y = point.y + extendLen;
	if(Y < height)
	{
		for(i=point.x-extendLen; i<=X; i++)
		{
			if(i>=0 && i<width)
			{
				if(pUnchEdge[i+Y*width] == 0)
				{
					LinkPoints(point, CPoint(i,Y), pUnchEdge, width);
				}
			}
		}
	}
	else
	{
		LinkPoints(point, CPoint(point.x, height-1), pUnchEdge, width);
	}

	// 正四方形左边寻找线段点并连接
	X = point.x - extendLen;
	if(X > 0)
	{
		for(i=point.y-extendLen; i<=Y; i++)
		{
			if(i>=0 && i<height)
			{
				if(pUnchEdge[X+i*width] == 0)
				{
					LinkPoints(point, CPoint(X,i), pUnchEdge, width);
				}
			}
		}
	}
	else
	{
		LinkPoints(point, CPoint(0,point.y), pUnchEdge, width);
	}

	// 正四方形右边寻找线段点并连接
	X = point.x + extendLen;
	if(X < width)
	{
		for(i=point.y-extendLen; i<=Y; i++)
		{
			if(i>=0 && i<height)
			{
				if(pUnchEdge[X+i*width] == 0)
				{
					LinkPoints(point, CPoint(X,i), pUnchEdge, width);
				}
			}
		}
	}
	else
	{
		LinkPoints(point, CPoint(width-1,point.y), pUnchEdge, width);
	}
}

// 在pUnchEdge上连接给点的两个点
void CDibObject::LinkPoints(CPoint pointA, CPoint pointB, unsigned char *pUnchEdge, int width)
{
	double lx,ly;           // x、y方向分量
	int x,y;                // 点坐标
	double k;               // 线段斜率

	if(pointA == pointB)
		return;

	x=pointA.x;
	y=pointA.y;
	lx=pointB.x-pointA.x;
	ly=pointB.y-pointA.y;
	if(lx==0)
	{
		if(y<pointB.y)
		{
			for(;y<=pointB.y;y++)
			{
				pUnchEdge[x+y*width]=0;
			}
		}
		else
		{
			for(;y>=pointB.y;y--)
			{
				pUnchEdge[x+y*width]=0;
			}
		}
	}

	else
	{
		k=ly/lx;
		if(fabs(k)>1) 
		{
			if(y<pointB.y)
			{
				for(;y<=pointB.y;y+=1) 
				{ 
					x=int((y-pointB.y)/k+pointB.x+0.5);
					pUnchEdge[x+y*width]=0;
				}
			}

			else
			{
				for(;y>=pointB.y;y-=1) 
				{ 
					x=int((y-pointB.y)/k+pointB.x+0.5);
					pUnchEdge[x+y*width]=0;
				}
			}
		} 
		else  
		{ 
			if(x<pointB.x)
			{
				for(;x<=pointB.x;x+=1) 
				{ 
					y=int((x-pointB.x)*k+pointB.y+0.5);
					pUnchEdge[x+y*width]=0;
				}
			}

			else
			{
				for(;x>=pointB.x;x-=1) 
				{ 
					y=int((x-pointB.x)*k+pointB.y+0.5);
					pUnchEdge[x+y*width]=0;
				}
			}
		} 
	}
}

/*************************************************************************
寻找湖泊，在整个图片上搜索灰度均匀的小片长方形区域，并以此区域扩展至边界点
(即已标记的灰度值为0的点)  
*************************************************************************/
void CDibObject::FindLake(unsigned char *pUnchSmooth, unsigned char *pUnchEdge, int width, int height)
{
	static double highFangcha=400;  // 灰度方差上限阀值
	int subWidth=2*10+1,subHeight=2*10+1;   // 小片长方形的长宽值
	int widthNum=width/subWidth;           // 图片宽度被划分成的段数 
	int heightNum=height/subHeight;        // 图片长度被划分成的段数
	int i,j;                                // 循环变量
	int x,y;                                // 点坐标变量
	double aveGray,grayFangCha;             // 平均灰度和灰度方差变量

	for(i=0;i<widthNum;i++)
	{
		for(j=0;j<heightNum;j++)
		{
			x=i*subWidth+subWidth/2;
			y=j*subHeight+subHeight/2;
			// 调用函数AnalyseGray获取平均灰度和灰度方差
			AnalyseGray(pUnchSmooth,width,x,y,subWidth/2,subHeight/2,aveGray,grayFangCha);
			// 灰度方差小于阀值并且该点未被湖泊区域覆盖，则以该点扩展湖泊区域
			if(grayFangCha<highFangcha && pUnchEdge[x+y*width]==255)
			{
				FillByPoint(pUnchSmooth,pUnchEdge,width,height,CPoint(x,y));
			}
		}
	}
}

/*************************************************************************
对给定区域灰度值进行分析，此区域是以(x,y)为中心，以2×subWidth+1为宽度，
以2×subHeight+1为高度的长方形，并在变量aveGray中返回平均灰度，在变量
grayFangCha中返回灰度方差 
 ************************************************************************/
void CDibObject::AnalyseGray(unsigned char *pUnchSmooth, int width, int x, int y,
							 int subWidth, int subHeight, double &aveGray, double &grayFangCha)
{
	int i,j;
	int X=x+subWidth;
	int Y=y+subHeight;
	int size;

	aveGray=0;
	grayFangCha=0;

	// 求平均灰度
	for(i=x-subWidth;i<=X;i++)
	{
		for(j=y-subHeight;j<=Y;j++)
		{
			aveGray+=pUnchSmooth[i+j*width];
		}
	}
	size=(2*subWidth+1)*(2*subHeight+1);
	aveGray/=size;

	// 求灰度方差
	for(i=x-subWidth;i<=X;i++)
	{
		for(j=y-subHeight;j<=Y;j++)
		{
			grayFangCha+=fabs(pUnchSmooth[i+j*width]-aveGray);
		}
	}
}

/*************************************************************************
对保存在数组pointArray中的点在位图pUnchSmooth中的灰度进行分析，并在变量
aveGray中返回平均灰度，在变量grayFangCha中返回灰度方差 
 ************************************************************************/
void CDibObject::AnalyseGray(unsigned char *pUnchSmooth, int width, CArray<CPoint, CPoint> &pointArray,
							 double &aveGray, double &grayFangCha)
{
	int pointNum;  // 点的个数
	int i;  // 循环变量
	CPoint point;  // 点变量

	aveGray = grayFangCha = 0;

	// 取得平均灰度
	pointNum = pointArray.GetSize();
	for(i=0; i<pointNum; i++)
	{
		point = pointArray.GetAt(i);
		aveGray += pUnchSmooth[point.x + point.y*width];
	}
	aveGray /= pointNum;

	// 取得灰度方差
	for(i=0; i<pointNum; i++)
	{
		point = pointArray.GetAt(i);
		grayFangCha += fabs(pUnchSmooth[point.x + point.y*width] - aveGray);
	}
	grayFangCha /= pointNum;
}

/*************************************************************************
 按给定点扩展湖泊区域
 灰度值说明: 
     原边界点:0     区域:255    通过验证的边界点:2    通过验证区域:1
	 带待验证边界:253      待验证区域:252   未通过验证边界和区域:254
*************************************************************************/
void CDibObject::FillByPoint(unsigned char *pUnchSmooth,unsigned char *pUnchEdge,
				 int width,int height,CPoint point)
{
	CArray<CPoint,CPoint> pointArray;   // 记录扩展点堆栈
	CPoint pointA,pointB;               // 点变量
	int nPos;                           // 数组索引
	int i;                              // 循环变量
	int grays[256];                     // 存放灰度值统计
	memset(grays,0,sizeof(int)*256);    // 灰度值统计清零

	// 对4邻域象素进行查询
	static int xNb[4] = {1, 0,-1, 0} ;
	static int yNb[4] = {0, 1, 0,-1} ;

	nPos=point.x+point.y*width;
	pUnchEdge[nPos]=252;
	grays[pUnchSmooth[nPos]]++;
	pointArray.Add(point);
	while(pointArray.GetSize())
	{
		pointA=pointArray.GetAt(0);
		pointArray.RemoveAt(0);

		for(i=0;i<4;i++)
		{
			pointB.x=pointA.x+xNb[i];
			pointB.y=pointA.y+yNb[i];
			if(pointB.x>=0 && pointB.x<width && pointB.y>=0 && pointB.y<height)
			{
				nPos=pointB.x+pointB.y*width;
				if(pUnchEdge[nPos]==0)
					pUnchEdge[nPos] = 253;
				else if(pUnchEdge[nPos]==255)
				{
					pUnchEdge[nPos]=252;
					grays[pUnchSmooth[nPos]]++;
					pointArray.Add(pointB);
				}
			}
		}
	}

	// 进行验证，保留灰度方差小的区域
	VerifyGrayArea(pUnchEdge,width,height,grays);
}

/*************************************************************************
 按给定点扩展湖泊区域
 灰度值说明: 
     原边界点:0     区域:255    区域填充后边界点:2   区域:1
*************************************************************************/
void CDibObject::FillByPoint(unsigned char *pUnchEdge, int width,int height,CPoint point)
{
	CArray<CPoint,CPoint> pointArray;   // 记录扩展点堆栈
	CPoint pointA,pointB;               // 点变量
	int nPos;                           // 数组索引
	int i;                              // 循环变量

	// 对4邻域象素进行查询
	static int xNb[4] = {1, 0,-1, 0} ;
	static int yNb[4] = {0, 1, 0,-1} ;

	nPos=point.x+point.y*width;
	if(pUnchEdge[nPos] == 0)
	{
		AfxMessageBox("选取种子点错误—位置!");
		return ;
	}
	pUnchEdge[nPos] = 1;
	pointArray.Add(point);
	while(pointArray.GetSize())
	{
		pointA=pointArray.GetAt(0);
		pointArray.RemoveAt(0);

		for(i=0;i<4;i++)
		{
			pointB.x=pointA.x+xNb[i];
			pointB.y=pointA.y+yNb[i];
			if(pointB.x>=0 && pointB.x<width && pointB.y>=0 && pointB.y<height)
			{
				nPos=pointB.x+pointB.y*width;
				if(pUnchEdge[nPos]==0)
					pUnchEdge[nPos] = 2;
				else if(pUnchEdge[nPos]==255)
				{
					pUnchEdge[nPos]=1;
					pointArray.Add(pointB);
				}
			}
		}
	}
}

void CDibObject::FillByPoint1(unsigned char *pUnchEdge,unsigned char *pUnchSmooth,
							 int width,int height,CPoint point)
{
	CArray<CPoint,CPoint> pointArray;   // 记录扩展点堆栈
	CPoint pointA,pointB;               // 点变量
	int nPos;                           // 数组索引
	int x, y;                              // 循环变量
	int len=5;
	double aveHuidu=0;
	
	// 对4邻域象素进行查询
	static int xNb[4] = {1, 0,-1, 0} ;
	static int yNb[4] = {0, 1, 0,-1} ;
	
	for(x=point.x-len; x<=point.x+len; x++)
	{
		for(y=point.y-len; y<=point.y+len;y++)
		{
			nPos = x + y*width;
			aveHuidu += pUnchEdge[nPos];
		}
	}
	len = (2*len+1)*(2*len+1);
	aveHuidu/=len;
	
	for(x=0;x<width;x++)
	{
		for(y=0;y<height;y++)
		{
			nPos = x +y*width;
			if( (pUnchEdge[nPos]<(aveHuidu+20)) && (pUnchEdge[nPos]>(aveHuidu-20)))
			{
				pUnchSmooth[nPos]=0;
			}
		}
	}
}

// 进行验证，保留灰度方差小的区域
double CDibObject::VerifyGrayArea(unsigned char *pUnchEdge,int width,int height,int grays[])
{
	static highGrayFangCha=10;          // 灰度方差最高阀值
	int i,j;                            // 循环变量
	double sumGray=0;                   // 总灰度值
	int sumPoints=0;                    // 点的个数
	double aveGray;                     // 平均灰度
	int nPos;                           // 数组索引

	// 对灰度统计进行分析
	for(i=0;i<256;i++)
	{
		sumGray+=grays[i]*i;
		sumPoints+=grays[i];
	}
	aveGray=sumGray/sumPoints;

	// 计算灰度方差
	sumGray=0;
	for(i=0;i<256;i++)
	{
		sumGray+=fabs(i-aveGray)*grays[i];
	}
	sumGray/=sumPoints;

	// 去除灰度方差大于highGrayFangCha的区域
	if(sumGray>highGrayFangCha)
	{
		for(i=0;i<width;i++)
		{
			for(j=0;j<height;j++)
			{
				nPos=i+j*width;
				// 将待验证边界和区域置为未通过验证的边界和区域
				if(pUnchEdge[nPos]==252 || pUnchEdge[nPos]==253)
					pUnchEdge[nPos]=254;
			}
		}
		return -1;
	}

	// 保留灰度方差小于highGrayFangCha的区域
	else
	{
		for(i=0;i<width;i++)
		{
			for(j=0;j<height;j++)
			{
				nPos=i+j*width;
				// 将待验证边界置为通过验证的边界
				if(pUnchEdge[nPos]==253)
					pUnchEdge[nPos]=2;
				// 将待验证区域置为通过验证的区域
				else if(pUnchEdge[nPos]==252)
					pUnchEdge[nPos]=1;
			}
		}
		return aveGray;
	}
}

// 去除边界点或通过验证的区域,边界点灰度为0,通过验证的区域点灰度为1,通过验证的边界点灰度为2,可自由选择
void CDibObject::RemoveEdge(unsigned char *pUnchEdge, int width, int height, int gray)
{
	int x, y;        // 循环变量
	int nPos;        // 数组索引

	for(x=0; x<width; x++)
		for(y=0; y<height; y++)
		{
			nPos = x+y*width;
			if(pUnchEdge[nPos] <= gray)
				pUnchEdge[nPos] = 255;
		}
}

// 层次记号编组
void CDibObject::CengCiBianZuLake(CArray<CLine*,CLine*> &roadEdgeLineArray, CGroup *pGroups, int groupNum)
{
	int lineNum;  // 线段数量
	BOOL finish = FALSE;  // 是否完成
	int i, j, k;  // 循环变量
	CLine *pLine1, *pLine2;  // 线段指针
	int groupID;  // 线段编组号
	
	lineNum = roadEdgeLineArray.GetSize();
	while(!finish)
	{
		finish = TRUE;
		for(i=0; i<lineNum; i++)
		{
			pLine1 = roadEdgeLineArray.GetAt(i);
			for(j=0; j<lineNum; j++)
			{
				pLine2 = roadEdgeLineArray.GetAt(j);
				// 如果编组成功则合并相应组
				if(pLine1->m_groupID != pLine2->m_groupID && LinesXiangLin(pLine1, pLine2))
				{
					// 将所有编组号为pLine2->m_groupID的线段的编组号改为pLine1->m_groupID
					groupID = pLine2->m_groupID;
					for(k=0; k<lineNum; k++)
					{
						pLine2 = roadEdgeLineArray.GetAt(k);
						if(pLine2->m_groupID == groupID)
						{
							pLine2->m_groupID = pLine1->m_groupID;
						}
					}
					// 合并线段组
					HeBingGroup(pGroups, groupNum, pLine1->m_groupID, groupID, FALSE);
					finish = FALSE;
				}
			}
		}
	}
}

// 检验两线段是否相邻，若相邻则进行连接
BOOL CDibObject::LinesXiangLin(CLine *pLine1, CLine *pLine2)
{
	static double highJuLi=3;   // 两线段相邻的最大距离
	CPoint startPt,endPt;   // 线段端点
	int pos;  // 相邻点
	BOOL xiangLin=FALSE;    // 两线段是否相邻

	startPt=pLine1->m_pointArray.GetAt(0);
	endPt=pLine1->m_pointArray.GetAt(pLine1->m_pointArray.GetSize() - 1);

	// 判断线段1前端点是否接近线段2
	if(XiangLin(startPt, pLine2, highJuLi, pos))
	{
		xiangLin=TRUE;
	}

	// 判断线段1后端点是否接近线段2
	else if(XiangLin(endPt, pLine2, highJuLi, pos))
	{
		xiangLin=TRUE;
	}

	return xiangLin;
}

// 对湖泊区域进行验证
void CDibObject::CheckLake(CGroup *pGroups, int groupNum, CArray<CLine*, CLine *> &edgeLineArray,
						   unsigned char *pUnchEdge, int width, int height)
{
	// 对8邻域象素进行查询
	static int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1} ;
	static int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1} ;
	int i, j, k;  // 循环变量
	int lineNum;  // 边界线数量
	CLine *pLine;  // 线段指针
	CPoint point, point1;  // 点变量
	BOOL find;  // 是否找到

	lineNum = edgeLineArray.GetSize();
	for(i=0; i<groupNum; i++)
	{
		if(pGroups[i].m_groupID >= 0)
		{
			// 如果区域边界是规则图形
			if(!CheckLakeByEdge(pGroups[i].m_groupID, edgeLineArray))
			{
				find = FALSE;
				// 在属于pGroups[i].m_groupID组的边界线附近寻找已验证的区域点
				for(j=0; j<lineNum && !find; j++)
				{
					pLine = edgeLineArray.GetAt(j);
					if(pLine->m_groupID == pGroups[i].m_groupID)
					{
						point = pLine->m_pointArray.GetAt(0);
						for(k=0; k<8; k++)
						{
							point1.x = point.x + xNb[k];
							point1.y = point.y + yNb[k];
							if(point1.x>=0 && point1.x<width && point1.y>=0 && point1.y<height
								&& pUnchEdge[point1.x + point1.y*width] == 1)
							{
								find = TRUE;
								break;
							}
						}
					}
				}

				// 验证已验证的区域点(灰度为1)是否在已验证的边界点(灰度为2)内部
				if(CheckInsidePoint(point1, pGroups[i].m_groupID, edgeLineArray, pUnchEdge, width, height))
				{
					// 去除区域及边界
					RemoveAreaAndEdgeByPoint(point1, pUnchEdge, width, height, 2);
				}
			}
		}
	}
}

// 验证已验证的区域点(灰度为1)是否在已验证的边界点(灰度为2)内部
BOOL CDibObject::CheckInsidePoint(CPoint point, int groupID, CArray<CLine*, CLine*> &edgeLineArray,
								  unsigned char *pUnchEdge, int width, int height)
{
	int lineNum;  // 线段数量
	int i;  // 循环变量
	CLine *pLine;  // 线段指针
	CRect rectEdge, rectArea;  // 矩形变量
	CPoint point1;  // 点变量

	// 初始化rectEdge为区域边界点
	lineNum = edgeLineArray.GetSize();
	for(i=0; i<lineNum; i++)
	{
		pLine = edgeLineArray.GetAt(i);
		if(pLine->m_groupID == groupID)
		{
			point1 = pLine->m_pointArray.GetAt(0);
			rectEdge.left = rectEdge.right = point1.x;
			rectEdge.top = rectEdge.bottom = point1.y;
			break;
		}
	}
	
	// 获取包含区域边界的最小矩形
	for(i=0; i<lineNum; i++)
	{
		pLine = edgeLineArray.GetAt(i);
		if(pLine->m_groupID == groupID)
		{
			GetSmallRectOfLine(pLine, rectEdge);
		}
	}

	// 初始化rectArea为区域点point
	rectArea.left = rectArea.right = point.x;
	rectArea.top = rectArea.bottom = point.y;

	// 获取包含区域的最小矩形
	GetSmallRectOfArea(point, pUnchEdge, width, height, rectArea);

	// 包含区域的矩形大于包含区域边界的矩形,即已验证的区域点在已验证的边界点外
	if(rectArea.left<rectEdge.left || rectArea.top<rectEdge.top || rectArea.right>rectEdge.right
		|| rectArea.bottom>rectEdge.bottom)
	{
		return FALSE;
	}

	return TRUE;
}

// 获取包含线段的最小矩形
void CDibObject::GetSmallRectOfLine(CLine *pLine, CRect &rect)
{
	int pointNum;  // 点数量
	int i;  // 循环变量
	CPoint point;  // 点变量

	pointNum = pLine->m_pointArray.GetSize();
	for(i=0; i<pointNum; i++)
	{
		point = pLine->m_pointArray.GetAt(i);
		if(point.x < rect.left)
		{
			rect.left = point.x;
		}
		if(point.y < rect.top)
		{
			rect.top = point.y;
		}
		if(point.x > rect.right)
		{
			rect.right = point.x;
		}
		if(point.y > rect.bottom)
		{
			rect.bottom = point.y;
		}
	}
}

// 获取包含区域的最小矩形
void CDibObject::GetSmallRectOfArea(CPoint point, unsigned char *pUnchEdge, int width,
									int height, CRect &rect)
{
	// 对8邻域象素进行查询
	static int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1} ;
	static int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1} ;
	int gray;  // 灰度值
	CArray<CPoint, CPoint> pointArray;  // 用于存放点的数组
	CPoint point1, point2;  // 点变量
	int i, x, y;  // 循环变量
	int nPos;  // 数组索引

	// 获取灰度值
	gray = pUnchEdge[point.x + point.y*width];

	// 初始化点数组
	pUnchEdge[point.x + point.y*width] = 8;
	pointArray.Add(point);
	while(pointArray.GetSize() != 0)
	{
		point1 = pointArray.GetAt(0);
		pointArray.RemoveAt(0);
		for(i=0; i<8; i++)
		{
			point2.x = point1.x + xNb[i];
			point2.y = point1.y + yNb[i];
			if(point2.x>=0 && point2.x<width && point2.y>=0 &&point2.y<height
				&& pUnchEdge[point2.x + point2.y*width] == gray)
			{
				pUnchEdge[point2.x + point2.y*width] = 8;
				pointArray.InsertAt(0, point2);
				if(point2.x < rect.left)
				{
					rect.left = point2.x;
				}
				if(point2.y < rect.top)
				{
					rect.top = point2.y;
				}
				if(point2.x > rect.right)
				{
					rect.right = point2.x;
				}
				if(point2.y > rect.bottom)
				{
					rect.bottom = point2.y;
				}
			}
		}
	}

	// 还原pUnchEdge为原来的值
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x + y*width;
			if(pUnchEdge[nPos] == 8)
			{
				pUnchEdge[nPos] = gray;
			}
		}
	}
}

// 去除区域及边界
void CDibObject::RemoveAreaAndEdgeByPoint(CPoint point, unsigned char *pUnchEdge,
										  int width, int height, int gray)
{
	// 对8邻域象素进行查询
	static int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1} ;
	static int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1} ;
	CArray<CPoint, CPoint> pointArray;  // 用于存放点的数组
	CPoint point1, point2;  // 点变量
	int i;  // 循环变量
	int nPos;  // 数组索引

	pUnchEdge[point.x + point.y*width] = 255;
	// 初始化点数组
	pointArray.Add(point);
	while(pointArray.GetSize() != 0)
	{
		point1 = pointArray.GetAt(0);
		pointArray.RemoveAt(0);
		for(i=0; i<8; i++)
		{
			point2.x = point1.x + xNb[i];
			point2.y = point1.y + yNb[i];
			nPos = point2.x + point2.y*width;
			if(point2.x>=0 && point2.x<width && point2.y>=0 && point2.y<height
				&&pUnchEdge[nPos] <= gray)
			{
				pUnchEdge[nPos] = 255;
				pointArray.InsertAt(0, point2);
			}
		}
	}
}

// 调试用
// 画出找到的直线
void CDibObject::PaintFindLine(CArray<CFindedLine,CFindedLine> &findedLineArray)
{
	int findedLineNum;                // 找到的直线的数量
	int i,j;                          // 循环变量
	CFindedLine findedLine;           // 直线变量
	CPoint startPoint,endPoint;       // 直线前后端点
	CArray<CPoint,CPoint> linePoints; // 用于记录直线上前后端点间的点
	int pointNum;                     // 直线上前后端点间的点的个数
	CPoint point;                     // 点变量

	findedLineNum=findedLineArray.GetSize();
	for(i=0;i<findedLineNum;i++)
	{
		findedLine=findedLineArray.GetAt(i);
		startPoint=findedLine.startPoint;
		endPoint=findedLine.endPoint;

		linePoints.RemoveAll();
		GetLinePoints(startPoint,endPoint,linePoints);
		pointNum=linePoints.GetSize();
		for(j=0;j<pointNum;j++)
		{
			point=linePoints.GetAt(j);
			pgUnchEdge[point.x+point.y*ngWidth]=0;
		}
	}
}
// 调试完
// 通过边界对区域进行验证是否是湖泊
BOOL CDibObject::CheckLakeByEdge(int groupID, CArray<CLine*, CLine*> &edgeLineArray)
{
	int lineNum;                                      // 线段数量
	int i;                                            // 循环变量
	CLine *pLine;                                     // 线段指针变量
	double edgeLineLen = 0;                             // 边界线长度
	CArray<CPoint,CPoint> cornerArray;                // 用于存放角点
	CArray<CFindedLine,CFindedLine> findedLineArray;  // 用于存放找到的直线
	CArray<CDegree,CDegree> degreeArray;              // 用于存放两直线之间的夹角

	lineNum = edgeLineArray.GetSize();
	for(i=0; i<lineNum; i++)
	{
		pLine = edgeLineArray.GetAt(i);
		if(pLine->m_groupID == groupID)
		{
			edgeLineLen += pLine->m_lineLength;
			// 求出此条线段上的角点存放于cornerArray数组中
			GetLineCorner(pLine,cornerArray);
			// 通过角点寻找直线
			FindLine(cornerArray,findedLineArray,degreeArray);
		}
	}

	// 调试用
	//if(groupID == 4)
	//PaintFindLine(findedLineArray);
	// 调试完

	// 通过直线长度与边界线长度的比值检验
	if(!CheckByLineLenRate(findedLineArray,edgeLineLen))
		return FALSE;

	// 通过平行线检验
	if(!CheckByParallelLine(findedLineArray,edgeLineLen))
		return FALSE;

	// 通过寻找到的直线夹角检验
	if(!CheckByDegree(degreeArray))
		return FALSE;

	return TRUE;
}

// 求取线段上的角点并存放于cornerArray数组中
void CDibObject::GetLineCorner(CLine* pLine,CArray<CPoint,CPoint> &cornerArray)
{
	static int stepLen=6;               // 检测步长
	static double minDegreeFaZhi=157;   // 最小角阀值
	int pointNum;                       // 线段所包含的点的数量
	int i;                              // 循环变量
	CPoint point1,point,point2;         // 前中后点变量
	double degree;                      // 角的度数

	// 清空cornerArray数组
	cornerArray.RemoveAll();
	pointNum=pLine->m_pointArray.GetSize();
	
	// 如果线段太短,则不验证
	if(pointNum<2*stepLen+1)
		return;

	// 线段前端点当作角点入cornerArray数组
	cornerArray.Add(pLine->m_pointArray.GetAt(0));

	pointNum-=stepLen;
	for(i=stepLen;i<pointNum;i++)
	{
		// 获得前中后三点
		point1=pLine->m_pointArray.GetAt(i-stepLen);
		point=pLine->m_pointArray.GetAt(i);
		point2=pLine->m_pointArray.GetAt(i+stepLen);

		// 获得由中前点所在矢量与中后点所在矢量的夹角
		degree=GetDirCha(point1.x-point.x,point1.y-point.y,point2.x-point.x,point2.y-point.y)*180/PI;

		// 获取角点两端开始点
		if(degree<minDegreeFaZhi)
		{
			// 保存角点前端开始点
			cornerArray.Add(point);
			i++;
			while(i<pointNum)
			{
				point1=pLine->m_pointArray.GetAt(i-stepLen);
				point=pLine->m_pointArray.GetAt(i);
				point2=pLine->m_pointArray.GetAt(i+stepLen);
				
				// 获得由中前点所在矢量与中后点所在矢量的夹角
				degree=GetDirCha(point1.x-point.x,point1.y-point.y,point2.x-point.x,point2.y-point.y)*180/PI;
				if(degree>minDegreeFaZhi)
					break;
				i++;
			}
			// 保存角点后端开始点
			cornerArray.Add(point);
		}
	}

	// 线段后端点当作角点入cornerArray数组
	cornerArray.Add(pLine->m_pointArray.GetAt(pointNum+stepLen-1));
}

/*************************************************************************
通过角点寻找直线
最小二乘法直线拟合:
   直线:Ax+By+C=0;
   求系数A:            A=(∑xi∑yi-n∑xi*yi) / (n∑xi*xi-(∑xi)*(∑xi))
   求系数B:            B=1
   求系数C:            C=(∑xi*yi∑xi-∑yi∑xi*xi) / (n∑xi*xi-(∑xi)*(∑xi))
   求线性相关系数r:    r=∑((xi-ax)(yi-ay)) / sqrt(∑((xi-ax)*(xi-ax)) * ∑((yi-ay)*(yi-ay)))
   若fabs(r)趋近于1,则x,y间线性关系好,fabs(r)趋近0,则x,y间无线性关系
**************************************************************************/
void CDibObject::FindLine(CArray<CPoint,CPoint> &cornerArray,CArray<CFindedLine,CFindedLine> &findedLineArray,
			  CArray<CDegree,CDegree> &degreeArray)
{
	static double lowPointLineJuLi=2;    // 点到直线的最小距离阀值
	static double lowLen=20;             // 所寻找到的直线最小长度
	int cornerNum;                       // 角点数量
	int i;                               // 循环变量
	CPoint point,point1;                 // 点变量
	double sumX,sumY,sumXY,sumXX;        // x、y、xy、xx之和
	int n;                               // 参与拟合直线的点的个数
	double temp,temp1,temp2;             // 存放临时计算数值
	double A,B,C;                        // 直线系数
	double pointLineJuLi;                // 点到直线的距离
	int index;                           // 数组索引
	double juLi;                         // 两点之间的距离
	CFindedLine findedLine1,findedLine2; // 寻找到的直线
	double degree;                       // 直线与x轴夹角
	int findedLineNum=0;                 // 找到的直线条数
	CDegree linesDegree;            // CDegree变量

	cornerNum=cornerArray.GetSize()-1;
	for(i=0;i<cornerNum;)
	{
		point=cornerArray.GetAt(i);
		sumX=point.x;
		sumY=point.y;
		sumXY=point.x*point.y;
		sumXX=point.x*point.x;

		n=2;
		point=cornerArray.GetAt(i+1);
		sumX+=point.x;
		sumY+=point.y;
		sumXY+=point.x*point.y;
		sumXX+=point.x*point.x;

		// 计算直线系数
		temp=n*sumXX-sumX*sumX;
		temp1=sumX*sumY-n*sumXY;
		temp2=sumXY*sumX-sumY*sumXX;
		if(temp==0)
		{
			A=1;
			B=0;
			C=-point.x;
		}
		else
		{
			A=temp1/temp;
			B=1;
			C=temp2/temp;
		}
		
		// 扩展用于拟合直线的点
		while(TRUE)
		{
			index=i+n;
			if(index>cornerNum) // 如果角点数组到达最后一个角点,则退出循环
				break;

			point=cornerArray.GetAt(index);
			// 得到点到直线的距离
			pointLineJuLi=GetPointLineJuLi(A,B,C,point);

			// 如果点到直线的距离小于阀值,则认为该点在直线上,将该点加入用于拟合的点
			if(pointLineJuLi<lowPointLineJuLi)
			{
				n++;

				sumX+=point.x;
				sumY+=point.y;
				sumXY+=point.x*point.y;
				sumXX+=point.x*point.x;
				
				// 计算直线系数
				temp=n*sumXX-sumX*sumX;
				temp1=sumX*sumY-n*sumXY;
				temp2=sumXY*sumX-sumY*sumXX;
				if(temp==0)
				{
					A=1;
					B=0;
					C=-point.x;
				}
				else
				{
					A=temp1/temp;
					B=1;
					C=temp2/temp;
				}
			}
			// 如果点到直线的距离大于阀值,则用于拟合的点扩展结束,退出循环
			else
			{
				break;
			}
		}

		// 找到直线,保存该直线
		// 取得前端点
		point=cornerArray.GetAt(i);
		

		// 取得后端点
		i=i+n-1;
		point1=cornerArray.GetAt(i);

		// 得到两点之间的距离
		juLi=GetJuLi(point,point1);
		// 如果此直线长度大于阀值,则保存该直线
		if(juLi>lowLen)
		{
			findedLine1.startPoint=GetTouYingPoint(A,B,C,point);
			findedLine1.endPoint=GetTouYingPoint(A,B,C,point1);
			findedLine1.length=juLi;
			
			// 取得直线与x轴夹角
			if(B==0)
				degree=90;
			else
			{
				degree=atan(-A/B)*180/PI;
				if(degree<0)
					degree+=180;
			}

			findedLine1.degree=degree;

			// 如果前面已经找到过直线,则取出那条直线求两直线夹角
			if(findedLineNum)
			{
				findedLine2=findedLineArray.GetAt(findedLineArray.GetSize()-1);
				degree=GetDirCha(findedLine1.endPoint.x-findedLine1.startPoint.x,findedLine1.endPoint.y-findedLine1.startPoint.y,
					findedLine2.startPoint.x-findedLine2.endPoint.x,findedLine2.startPoint.y-findedLine2.endPoint.y)*180/PI;
				linesDegree.findedLine1=findedLine2;
				linesDegree.degree=degree;
				linesDegree.findedLine2=findedLine1;
				degreeArray.Add(linesDegree);
			}
			
			// 将该直线存入数组findedLineArray
			findedLineArray.Add(findedLine1);
			findedLineNum++;
		}
	}
}

// 通过直线长度与边界线长度的比值检验
BOOL CDibObject::CheckByLineLenRate(CArray<CFindedLine,CFindedLine> &findedLineArray,double edgeLineLen)
{
	static double higeLineLenRate=0.4;    // 直线长度在边界线长度中所占的最大比值
	int findedLineNum;                    // 寻找到的直线数量
	int i;                                // 循环变量
	double lineLen=0;                     // 直线总长度

	// 求取直线长度
	findedLineNum=findedLineArray.GetSize();
	for(i=0;i<findedLineNum;i++)
	{
		lineLen+=findedLineArray.GetAt(i).length;
	}

	// 如果直线长度与边界线长度的比值超过阀值则去除区域
	if(edgeLineLen==0 || lineLen/edgeLineLen>higeLineLenRate)
		return FALSE;

	return TRUE;
}

// 通过平行线检验
BOOL CDibObject::CheckByParallelLine(CArray<CFindedLine,CFindedLine> &findedLineArray,double edgeLineLen)
{
	static double lowDegree=4;           // 判为平行线夹角的最低阀值
	static double lowValue=0.15;         // 权值最低阀值
	int findedLineNum;                   // 寻找到的直线数量
	int i,j;                             // 循环变量
	CFindedLine findedLine1,findedLine2; // 寻找到的直线变量
	double degree;                       // 两直线夹角
	int parallelLineNum=0;               // 平行线数量
	double parallelLineLen=0;            // 平行线长度
	double quanZhi1=0.5,quanZhi2=0.25;   // 权值向量
	double value;                        // 权值

	if(edgeLineLen==0)
		return FALSE;

	findedLineNum=findedLineArray.GetSize();
	// 取得平行线长度
	for(i=0;i<findedLineNum;i++)
	{
		findedLine1=findedLineArray.GetAt(i);
		for(j=0;j<findedLineNum;j++)
		{
			if(i!=j)
			{
				findedLine2=findedLineArray.GetAt(j);
				degree=findedLine1.degree>findedLine2.degree?findedLine1.degree-findedLine2.degree:
				findedLine2.degree-findedLine1.degree;
				degree=degree<90?degree:180-degree;
				if(degree<lowDegree)
				{
					parallelLineLen+=findedLine1.length;
					break;
				}
			}
		}
	}

	// 取得平行线数量
	for(i=0;i<findedLineNum;i++)
	{
		findedLine1=findedLineArray.GetAt(i);
		for(j=0;j<findedLineNum;j++)
		{
			if(i!=j)
			{
				findedLine2=findedLineArray.GetAt(j);
				degree=findedLine1.degree>findedLine2.degree?findedLine1.degree-findedLine2.degree:
				findedLine2.degree-findedLine1.degree;
				degree=degree<90?degree:180-degree;
				if(degree<lowDegree)
				{
					parallelLineNum++;
				}
			}
		}
	}

	// 如果权值大于阀值,则去除区域
	value=quanZhi1*parallelLineLen/edgeLineLen+quanZhi2*parallelLineNum/findedLineNum;
	if(value>lowValue)
		return FALSE;

	return TRUE;
}

// 通过寻找到的直线夹角检验
BOOL CDibObject::CheckByDegree(CArray<CDegree,CDegree> &degreeArray)
{
	static double lowDegreeFangCha=0;  // 方差阀值
	static int stepLen=10;             // 步长
	static double highNumRate=2;       // 数量比值阀值
	int degreeNum;                     // 夹角数量
	int i;                             // 循环变量
	CDegree degree;                    // 夹角变量
	double aveDegree=0;                // 夹角度数之和
	double degreeFangCha=0;            // 夹角方差
	int degreeAnalyseArray[18];        // 角度个数统计数组
	int halfStepLen=stepLen/2;         // 半个步长
	int maxNum;                        // 数量最大值
	int index;                         // 数组索引
	int number;                        // 参与统计的夹角数量

	degreeNum=degreeArray.GetSize();
	if(degreeNum==0)
		return TRUE;

	// 求夹角平均值
	for(i=0;i<degreeNum;i++)
	{
		degree=degreeArray.GetAt(i);
		aveDegree+=degree.degree;
	}
	aveDegree/=degreeNum;

	// 求夹角方差
	for(i=0;i<degreeNum;i++)
	{
		degree=degreeArray.GetAt(i);
		degreeFangCha+=fabs(degree.degree-aveDegree);
	}
	degreeFangCha/=degreeNum;

	// 如果角度方差小于阀值,则去除区域
	if(degreeFangCha<lowDegreeFangCha)
		return FALSE;

	// 将degreeAnalyseArray数组值全部置为0
	memset(degreeAnalyseArray,0,18*4);
	// 统计角度
	for(i=0;i<degreeNum;i++)
	{
		degree=degreeArray.GetAt(i);
		degreeAnalyseArray[int((degree.degree+halfStepLen)/stepLen)]++;
	}
	
	// 在度数相近的夹角组中,找出数量最多的一组
	maxNum=0;
	index=0;
	for(i=0;i<18;i++)
	{
		if(maxNum<degreeAnalyseArray[i])
		{
			maxNum=degreeAnalyseArray[i];
			index=i;
		}
	}
	degreeAnalyseArray[index]=0;

	// 在度数相近的夹角组中,找出数量次多的一组
	number=0;
	index=-1;
	for(i=0;i<18;i++)
	{
		if(number<degreeAnalyseArray[i])
		{
			number=degreeAnalyseArray[i];
			index=i;
		}
	}

	number+=maxNum;
	// 如果角度过于集中,则去除区域
	if(((double)number)/degreeNum>highNumRate)
		return FALSE;

	return TRUE;
}

/*************************************************************************
得到点到直线的距离
直线为:Ax+By+C=0     点为:P(x0,y0)
**************************************************************************/
double CDibObject::GetPointLineJuLi(double A,double B,double C,CPoint point)
{
	return fabs(A*point.x+B*point.y+C)/sqrt(A*A+B*B);
}

/*************************************************************************
得到一个点在直线上的投影
直线为:Ax+By+C=0     点为:P(x0,y0)
**************************************************************************/
CPoint CDibObject::GetTouYingPoint(double A,double B,double C,CPoint point)
{
	CPoint touYingPoint;      // 投影点

	if(A==0)
	{
		touYingPoint.x=point.x;
		touYingPoint.y=int(-C/B+0.5);	
	}
	else if(B==0)
	{
		touYingPoint.x=int(-C/A+0.5);
		touYingPoint.y=point.y;
	}
	else
	{
		touYingPoint.x=int(0.5*point.x-B*point.y/(2*A)-C/(2*A)+0.5);
		touYingPoint.y=int(0.5*point.y-A*point.x/(2*B)-C/(2*B)+0.5);
	}

	return touYingPoint;
}

// 交互道路提取
BOOL CDibObject::InteractRoadExtract(CPoint roadSeed1, CPoint roadSeed2, CDibObject *pDibObject)
{
	static int N = 7;               // N领域范围半径，与GetRoadEdgePoint中一致
	static int M = 3 * N;           // M领域范围半径
	static double highFangCha = 13;  // 最大方差值
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	// 线段两端点M、N领域内的平均灰度
	double point1AveGrayN, point1AveGrayM, point2AveGrayN, point2AveGrayM;
	CPoint grayestPoint1, grayerPoint1, grayestPoint2, grayerPoint2;  // 灰度最大点、次大点
	CPoint whitestPoint1, whiterPoint1, whitestPoint2, whiterPoint2;  // 亮度最大点、次大点
	CArray<CPoint, CPoint> roadPointArray;  // 记录道路上的点
	double aveGray, grayFangCha;  // 平均灰度和灰度方差

	if(!CheckRoadSeed(roadSeed1, roadSeed2))
	{
		return FALSE;
	}

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		AfxMessageBox("选取种子点错误—位置!");
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	// 将pBits所指向位图的位图数据复制到pUnchSmooth所指向的位图
	CopyBitmap(pUnchSmooth, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	point1AveGrayN=GetAveGray(roadSeed1, pUnchSmooth, m_nWidth, m_nHeight, N);
	point1AveGrayM=GetAveGray(roadSeed1, pUnchSmooth, m_nWidth, m_nHeight, M);
	point2AveGrayN=GetAveGray(roadSeed2, pUnchSmooth, m_nWidth, m_nHeight, N);
	point2AveGrayM=GetAveGray(roadSeed2, pUnchSmooth, m_nWidth, m_nHeight, M);
	
	if(point1AveGrayN > point1AveGrayM && point2AveGrayN > point2AveGrayM)
	{
		GetGrayestPoint(roadSeed1, roadSeed2, pUnchSmooth, m_nWidth, m_nHeight, grayestPoint1, grayerPoint1);
		GetGrayestPoint(roadSeed2, roadSeed1, pUnchSmooth, m_nWidth, m_nHeight, grayestPoint2, grayerPoint2);
		// 取得两点之间的点
		GetLinePoints(grayestPoint1, grayestPoint2, roadPointArray);
		// 调用函数AnalyseGray获取平均灰度和灰度方差
		AnalyseGray(pUnchSmooth, m_nWidth, roadPointArray, aveGray, grayFangCha);
		if(grayFangCha > highFangCha)
		{
			AfxMessageBox("选取种子点错误—非道路边缘!");
			return FALSE;
		}
		roadPointArray.RemoveAll();
		TrackGrayRoadByEdgePoint(roadSeed1, roadSeed2, pUnchSmooth, m_nWidth, m_nHeight, 68, roadPointArray);
	}
	else if(point1AveGrayN < point1AveGrayM && point2AveGrayN < point2AveGrayM)
	{
		GetGrayestPoint(roadSeed1, roadSeed2, pUnchSmooth, m_nWidth, m_nHeight, whitestPoint1,whiterPoint1);
		GetGrayestPoint(roadSeed2, roadSeed1, pUnchSmooth, m_nWidth, m_nHeight, whitestPoint2,whiterPoint2);
		// 取得两点之间的点
		GetLinePoints(whitestPoint1, whitestPoint2, roadPointArray);
		// 调用函数AnalyseGray获取平均灰度和灰度方差
		AnalyseGray(pUnchSmooth, m_nWidth, roadPointArray, aveGray, grayFangCha);
		if(grayFangCha > highFangCha)
		{
			AfxMessageBox("选取种子点错误—非道路边缘!");
			return FALSE;
		}
		roadPointArray.RemoveAll();
		TrackWhiteRoadByEdgePoint(roadSeed1, roadSeed2, pUnchSmooth, m_nWidth, m_nHeight, 68, roadPointArray);
	}
	else
	{
		AfxMessageBox("选取种子点错误—非道路边缘!");
	}

	// 清除位图pUnchSmooth中数据
	ClearBitmap(pUnchSmooth, m_nWidth, m_nHeight);

	//交互提取的道路信息存放在txt文件中
	//RecordPoints(roadPointArray);

	// 画出单条道路
	PaintOneRoad(pUnchSmooth, m_nWidth, roadPointArray);

	//交互提取的道路信息放在txt文件中
	RecordRoadPoints(pUnchSmooth, m_nWidth, m_nHeight);

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);
	CopyBitmapPixel(pBits, widthBytes, m_nHeight, pUnchSmooth, m_nWidth, m_nHeight);
	
	::GlobalUnlock(pDibObject->m_hDib);
	::GlobalUnlock(m_hDib);
	delete []pUnchSmooth;
	return TRUE;
}

// 验证道路种子点是否满足要求
BOOL CDibObject::CheckRoadSeed(CPoint roadSeed1, CPoint roadSeed2)
{
	if(roadSeed1 == roadSeed2)
	{
		return FALSE;
	}
	if(roadSeed1.x<0 || roadSeed1.x>=m_nWidth || roadSeed1.y<0 || roadSeed1.y>=m_nHeight)
	{
		return FALSE;
	}
	if(roadSeed2.x<0 || roadSeed2.x>=m_nWidth || roadSeed2.y<0 || roadSeed2.y>=m_nHeight)
	{
		return FALSE;
	}

	return TRUE;
}

// 水体交互提取
BOOL CDibObject::InteractLakeExtract(CPoint lakeSeed, CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchEdge;  // 存放提取出的边界
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度
	CArray<CLine*,CLine*> edgeLineArray;  // 道路边界线数组

	if(!CheckLakeSeed(lakeSeed))
	{
		return FALSE;
	}

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];
	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];
	// 对原图象进行高斯滤波
	GaussianSmooth(pUnchEdge, m_nWidth, m_nHeight, sigma, pUnchSmooth);

	pnGradX = new int[m_nWidth * m_nHeight];
	pnGradY = new int[m_nWidth * m_nHeight];
	// 计算方向导数
	DirGrad(pUnchSmooth, m_nWidth, m_nHeight, pnGradX, pnGradY);

	pnGradMag = new int[m_nWidth * m_nHeight];
	// 计算梯度的幅度
	GradMagnitude(pnGradX, pnGradY, m_nWidth, m_nHeight, pnGradMag);

	// 应用non-maximum 抑制
	NonmaxSuppress(pnGradMag, pnGradX, pnGradY, m_nWidth, m_nHeight, pUnchEdge);

	// 应用Hysteresis，找到所有的边界
	Hysteresis(pnGradMag, m_nWidth, m_nHeight, dRatioLow, dRatioHigh, pUnchEdge);

	// 获取边界线记录于变量edgeLineArray中
	GetEdgeLine(pUnchEdge, m_nWidth, m_nHeight, 0, edgeLineArray);

	// 对线段两端点进行延伸，以填补小缺口
	ExtendLine(edgeLineArray, pUnchEdge, m_nWidth, m_nHeight);

	// 按给定点扩展湖泊区域
	FillByPoint(pUnchEdge, m_nWidth, m_nHeight, lakeSeed);

	
	// 去除边界点或通过验证的区域,边界点灰度为0,通过验证的区域点灰度为1,
	// 通过验证的边界点灰度值为2,可自由选择
	RemoveEdge(pUnchEdge, m_nWidth, m_nHeight, 0);

	//获取水体的边界点记录于txt文件上
	RecordWaterEdgePoints(pUnchEdge, m_nWidth, m_nHeight);
	//获取水体区域点记录于txt文件上
	RecordWaterPoints(pUnchEdge, m_nWidth, m_nHeight);

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	// 将pUnchEdge所指向的位图数据复制到pBits所指向位图的位图
	CopyBitmapPixel(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);
	
	::GlobalUnlock(pDibObject->m_hDib);

	// 清除edgeLineArray数组
	ClearLineArray(edgeLineArray);
	delete []pnGradMag;
	delete []pnGradY;
	delete []pnGradX;
	delete []pUnchSmooth;
	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);

	return TRUE;
}

// 水体交互提取
BOOL CDibObject::InteractAllLakeExtract(CPoint lakeSeed, CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchEdge;  // 存放提取出的边界
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度
	CArray<CLine*,CLine*> edgeLineArray;  // 道路边界线数组

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];
	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];
	// 对原图象进行高斯滤波
	memset(pUnchSmooth,255,m_nWidth * m_nHeight);
	
	// 按给定点扩展湖泊区域
	FillByPoint1(pUnchEdge, pUnchSmooth, m_nWidth, m_nHeight, lakeSeed);

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	// 将pUnchEdge所指向的位图数据复制到pBits所指向位图的位图
	CopyBitmap(pBits, widthBytes, m_nHeight, pUnchSmooth, m_nWidth, m_nHeight);
	
	::GlobalUnlock(pDibObject->m_hDib);

	// 清除edgeLineArray数组
	ClearLineArray(edgeLineArray);
	delete []pUnchSmooth;
	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);

	return TRUE;
}

// 验证湖泊种子点是否满足要求
BOOL CDibObject::CheckLakeSeed(CPoint lakeSeed)
{
	if(lakeSeed.x<0 || lakeSeed.x>=m_nWidth || lakeSeed.y<0 || lakeSeed.y>=m_nHeight)
	{
		return FALSE;
	}

	return TRUE;
}

// 画多边形
BOOL CDibObject::DrawPolygon(CArray<CPoint,CPoint> &polygonPoints)
{
	// 验证多边形的点所形成的区域是否是有效区域
	if(polygonPoints.GetSize() < 3)
	{
		AfxMessageBox("点的个数不足以构成多边形!");
		return FALSE;
	}

	if(!CheckPolygonPoints(polygonPoints))
		return FALSE;

	CPoint point1, point2, point;  // 点变量
	CArray<CPoint,CPoint> pointArray;  // 存放点的数组

	point1 = polygonPoints.GetAt(0);
	// 将第一个点加到数组末尾,形成闭合多边形
	polygonPoints.Add(point1);

	// 画多边形边界线
	DrawPolygonLine(polygonPoints);

	//获取手工提取的水体边界点，记录于txt文件中
	CArray<CPoint,CPoint> multiPoints;
	GetMultiPoints(polygonPoints,multiPoints);
	RecordPoints_wateredge_human(multiPoints);

	point2 = polygonPoints.GetAt(1);
	point2.x = int((double(point1.x + point2.x))/2 + 0.5);
	point2.y = int((double(point1.y + point2.y))/2 + 0.5);

	// 获取线段两侧的两个点(pointArray[0]、pointArray[pointArray.GetSize() - 1])
	GetParePoints(point2, point1, 3, m_nWidth, m_nHeight, pointArray);

	// 获取多边形内部种子点
	point = pointArray.GetAt(0);
	if(InsidePolygon(point, polygonPoints));
	else
	{
		point = pointArray.GetAt(pointArray.GetSize() - 1);
		if(!InsidePolygon(point, polygonPoints))
			return FALSE;
	}

	// 种子点填充算法填充多边形
	FillArea(point);

	return TRUE;
}

// 验证多边形的点所形成的区域是否是有效区域
BOOL CDibObject::CheckPolygonPoints(CArray<CPoint,CPoint> &polygonPoints)
{
	int pointNum;  // 点的数量
	int i;  // 循环变量
	CPoint point;  // 点变量
	
	pointNum = polygonPoints.GetSize();
	for(i=0; i<pointNum; i++)
	{
		point = polygonPoints.GetAt(i);
		if(point.x<0 || point.x>m_nWidth || point.y<0 || point.y>m_nHeight)
		{
			AfxMessageBox("选取点错误—超出图片边界!");
			return FALSE;
		}
	}

	return TRUE;
}

void CDibObject::GetMultiPoints(CArray<CPoint,CPoint> &polygonPoints,CArray<CPoint,CPoint> &pointArray)
{

	int widthBytes;  // 图像字节宽度
	int pointNum, pointNum1;  // 点数量
	CPoint point1, point2, point;  // 点变量
	int i, j;  // 循环变量
	// 获取分割后的图像文件头指针

	pointNum = polygonPoints.GetSize();
	point1 = polygonPoints.GetAt(0);
	for(i=1; i<pointNum; i++)
	{
		point2 = polygonPoints.GetAt(i);
		GetLinePoints(point1, point2, pointArray);
		pointNum1 = pointArray.GetSize();
		for(j=0; j<pointNum1; j++)
		{
			point = pointArray.GetAt(j);
		}
		point1 = point2;
	}
}
// 画多边形边界
void CDibObject::DrawPolygonLine(CArray<CPoint,CPoint> &polygonPoints)
{
	unsigned char *pBits;  // 图像位图数据起始指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像字节宽度
	int pointNum, pointNum1;  // 点数量
	CPoint point1, point2, point;  // 点变量
	int i, j;  // 循环变量
	CArray<CPoint,CPoint> pointArray;  // 存放点的数组

	pBits = (unsigned char *)::GlobalLock( m_hDib );
	if( pBits == NULL ) return;

	// 获取分割后的图像文件头指针
	pBFH = (BITMAPFILEHEADER *) pBits;
	pBits += pBFH->bfOffBits;

	// 获取图像字节宽度
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	// 画多边形边界线
	pointNum = polygonPoints.GetSize();
	point1 = polygonPoints.GetAt(0);
	for(i=1; i<pointNum; i++)
	{
		point2 = polygonPoints.GetAt(i);
		GetLinePoints(point1, point2, pointArray);
		pointNum1 = pointArray.GetSize();
		for(j=0; j<pointNum1; j++)
		{
			point = pointArray.GetAt(j);
			pBits[point.x + point.y*widthBytes] = 2;
		}
		point1 = point2;
	}
//	RecordPoints(pointArray);//生成手工画道路的txt文件

/*	polygonPoints.RemoveAll();
	for(i=0;i<pointArray.GetSize();i++)
	{
		polygonPoints.Add(pointArray.GetAt(i));
	}*/
	::GlobalUnlock( m_hDib );
}

// 判断一个点是否在多边形内部
BOOL CDibObject::InsidePolygon(CPoint point, CArray<CPoint,CPoint> &polygonPoints)
{    
    CPoint point1,point2;  // 点变量
	int pointNum;  // 点数量
	int i;  // 循环变量
	double xinters;  // 交点横坐标
	int counter=0;  // 交点次数

	pointNum = polygonPoints.GetSize();
    point1 = polygonPoints.GetAt(0);
    for(i=1; i<pointNum; i++)
	{
        point2 = polygonPoints.GetAt(i);
        if(point.y > min(point1.y, point2.y))
		{
            if(point.y <= max(point1.y, point2.y))
			{
                if(point.x < max(point1.x, point2.x))
				{
					xinters = int((double(point.y-point1.y)*(point2.x-point1.x))/(point2.y-point1.y)
						+ point1.x + 0.5);
					if(point1.x == point2.x || point.x < xinters) counter++;
                }
            }
        }
        point1 = point2;
    }

    if(counter%2 == 0)
        return false;

    return true;
}

// 种子点填充算法填充多边形
void CDibObject::FillArea(CPoint point)
{
	// 对4邻域象素进行查询
	static int xNb[4] = {1, 0,-1, 0} ;
	static int yNb[4] = {0, 1, 0,-1} ;
	unsigned char *pBits;  // 图像位图数据起始指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像字节宽度
	int nPos;  // // 数组索引
	CArray<CPoint,CPoint> pointArray;  // 存放点的数组
	CPoint pointA, pointB;  // 点变量
	int i;  // 循环变量

	pBits = (unsigned char *)::GlobalLock( m_hDib );
	if( pBits == NULL ) return;

	// 获取分割后的图像文件头指针
	pBFH = (BITMAPFILEHEADER *) pBits;
	pBits += pBFH->bfOffBits;

	// 获取图像字节宽度
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pBits[point.x + point.y*widthBytes] = 0;
	pointArray.Add(point);
	while(pointArray.GetSize())
	{
		pointA = pointArray.GetAt(0);
		pointArray.RemoveAt(0);
		for(i=0; i<4; i++)
		{
			pointB.x = pointA.x + xNb[i];
			pointB.y = pointA.y + yNb[i];
			nPos = pointB.x + pointB.y*widthBytes;
			if(pBits[nPos]!=0 && pBits[nPos]!=2)
			{
				pBits[nPos] = 0;
				pointArray.Add(pointB);
			}
		}
	}	
	RecordPoints_waterbody_human(pBits,m_nWidth,m_nHeight);//生成手工提取水体区域的txt文件

	::GlobalUnlock( m_hDib );
}

// 擦除
void CDibObject::Earser(CPoint point)
{
	unsigned char *pBits;  // 图像位图数据起始指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像字节宽度
	int x, y;  // 循环变量
	int nPos;  // // 数组索引

	if(point.x<3 || point.x>m_nWidth-4 || point.y<3 || point.y>m_nHeight-4)
		return;

	pBits = (unsigned char *)::GlobalLock( m_hDib );
	if( pBits == NULL ) return;

	// 获取分割后的图像文件头指针
	pBFH = (BITMAPFILEHEADER *) pBits;
	pBits += pBFH->bfOffBits;

	// 获取图像字节宽度
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	for(x=-6; x<=6; x++)
	{
		for(y=-6; y<=6; y++)
		{
			nPos = point.x + x + (point.y + y)*widthBytes;
			pBits[nPos] = 255;
		}
	}

	::GlobalUnlock( m_hDib );
}

// 擦除选中的区域
void CDibObject::EarseArea(CPoint point1, CPoint point2)
{
	int minX, maxX, minY, maxY;  // 擦除区域坐标点
	unsigned char *pBits;  // 图像位图数据起始指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像字节宽度
	int x, y;  // 循环变量

	minX = point1.x < point2.x? point1.x: point2.x;
	maxX = point1.x > point2.x? point1.x: point2.x;
	minY = point1.y < point2.y? point1.y: point2.y;
	maxY = point1.y > point2.y? point1.y: point2.y;

	minX = minX > 0? minX: 0;
	minX = minX < m_nWidth? minX: m_nWidth-1;
	maxX = maxX > 0? maxX: 0;
	maxX = maxX < m_nWidth? maxX: m_nWidth-1;
	minY = minY > 0? minY: 0;
	minY = minY < m_nHeight? minY: m_nHeight-1;
	maxY = maxY > 0? maxY: 0;
	maxY = maxY < m_nHeight? maxY: m_nHeight-1;

	pBits = (unsigned char *)::GlobalLock( m_hDib );
	if( pBits == NULL ) return;

	// 获取分割后的图像文件头指针
	pBFH = (BITMAPFILEHEADER *) pBits;
	pBits += pBFH->bfOffBits;

	// 获取图像字节宽度
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	for(x=minX; x<=maxX; x++)
	{
		for(y=minY; y<=maxY; y++)
		{
			pBits[x + y*widthBytes] = 255;
		}
	}

	::GlobalUnlock( m_hDib );
}

// 画线
void CDibObject::DrawLines(CArray<CPoint,CPoint> &linePoints)
{
	if(!CheckPolygonPoints(linePoints))
		return;
	DrawPolygonLine(linePoints);
}

// 调试用
// 画出所有道路边界线
void CDibObject::PaintAllRoadEdgeLine(unsigned char *pUnchEdge, int nWidth,
					CArray<CLine*,CLine*> &roadEdgeLineArray)
{
	int roadEdgeLineNum;
	int i;

	roadEdgeLineNum=roadEdgeLineArray.GetSize();
	for(i=0;i<roadEdgeLineNum;i++)
		PaintOneRoadEdgeLine(pUnchEdge,nWidth,roadEdgeLineArray,i);
}

// 按组画出道路边界线
void CDibObject::PaintRoadEdgeLineByGroup(unsigned char *pUnchEdge, int width,
		CArray<CLine*,CLine*> &roadEdgeLineArray, int groupID)
{
	int roadEdgeLineNum;
	CLine *pLine;
	int i;
	
	roadEdgeLineNum=roadEdgeLineArray.GetSize();
	for(i=0;i<roadEdgeLineNum;i++)
	{
		pLine = roadEdgeLineArray.GetAt(i);
		if(pLine->m_groupID == groupID)
			PaintOneRoadEdgeLine(pUnchEdge,width,roadEdgeLineArray,i);
	}
}

// 画出单条道路边界线
void CDibObject::PaintOneRoadEdgeLine(unsigned char *pUnchEdge, int nWidth,
					CArray<CLine*,CLine*> &roadEdgeLineArray, int index)
{
	CLine *pLine;
	int pointNum;
	int i;
	CPoint point;
	int nPos;

	pLine=roadEdgeLineArray.GetAt(index);
	pointNum=pLine->m_pointArray.GetSize();
	for(i=0;i<pointNum;i++)
	{
		point=pLine->m_pointArray.GetAt(i);
		nPos=point.x+point.y*nWidth;
		pUnchEdge[nPos]=0;
	}
}
void CDibObject::RecordPoints(CArray<CPoint,CPoint> &Points)
{
	CString filename;
	filename.Format("手工提取道路坐标%d.txt",m_H_r);
//	CFile file("RoadPointArray.txt",CFile::modeCreate | CFile::modeWrite);
	CFile file(filename,CFile::modeCreate | CFile::modeWrite);
	
	CString str;
	str.Format("该道路所包含的点为\r\n\r\n");
	file.Write(str,str.GetLength());
	int m_point,i;
	CPoint point;
	m_point=Points.GetSize();
	for(i=0;i<m_point;i++)
	{
		point=Points.GetAt(i);
		str.Format("(%d,%d)、",point.x,m_nHeight-1-point.y);
		file.Write(str,str.GetLength());
	}
	m_H_r++;
}
void CDibObject::RecordPoints_wateredge_human(CArray<CPoint,CPoint> &Points)
{

	CString filename;
	filename.Format("手工提取水体边界坐标%d.txt",m_H_we);
//	CFile file("RoadPointArray.txt",CFile::modeCreate | CFile::modeWrite);
	CFile file(filename,CFile::modeCreate | CFile::modeWrite);
	
	CString str;
	str.Format("该水体边缘线所包含的点为\r\n\r\n");
	file.Write(str,str.GetLength());
	int m_point,i;
	CPoint point;
	m_point=Points.GetSize();
	for(i=0;i<m_point;i++)
	{
		point=Points.GetAt(i);
		str.Format("(%d,%d)、",point.x,m_nHeight-1-point.y);
		file.Write(str,str.GetLength());
	}
	m_H_we++;
}
void CDibObject::RecordPoints_waterbody_human(unsigned char *pUnchEdge, int width, int height)
{
	
	CString filename;
	filename.Format("手工提取水体区域坐标%d.txt",m_H_wb);
	CFile file(filename,CFile::modeCreate | CFile::modeWrite);

	CString str;
	str.Format("人工提取水体区域的点为\r\n\r\n");
	file.Write(str,str.GetLength());

	int x, y;        // 循环变量
	int nPos;        // 数组索引

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x+y*width;
			if(pUnchEdge[nPos] == 0)
			{
				str.Format("(%d,%d)、",x,m_nHeight-1-y);
				file.Write(str,str.GetLength());
			}
		}
	}
	m_H_wb++;
}

void CDibObject::RecordWaterEdgePoints(unsigned char *pUnchEdge, int width, int height)
{
	CString filename;
	filename.Format("交互水体提取边界坐标%d.txt",m_I_we);
	CFile file(filename,CFile::modeCreate | CFile::modeWrite);

	CString str;
	str.Format("该水体的边界点为\r\n\r\n");
	file.Write(str,str.GetLength());

	int x, y;        // 循环变量
	int nPos;        // 数组索引

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x+y*width;
			if(pUnchEdge[nPos] == 2)
			{
				str.Format("(%d,%d)、",x,m_nHeight-1-y);
				file.Write(str,str.GetLength());
			}
		}
	}
	m_I_we++;

}
void CDibObject::RecordWaterPoints(unsigned char *pUnchEdge, int width, int height)
{
	CString filename;
	filename.Format("交互水体提取区域坐标%d.txt",m_I_wb);
	CFile file(filename,CFile::modeCreate | CFile::modeWrite);

	CString str;
	str.Format("所有水体区域的点为\r\n\r\n");
	file.Write(str,str.GetLength());

	int x, y;        // 循环变量
	int nPos;        // 数组索引

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x+y*width;
			if(pUnchEdge[nPos] == 1)
			{
				str.Format("(%d,%d)、",x,m_nHeight-1-y);
				file.Write(str,str.GetLength());
			}
		}
	}
	m_I_wb++;
}
void CDibObject::RecordRoadPoints(unsigned char *pUnchEdge, int width, int height)
{
	CString filename;
	filename.Format("交互道路提取结果%d.txt",m_I_r);
	CFile file(filename,CFile::modeCreate | CFile::modeWrite);

	CString str;
	str.Format("交互提取的道路点为\r\n\r\n");
	file.Write(str,str.GetLength());

	int x, y;        // 循环变量
	int nPos;        // 数组索引

	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			nPos = x+y*width;
			if(pUnchEdge[nPos] == 0)
			{
				str.Format("(%d,%d)、",x,m_nHeight-1-y);
				file.Write(str,str.GetLength());
			}
		}
	}
	m_I_r++;
}

void CDibObject::RecordRoadEdgeLineToText(CArray<CLine*,CLine*> &roadEdgeLineArray)
{
	CFile file("道路自动提取结果.txt",CFile::modeCreate | CFile::modeWrite);
	int roadEdgeLineNum,pointNum;
	CString str;
	int i,k;
	CLine *pLine;
	CPoint point;

	roadEdgeLineNum = roadEdgeLineArray.GetSize();
	str.Format("总道路边界线数量是：%d\r\n\r\n",roadEdgeLineNum);
	file.Write(str,str.GetLength());

	for(i=0;i<roadEdgeLineNum;i++)
	{
		pLine=roadEdgeLineArray.GetAt(i);
		str.Format("边界线%d(GroupID:%d):  线段长度:%f  平行线对数:%d  边界端点:%d\r\n\r\n点坐标为:",
			i,pLine->m_groupID,pLine->m_lineLength,pLine->m_parallels,pLine->m_edgeLines);
		file.Write(str,str.GetLength());
		pointNum=pLine->m_pointArray.GetSize();
		for(k=0;k<pointNum;k++)
		{
			point=pLine->m_pointArray.GetAt(k);
			str.Format("(%d,%d)、",point.x,m_nHeight-1-point.y);
			file.Write(str,str.GetLength());
		}
		file.Write("\r\n\r\n",4);
	}

	file.Close();
}

void CDibObject::RecordWaterEdgeLineToText(CArray<CLine*,CLine*> &waterEdgeLineArray)
{
	CFile file("水体自动提取结果.txt",CFile::modeCreate | CFile::modeWrite);
	int waterEdgeLineNum,pointNum;
	CString str;
	int i,k;
	CLine *pLine;
	CPoint point;

	waterEdgeLineNum = waterEdgeLineArray.GetSize();
	str.Format("总水体边界线数量是：%d\r\n\r\n",waterEdgeLineNum);
	file.Write(str,str.GetLength());

	for(i=0;i<waterEdgeLineNum;i++)
	{
		pLine=waterEdgeLineArray.GetAt(i);
		str.Format("边界线%d(GroupID:%d):  线段长度:%f  平行线对数:%d  边界端点:%d\r\n\r\n点坐标为:",
			i,pLine->m_groupID,pLine->m_lineLength,pLine->m_parallels,pLine->m_edgeLines);
		file.Write(str,str.GetLength());
		pointNum=pLine->m_pointArray.GetSize();
		for(k=0;k<pointNum;k++)
		{
			point=pLine->m_pointArray.GetAt(k);
			str.Format("(%d,%d)、",point.x,m_nHeight-1-point.y);
			file.Write(str,str.GetLength());
		}
		file.Write("\r\n\r\n",4);
	}

	file.Close();
}
// 将编组信息记录于文本中
void CDibObject::RecordGroupsToText(CGroup *pGroups, int lineNum)
{
	CFile file("groups.txt",CFile::modeCreate | CFile::modeWrite);
	int i;  // 循环变量
	CString str;

	for(i=0; i<lineNum; i++)
	{
		if(pGroups[i].m_groupID >= 0)
		{
			str.Format("分组%d信息如下:\r\n\r\n",pGroups[i].m_groupID);
			file.Write(str,str.GetLength());
			str.Format("线段总长度:    %f\r\n平行线数量:    %d\r\n边界端点数量:  %d\r\n",
				pGroups[i].m_lineLength,pGroups[i].m_parallels,pGroups[i].m_edgeLines);
			file.Write(str,str.GetLength());
			str.Format("该组置信度为:  %f\r\n",pGroups[i].m_beliefDegree);
			file.Write(str,str.GetLength());
			str.Format("是否被舍弃:    %s\r\n","是");
			if(pGroups[i].m_believed)
			{
				str.Format("是否被舍弃:    %s\r\n","否");
			}
			file.Write(str,str.GetLength());
			str.Format("种子点为:      点1(%d,%d)  点2(%d,%d)\r\n\r\n",pGroups[i].m_seedPoint1.x,
				pGroups[i].m_seedPoint1.y,pGroups[i].m_seedPoint2.x,pGroups[i].m_seedPoint2.y);
			file.Write(str,str.GetLength());
		}
	}

	file.Close();
}

BOOL CDibObject::GaussFilter(CDibObject *pDibObject)
{

	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];
	// 对原图象进行高斯滤波
	GaussianSmooth(pUnchEdge, m_nWidth, m_nHeight, sigma, pUnchSmooth);

	// 调试用
	//CopyBitmap(pBits, widthBytes, m_nHeight, pUnchSmooth, m_nWidth, m_nHeight);
	// 调试完

	// 将pBits所指向位图的位图数据复制到pUnchSmooth所指向的位图
	//CopyBitmap(pUnchSmooth, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);



	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	
	CopyBitmap(pBits, widthBytes, m_nHeight, pUnchSmooth, m_nWidth, m_nHeight);


	::GlobalUnlock(pDibObject->m_hDib);



	delete []pUnchSmooth;
	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
	return TRUE;
}

BOOL CDibObject::Canny(CDibObject *pDibObject)
{	
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);

	pUnchSmooth = new unsigned char[m_nWidth * m_nHeight];
	// 对原图象进行高斯滤波
	GaussianSmooth(pUnchEdge, m_nWidth, m_nHeight, sigma, pUnchSmooth);

	// 调试用
	//CopyBitmap(pBits, widthBytes, m_nHeight, pUnchSmooth, m_nWidth, m_nHeight);
	// 调试完

	pnGradX = new int[m_nWidth * m_nHeight];
	pnGradY = new int[m_nWidth * m_nHeight];
	// 计算方向导数
	DirGrad(pUnchSmooth, m_nWidth, m_nHeight, pnGradX, pnGradY);

	pnGradMag = new int[m_nWidth * m_nHeight];
	// 计算梯度的幅度
	GradMagnitude(pnGradX, pnGradY, m_nWidth, m_nHeight, pnGradMag);

	// 应用non-maximum 抑制
	NonmaxSuppress(pnGradMag, pnGradX, pnGradY, m_nWidth, m_nHeight, pUnchEdge);

	// 应用Hysteresis，找到所有的边界
	Hysteresis(pnGradMag, m_nWidth, m_nHeight, dRatioLow, dRatioHigh, pUnchEdge);

	// 将pBits所指向位图的位图数据复制到pUnchSmooth所指向的位图
	//CopyBitmap(pUnchSmooth, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);


	// 清除位图pUnchEdge中数据
//	ClearBitmap(pUnchEdge, m_nWidth, m_nHeight);

	// 画出所有道路
//	PaintAllRoad(pUnchEdge, m_nWidth, roadArray);



	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	
	//CopyBitmapPixel(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);
	CopyBitmapPixel(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);

	//调试
	//CopyPixel(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);

	::GlobalUnlock(pDibObject->m_hDib);

	// 清除roadArray数组
	//ClearLineArray(roadArray);

//	if(pGroups != NULL)
//	{
//		delete []pGroups;
	//}

	// 清除roadEdgeLineArray数组
//	ClearLineArray(roadEdgeLineArray);

	delete []pnGradMag;
	delete []pnGradY;
	delete []pnGradX;
	delete []pUnchSmooth;
	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
	return TRUE;
}



BOOL CDibObject::Robert(CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);



	int i,j,t1,t2,t;
	for(i=0;i<m_nHeight;i++)
	{
		for(j=0;j<m_nWidth;j++)
		{
			t1=*(pUnchEdge+i*m_nWidth+j)-*(pUnchEdge+(i+1)*m_nWidth+j+1);
			t2=*(pUnchEdge+(i+1)*m_nWidth+j)-*(pUnchEdge+i*m_nWidth+j+1);
			t=sqrt(t1*t1+t2*t2);
			if(t>20)
			{
				*(pUnchEdge+i*m_nWidth+j)=0;
			}
		}
	}



	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	
	CopyBitmap(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);


	::GlobalUnlock(pDibObject->m_hDib);


	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
	return TRUE;
}

void CDibObject::Zoom(float ratioX,float ratioY)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *m_pImgDataOut; 
	
	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return ;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;

	BITMAPINFOHEADER *pBIH;
	pBIH = (BITMAPINFOHEADER *) &pBits[sizeof(BITMAPFILEHEADER)];

	pBits += pBFH->bfOffBits;


	int	m_nWidthOut=int(m_nWidth*ratioX+0.5) ;
	int	m_nHeightOut=int(m_nHeight*ratioY+0.5); 
	int lineByteIn=WidthBytes(m_nBits,m_nWidth);
    int	lineByteOut=WidthBytes(m_nBits,m_nWidthOut);
	m_pImgDataOut=new unsigned char[lineByteOut*m_nHeightOut];
	
	int coordinateX, coordinateY;
	int i, j;
	int k;
	for(i=0; i< m_nHeightOut; i++)
	{
		for(j=0; j<m_nWidthOut; j++)
		{  
			//输出图像坐标为(j,i)的像素映射到原图中的坐标值，即插值位置
			coordinateX=j/ratioX+0.5;
			coordinateY=i/ratioY+0.5;
			
			//若插值位置在输入图像范围内，则近邻插值
			if(0<=coordinateX&&coordinateX<m_nWidth
				&& coordinateY>=0&&coordinateY<m_nHeight)
			{
				for(k=0;k<3;k++)
					*(m_pImgDataOut + i * lineByteOut + j*3 + k) 
					=*(pBits+ coordinateY*lineByteIn+ coordinateX*3 + k) ;
			}
			else //若不在输入图像范围内，则置255  
			{
				for(k=0;k<3;k++)
					*(m_pImgDataOut + i * lineByteOut + j*3+k) = 255;
			}
			
		}
	}
	pBIH->biWidth=m_nWidthOut;
	pBIH->biHeight=m_nHeightOut;
	memcpy(pBits,m_pImgDataOut,lineByteOut*m_nHeightOut);
	m_nWidth=pBIH->biWidth;
	m_nHeight=pBIH->biHeight;

/*	for(i=0; i< m_nHeightOut; i++)
	{
		for(j=0; j<m_nWidthOut; j++)
		{
			*(pBits+i*m_nWidthOut+j)=*(m_pImgDataOut+i*m_nWidthOut+j);
		}
	}*/
	/*
//	widthBytes = WidthBytes(m_nBits, m_nWidth);


	pUnchEdge = new unsigned char[widthBytes * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, widthBytes, m_nHeight, pBits, widthBytes, m_nHeight);


	int m_nWidthOut=int(m_nWidth*0.5+0.5);
	int m_nHeightOut=int(m_nHeight*0.5+0.5);
	int i,j,k;
	int coordinateX,coordinateY;

	//字节数
	widthBytes=(m_nWidth*m_nBits/8+3)/4*4;
    int	widthBytesOut=(m_nWidthOut*m_nBits/8+3)/4*4;

	for(i=0;i<m_nHeightOut;i++)
	{
		for(j=0;j<m_nWidthOut;j++)
		{
			coordinateX=j/0.5+0.5;//coordinate大于i
			coordinateY=i/0.5+0.5;
			if(0<=coordinateX&&coordinateX<m_nWidth&&coordinateY>=0&&coordinateY<m_nHeight)
			{
				for(k=0;k<3;k++)
				{
					*(pBits + i * widthBytesOut + j*3 + k)
					=*(pUnchEdge+ coordinateY*widthBytes + coordinateX*3 + k) ;
				}
			}
			else //若不在输入图像范围内，则置255  
			{
				for(k=0;k<3;k++)
					*(pBits + i * widthBytesOut + j*3+k) = 255;
			}

		}
	}


	m_nWidth=m_nWidthOut;
	m_nHeight=m_nHeightOut;

*/

	delete []m_pImgDataOut;
	::GlobalUnlock(m_hDib);
	return ;
}

BOOL CDibObject::Prewitt(CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);






	int i,j,x,y,t;
	for(i=1;i<m_nHeight-1;i++)
	{
		for(j=1;j<m_nWidth-1;j++)
		{
            //x方向梯度
			x= *(pUnchEdge+(i-1)*m_nWidth+j+1)
			 + *(pUnchEdge+i*m_nWidth+j+1)
			 + *(pUnchEdge+(i+1)*m_nWidth+j+1)
			 - *(pUnchEdge+(i-1)*m_nWidth+j-1)
			 - *(pUnchEdge+i*m_nWidth+j-1)
			 - *(pUnchEdge+(i+1)*m_nWidth+j-1);

			//y方向梯度
			y= *(pUnchEdge+(i-1)*m_nWidth+j-1)
			 + *(pUnchEdge+(i-1)*m_nWidth+j)
			 + *(pUnchEdge+(i-1)*m_nWidth+j+1)
			 - *(pUnchEdge+(i+1)*m_nWidth+j-1)
			 - *(pUnchEdge+(i+1)*m_nWidth+j)
			 - *(pUnchEdge+(i+1)*m_nWidth+j+1);

			//t=sqrt(x*x+y*y)+0.5;
			t=x>y?x:y;
			if(t>130)
			{
				*(pUnchEdge+i*m_nWidth+j)=0;
			}
		}
	}


	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	
	CopyBitmap(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);


	::GlobalUnlock(pDibObject->m_hDib);


	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
	return TRUE;
}


BOOL CDibObject::Sobel(CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);






	int i,j,x,y,t;
	for(i=1;i<m_nHeight-1;i++)
	{
		for(j=1;j<m_nWidth-1;j++)
		{
            //x方向梯度
			x= *(pUnchEdge+(i-1)*m_nWidth+j+1)
			 + 2**(pUnchEdge+i*m_nWidth+j+1)
			 + *(pUnchEdge+(i+1)*m_nWidth+j+1)
			 - *(pUnchEdge+(i-1)*m_nWidth+j-1)
			 - 2**(pUnchEdge+i*m_nWidth+j-1)
			 - *(pUnchEdge+(i+1)*m_nWidth+j-1);

			//y方向梯度
			y= *(pUnchEdge+(i-1)*m_nWidth+j-1)
			 + 2**(pUnchEdge+(i-1)*m_nWidth+j)
			 + *(pUnchEdge+(i-1)*m_nWidth+j+1)
			 - *(pUnchEdge+(i+1)*m_nWidth+j-1)
			 - 2**(pUnchEdge+(i+1)*m_nWidth+j)
			 - *(pUnchEdge+(i+1)*m_nWidth+j+1);

			//t=sqrt(x*x+y*y)+0.5;
			t=x>y?x:y;
			if(t>230)
			{
				*(pUnchEdge+i*m_nWidth+j)=0;
			}
		}
	}


	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	
	CopyBitmap(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);


	::GlobalUnlock(pDibObject->m_hDib);


	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
	return TRUE;
}

BOOL CDibObject::Laplacian(CDibObject *pDibObject)
{
	unsigned char *pBits;  // 图像位图数据指针
	BITMAPFILEHEADER *pBFH;  // 图像位图文件头指针
	int widthBytes;  // 图像位图字节宽度
	unsigned char *pUnchSmooth;  // 经过高斯滤波后的图像位图数据
	unsigned char *pUnchEdge;  // 存放提取出的边界
	double sigma = 0.4, dRatioLow = 0.4, dRatioHigh = 0.79;
	int *pnGradX; // 指向x方向导数的指针
	int *pnGradY;  // 指向y方向导数的指针
	int *pnGradMag;  // 梯度的幅度

	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(m_nBits, m_nWidth);

	pUnchEdge = new unsigned char[m_nWidth * m_nHeight];

	// 将pBits所指向位图的位图数据复制到pUnchEdge所指向的位图
	CopyBitmap(pUnchEdge, m_nWidth, m_nHeight, pBits, widthBytes, m_nHeight);






	int i,j,t;
	for(i=1;i<m_nHeight;i++)
	{
		for(j=1;j<m_nWidth;j++)
		{
			t=4**(pUnchEdge+i*m_nWidth+j)-*(pUnchEdge+(i-1)*m_nWidth+j)-*(pUnchEdge+(i+1)*m_nWidth+j)
				-*(pUnchEdge+i*m_nWidth+j-1)-*(pUnchEdge+i*m_nWidth+j+1);
		//	t=abs(t)+0.5;
			if(t>136)
			{
				*(pUnchEdge+i*m_nWidth+j)=0;
			}
		}
	}


	// 获取图像位图数据指针
	pBits = (unsigned char *)::GlobalLock(pDibObject->m_hDib);
	if(pBits == NULL)
	{
		return FALSE;
	}
	pBFH = (BITMAPFILEHEADER*)pBits;
	pBits += pBFH->bfOffBits;
	widthBytes = WidthBytes(pDibObject->m_nBits, pDibObject->m_nWidth);

	
	CopyBitmap(pBits, widthBytes, m_nHeight, pUnchEdge, m_nWidth, m_nHeight);


	::GlobalUnlock(pDibObject->m_hDib);


	delete []pUnchEdge;
	::GlobalUnlock(m_hDib);
	return TRUE;
	return TRUE;
}


// 调试完