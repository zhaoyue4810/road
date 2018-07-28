#ifndef __DibObject_H__
#define __DibObject_H__

#include <afxtempl.h>
#include "Line.h"
#include "Group.h"
#include "Structs.h"

#define FIXED_PALETTE 2
#define GETRGB555( a, b, c, d ) { WORD *wData = (WORD *) d; a = (unsigned char) ( ( (*wData) & 0x7c00 ) >> 7 ); b = (unsigned char) ( ( (*wData) & 0x03e0 ) >> 2 ); c = (unsigned char) ( ( (*wData) & 0x001f ) << 3 ); }

class CDibObject : public CObject
{
public:
	~CDibObject(){ GlobalFree(m_hDib); }
	//无参构造函数
	CDibObject();
	// 有参构造函数
	CDibObject( const char *pszFilename, CPoint &startPoint, CSize &jieQuSize,
		float scale, CDC *pDC = NULL, int nX = -1, int nY = -1 );
	//拷贝构造函数
	CDibObject(CDibObject* pDibObject);
	// 绘制图像
	BOOL Draw( CDC *, int x = -1, int y = -1 );
	// 绘制图像
	BOOL DrawPixel( CDC *, int x = -1, int y = -1 );

	BOOL DrawWhite( CDC *, int x = -1, int y = -1 );
	// 得到宽度(像素单位)
	int GetWidth( void );
	// 得到字节宽度
	int GetWidthBytes( void );
	// 得到高度(像素单位)
	int GetHeight( void );
	// 设置调色板
	BOOL SetPalette( CDC * );
	// 得到位图句柄
	HGLOBAL GetDib( void );
	// 获取合成通道
	BOOL GetMixedChannel(CDibObject *&pMixed);
	// 获取红色通道
	BOOL GetRedChannel(CDibObject *&pMixed);
	// 获取绿色通道
	BOOL GetGreenChannel(CDibObject *&pMixed);
	// 获取蓝色通道
	BOOL GetBlueChannel(CDibObject *&pMixed);
	// 设置最后调色板类型
	void SetPaletteBytes( int nPaletteBytes ){ m_nPaletteBytes = nPaletteBytes; }
	// 设置位图
	void SetDib( HGLOBAL hDib ){ m_hDib = hDib; }
	// 获取分割后的图像存放于参数pDibObject中
	BOOL GetSubDibObject(CRect &rect, CDibObject *&pDibObject);
	// 自动道路提取
	BOOL AutoRoadExtract(CDibObject *pDibObject);
	// 自动湖泊提取
	BOOL AutoLakeExtract(CDibObject *pDibObject);
	// 交互道路提取
	BOOL InteractRoadExtract(CPoint roadSeed1, CPoint roadSeed2, CDibObject *pDibObject);
	// 水体交互提取
	BOOL InteractLakeExtract(CPoint lakeSeed, CDibObject *pDibObject);
	// 水体交互提取
	BOOL InteractAllLakeExtract(CPoint lakeSeed, CDibObject *pDibObject);
	// 画多边形
	BOOL DrawPolygon(CArray<CPoint,CPoint> &polygonPoints);
	// 擦除
	void Earser(CPoint point);
	// 擦除选中的区域
	void EarseArea(CPoint point1, CPoint point2);
	// 画线
	void DrawLines(CArray<CPoint,CPoint> &linePoints);
	BOOL Canny(CDibObject *pDibObject);
	//void CopyPixel(unsigned char *pDestBits, int destWidth, int destHeight,unsigned char *pSrcBits, int srcWidth, int srcHeight);
	BOOL GaussFilter(CDibObject *pDibObject);
	BOOL Robert(CDibObject *pDibObject);
	BOOL Prewitt(CDibObject *pDibObject);
	BOOL Sobel(CDibObject *pDibObject);
	BOOL Laplacian(CDibObject *pDibObject);
	void Zoom(float ratioX,float ratioY);

protected:
	// 初始化参数
	void InitVars( BOOL bFlag = FALSE );
	// 读入图像
	BOOL Load( const char *, CPoint &startPoint, CSize &jieQuSize, float scale, CDC *pDC = NULL, int nX = -1, int nY = -1 );
	// 清除图像
	void KillImage( void );
	// 处理图像头
	void ProcessImageHeader( void );
	// 处理调色板
	void ProcessPalette( void );
	// 字节宽度
	int WidthBytes( int, int );
	// 创建逻辑调色板
	LOGPALETTE *CreateLogPalette( RGBQUAD *Palette, int NumColors );
	// 判断是否为256级灰度图像
	BOOL IsGray256();
	// 图像从彩色转换为灰度图像并保存于参数图像pDibObject中
	BOOL MakeGray256( BYTE mGrayType, CDibObject *&pDibObject );
	// 载入Geotiff格式图像
	HGLOBAL LoadGetTif(const char *pszFilename, CPoint &startPoint, CSize &jieQuSize, float scale);
	// 验证矩形所示区域是否是有效区域
	BOOL CheckRect(CRect &rect);
	// 图像位图数据复制
	void CopyBitmap(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight);
	// 图像位图数据复制，只复制灰度小于8的点
	void CopyBitmapPixel(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight);
	// 高斯滤波
	void GaussianSmooth(unsigned char *pUnchImg, int nWidth, int nHeight, 
										double sigma, unsigned char * pUnchSmthdImg);
	// 产生一维高斯数据滤波器
	void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize);
	// 计算方向导数
	void CDibObject::DirGrad(unsigned char *pUnchSmthdImg, int nWidth, int nHeight,
										 int *pnGradX , int *pnGradY);
	// 计算梯度的幅度
	void GradMagnitude(int *pnGradX, int *pnGradY, int nWidth, int nHeight, int *pnMag);
	//  抑止梯度图中非局部极值点的象素
	void NonmaxSuppress(int *pnMag, int *pnGradX, int *pnGradY, int nWidth, 
										int nHeight,	unsigned char *pUnchRst);
	// 找到所有的边界
	void Hysteresis(int *pnMag, int nWidth, int nHeight, double dRatioLow, 
							double dRatioHigh, unsigned char *pUnchEdge);
	// 估计TraceEdge需要的低阈值，以及Hysteresis函数使用的高阈值
	void EstimateThreshold(int *pnMag, int nWidth, int nHeight, int *pnThdHigh,int *pnThdLow, 
							unsigned char * pUnchEdge, double dRatioHigh, double dRationLow);
	// 进行边界点跟踪
	void TraceEdge (int y, int x, int nLowThd, unsigned char *pUnchEdge, int *pnMag, int nWidth);
	// 获取道路边界记录于参数roadEdgeLineArray中
	void GetRoadEdgeLine(unsigned char *pUnchEdge, int nWidth, int nHeight,
		CArray<CLine*,CLine*> &roadEdgeLineArray,unsigned char *pUnchSmooth,int *pnGradMag);
	// 初始化分组数组
	void InitGroups(CGroup *&pGroups, int &groupNum, CArray<CLine*,CLine*> &lineArray);
	// 检查数组pointArray中存储的线中是否有道路边界线，如果有截取后将其加入roadEdgeLineArray中
	void ExtractRoadEdgeLine(CArray<CPoint,CPoint> &pointArray,CArray<CLine*,CLine*> &roadEdgeLineArray,
		unsigned char *pUnchSmooth,int *pnGradMag,int nWidth,int nHeight);
	// 计算两矢量夹角，lx1和ly1为矢量1，lx2和ly2为矢量2
	double GetDirCha(double lx1,double ly1,double lx2,double ly2);
	// 检查已截取的线段subPointArray是否是道路边界，如果是则将其加入数组roadEdgeLineArray中
	void CheckRoadEdgeLine(CArray<CPoint,CPoint> &pointArray,CArray<CLine*,CLine*> &roadEdgeLineArray,
		unsigned char *pUnchSmooth,int *pnGradMag,int nWidth,int nHeight);
	// 检查指定点是否是边界点
	BOOL IsEdgePoint(CPoint point1,int nWidth,int nHeight);
	// 层次记号编组
	void CengCiBianZu(CArray<CLine*,CLine*> &roadEdgeLineArray, CGroup *pGroup, int groupNum);
	// 将同一区域中的边界线编为一组
	void CengCiBianZuLake(CArray<CLine*,CLine*> &roadEdgeLineArray, CGroup *pGroup, int groupNum);
	// 检查两线段是否可以编为一组
	BOOL CanBianZu(CLine *pLine1, CLine *pLine2, BOOL &parallel);
	// 检查指定点是否与指定线段相邻，并通过参数pos返回线段上的相邻点的索引
	BOOL XiangLin(CPoint point, CLine *pLine, double highJuLi, int &pos);
	// 得到两点之间的距离
	double GetJuLi(CPoint point1,CPoint point2);
	// 合并线段组
	void HeBingGroup(CGroup *pGroup, int groupNum, int group1ID, int group2ID, BOOL parallel);
	// 计算置信度
	void CalBeliefDegree(CGroup *pGroups, int groupNum);
	// 获取道路边界点
	BOOL GetRoadEdgePoint(CGroup *pGroups, int groupNum, CArray<CLine*,CLine*> &roadEdgeLineArray,
		int *pnGradMag, unsigned char *pUnchSmooth, int width, int height);
	// 计算指定点指定领域内的灰度
	double GetAveGray(CPoint point, unsigned char *pUnchSmooth, int width, int height, int length);
	// 根据道路种子点跟踪道路
	void TrackRoadByGroup(CGroup* pGroups, int groupNum, unsigned char *pUnchSmooth, int width, 
		int height, CArray<CLine*, CLine*> &roadArray);
	// 根据两个点向两个方向延伸道路
	void TrackGrayRoadByEdgePoint(CPoint point1, CPoint point2, unsigned char *pUnchSmooth, int width,
		int height, double highCost, CArray<CPoint, CPoint> &roadPointArray);
	// 根据两个点向两个方向延伸道路
	void TrackWhiteRoadByEdgePoint(CPoint point1, CPoint point2, unsigned char *pUnchSmooth, int width,
		int height, double highCost, CArray<CPoint, CPoint> &roadPointArray);
	// 在靠近pointA点处得到灰度最大点、灰度次大点
	void GetGrayestPoint(CPoint pointA,CPoint pointB,unsigned char *pUnchSmooth,int width,
		int height,CPoint &grayestPoint,CPoint &grayerPoint);
	// 在靠近pointA点处得到灰度最小点、灰度次小点
	void GetWhitestPoint(CPoint pointA,CPoint pointB,unsigned char *pUnchSmooth,int width,
		int height,CPoint &whitestPoint,CPoint &whiterPoint);
	// 得到参与比较的各个点，在经过pointA点并且斜率垂直于线段(pointA,pB)的小线段上找
	void GetParePoints(CPoint pointA,CPoint pointB,int extPointNum,int width,int height,
		CArray<CPoint,CPoint> &parePoints);
	// 灰色道路跟踪，从第一个点向第二个点方向延伸
	void TrackRoadBySeedPoint(CPoint pointA,CPoint pointB,CPoint pointB1,unsigned char *pUnchSmooth,
				   int width,int height,double highCost,CArray<CPoint, CPoint> &roadPointArray);
	// 得到线段AB上的各个点,记录点从A到B
	void GetLinePoints(CPoint pointA,CPoint pointB,CArray<CPoint,CPoint> &linePoints);
	// 得到下一个跟踪点，A—B—C
	CPoint GetNextTrackPoint(CPoint pointA,CPoint pointB,int trackLen,int width,int height);
	// 清除位图数据
	void ClearBitmap(unsigned char *pBitmap, int width, int height);
	// 画出所有道路
	void PaintAllRoad(unsigned char *pBits, int widthBytes, CArray<CLine*, CLine*> &roadArray);
	// 画出单条道路
	void PaintOneRoad(unsigned char *pBits, int widthBytes, CArray<CPoint, CPoint> &roadPointArray);
	// 清除数组
	void ClearLineArray(CArray<CLine*, CLine*> &array);
	// 知识推理减少虚警
	void FindRoad(CArray<CLine*,CLine*> &roadArray, int width, int height);
	// 连接头尾相连的可能中间断掉过的可能道路
	BOOL HeadTailLink(CLine *pLine1, CLine *pLine2);
	// 通过边界端点检查是否是真正的道路
	void CheckRoadByEdgePoint(CArray<CLine*, CLine*> &roadArray, int width, int height);
	// 索引为j的线段是否是连接到索引为i的道路上的道路
	BOOL CheckRoadByRoad(CArray<CLine*, CLine*> &roadArray, int j, int i);
	// 验证道路种子点是否满足要求
	BOOL CheckRoadSeed(CPoint roadSeed1, CPoint roadSeed2);

	// 获取边界线记录于变量edgeLineArray中
	void GetEdgeLine(unsigned char *pUnchEdge, int width, int height, int gray, CArray<CLine*,CLine*> &edgeLineArray);
	// 对线段两端点进行延伸，以填补小缺口
	void ExtendLine(CArray<CLine*, CLine*> &edgeLineArray,
		unsigned char *pUnchEdge, int width, int height);
	// 对端点进行延伸
	void ExtendPoint(CPoint point, unsigned char *pUnchEdge, int width, int height);
	// 在pUnchEdge上连接给点的两个点
	void LinkPoints(CPoint pointA, CPoint pointB, unsigned char *pUnchEdge, int width);
	// 寻找湖泊
	void FindLake(unsigned char *pUnchSmooth, unsigned char *pUnchEdge, int width, int height);
	// 获取平均灰度和灰度方差
	void AnalyseGray(unsigned char *pUnchSmooth, int width, int x, int y,
					int subWidth, int subHeight, double &aveGray, double &grayFangCha);
	// 获取平均灰度和灰度方差
	void AnalyseGray(unsigned char *pUnchSmooth, int width, CArray<CPoint, CPoint> &pointArray,
		double &aveGray, double &grayFangCha);
	 // 按给定点扩展湖泊区域
	void FillByPoint(unsigned char *pUnchSmooth,unsigned char *pUnchEdge,
				 int width,int height,CPoint point);
	// 按给定点扩展湖泊区域
	void FillByPoint1(unsigned char *pUnchSmooth,unsigned char *pUnchEdge,
				 int width,int height,CPoint point);
	void FillByPoint(unsigned char *pUnchEdge, int width,int height,CPoint point);
	// 进行验证，保留灰度方差小的区域
	double VerifyGrayArea(unsigned char *pUnchEdge,int width,int height,int grays[]);
	// 去除边界点或通过验证的区域,边界点灰度为0,通过验证的区域点灰度为1,通过验证的边界点灰度为2,可自由选择
	void RemoveEdge(unsigned char *pUnchEdge, int width, int height, int gray);
	// 检验两线段是否相邻，若相邻则进行连接
	BOOL LinesXiangLin(CLine *pLine1, CLine *pLine2);
	// 对湖泊区域进行验证
	void CheckLake(CGroup *pGroups, int groupNum, CArray<CLine*, CLine *> &edgeLineArray,
						   unsigned char *pUnchEdge, int width, int height);
	// 通过边界对区域进行验证是否是湖泊
	BOOL CheckLakeByEdge(int groupID, CArray<CLine*, CLine*> &edgeLineArray);
	// 求取线段上的角点并存放于cornerArray数组中
	void GetLineCorner(CLine* pLine,CArray<CPoint,CPoint> &cornerArray);
	// 通过角点寻找直线
	void CDibObject::FindLine(CArray<CPoint,CPoint> &cornerArray,
		CArray<CFindedLine,CFindedLine> &findedLineArray, CArray<CDegree,CDegree> &degreeArray);
	// 通过直线长度与边界线长度的比值检验
	BOOL CheckByLineLenRate(CArray<CFindedLine,CFindedLine> &findedLineArray,double edgeLineLen);
	// 通过平行线检验
	BOOL CheckByParallelLine(CArray<CFindedLine,CFindedLine> &findedLineArray,double edgeLineLen);
	// 通过寻找到的直线夹角检验
	BOOL CheckByDegree(CArray<CDegree,CDegree> &degreeArray);
	// 得到点到直线的距离
	double GetPointLineJuLi(double A,double B,double C,CPoint point);
	// 得到一个点在直线上的投影
	CPoint GetTouYingPoint(double A,double B,double C,CPoint point);
	// 验证已验证的区域点(灰度为1)是否在已验证的边界点(灰度为2)内部
	BOOL CheckInsidePoint(CPoint point, int groupID, CArray<CLine*, CLine*> &edgeLineArray,
								  unsigned char *pUnchEdge, int width, int height);
	// 获取包含线段的最小矩形
	void GetSmallRectOfLine(CLine *pLine, CRect &rect);
	// 获取包含区域的最小矩形
	void GetSmallRectOfArea(CPoint point, unsigned char *pUnchEdge, int width, int height, CRect &rect);
	// 去除区域及边界
	void RemoveAreaAndEdgeByPoint(CPoint point, unsigned char *pUnchEdge, int width, int height, int gray);
	// 验证湖泊种子点是否满足要求
	BOOL CheckLakeSeed(CPoint lakeSeed);
	// 验证多边形的点所形成的区域是否是有效区域
	BOOL CheckPolygonPoints(CArray<CPoint,CPoint> &polygonPoints);
	// 画多边形边界
	void DrawPolygonLine(CArray<CPoint,CPoint> &polygonPoints);
	// 判断一个点是否在多边形内部
	BOOL InsidePolygon(CPoint point, CArray<CPoint,CPoint> &polygonPoints);
	// 种子点填充算法填充多边形
	void FillArea(CPoint point);

	// 调试用
	// 画出所有道路边界线
	void PaintAllRoadEdgeLine(unsigned char *pUnchEdge, int nWidth,
		CArray<CLine*,CLine*> &roadEdgeLineArray);
	// 按组画出道路边界线
	void PaintRoadEdgeLineByGroup(unsigned char *pUnchEdge, int width,
		CArray<CLine*,CLine*> &roadEdgeLineArray, int groupID);
	// 画出单条道路边界线
	void PaintOneRoadEdgeLine(unsigned char *pUnchEdge, int nWidth,
		CArray<CLine*,CLine*> &roadEdgeLineArray, int index);
	// 将边界线信息记录于文本中
	void RecordRoadEdgeLineToText(CArray<CLine*,CLine*> &roadEdgeLineArray);
	void RecordWaterEdgeLineToText(CArray<CLine*,CLine*> &waterEdgeLineArray);
	// 将编组信息记录于文本中
	void RecordGroupsToText(CGroup *pGroup, int lineNum);
	// 画出找到的直线
	void PaintFindLine(CArray<CFindedLine,CFindedLine> &findedLineArray);
	// 调试完
public:
	void GetMultiPoints(CArray<CPoint,CPoint> &polygonPoints,CArray<CPoint,CPoint> &pointArray);
	//手工提取道路和水体
	void RecordPoints(CArray<CPoint,CPoint> &Points);
	void RecordPoints_wateredge_human(CArray<CPoint,CPoint> &Points);
	void RecordPoints_waterbody_human(unsigned char *pUnchEdge, int width, int height);

	//半自动提取道路和水体
	void RecordWaterEdgePoints(unsigned char *pUnchEdge, int width, int height);
	void RecordWaterPoints(unsigned char *pUnchEdge, int width, int height);
	void RecordRoadPoints(unsigned char *pUnchEdge, int width, int height);

public:
	int m_nWidth, m_nHeight, m_nBits, m_nColors, m_nImageType, m_nX,
		m_nY, m_nLastError,	m_nPaletteCreationType, m_nQuality, m_nPlanes,
		m_nScreenPlanes, m_nScreenBits, m_nPaletteBytes;
	HGLOBAL m_hDib;
	char *m_pszFilename;
	CPalette m_Palette;
	// 调试用
	unsigned char *pGUnchEdge;
	// 调试完

	//调试
//public:
//	int m_nWidth,m_nHeight;
	//调试
};

#endif