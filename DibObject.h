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
	//�޲ι��캯��
	CDibObject();
	// �вι��캯��
	CDibObject( const char *pszFilename, CPoint &startPoint, CSize &jieQuSize,
		float scale, CDC *pDC = NULL, int nX = -1, int nY = -1 );
	//�������캯��
	CDibObject(CDibObject* pDibObject);
	// ����ͼ��
	BOOL Draw( CDC *, int x = -1, int y = -1 );
	// ����ͼ��
	BOOL DrawPixel( CDC *, int x = -1, int y = -1 );

	BOOL DrawWhite( CDC *, int x = -1, int y = -1 );
	// �õ����(���ص�λ)
	int GetWidth( void );
	// �õ��ֽڿ��
	int GetWidthBytes( void );
	// �õ��߶�(���ص�λ)
	int GetHeight( void );
	// ���õ�ɫ��
	BOOL SetPalette( CDC * );
	// �õ�λͼ���
	HGLOBAL GetDib( void );
	// ��ȡ�ϳ�ͨ��
	BOOL GetMixedChannel(CDibObject *&pMixed);
	// ��ȡ��ɫͨ��
	BOOL GetRedChannel(CDibObject *&pMixed);
	// ��ȡ��ɫͨ��
	BOOL GetGreenChannel(CDibObject *&pMixed);
	// ��ȡ��ɫͨ��
	BOOL GetBlueChannel(CDibObject *&pMixed);
	// ��������ɫ������
	void SetPaletteBytes( int nPaletteBytes ){ m_nPaletteBytes = nPaletteBytes; }
	// ����λͼ
	void SetDib( HGLOBAL hDib ){ m_hDib = hDib; }
	// ��ȡ�ָ���ͼ�����ڲ���pDibObject��
	BOOL GetSubDibObject(CRect &rect, CDibObject *&pDibObject);
	// �Զ���·��ȡ
	BOOL AutoRoadExtract(CDibObject *pDibObject);
	// �Զ�������ȡ
	BOOL AutoLakeExtract(CDibObject *pDibObject);
	// ������·��ȡ
	BOOL InteractRoadExtract(CPoint roadSeed1, CPoint roadSeed2, CDibObject *pDibObject);
	// ˮ�彻����ȡ
	BOOL InteractLakeExtract(CPoint lakeSeed, CDibObject *pDibObject);
	// ˮ�彻����ȡ
	BOOL InteractAllLakeExtract(CPoint lakeSeed, CDibObject *pDibObject);
	// �������
	BOOL DrawPolygon(CArray<CPoint,CPoint> &polygonPoints);
	// ����
	void Earser(CPoint point);
	// ����ѡ�е�����
	void EarseArea(CPoint point1, CPoint point2);
	// ����
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
	// ��ʼ������
	void InitVars( BOOL bFlag = FALSE );
	// ����ͼ��
	BOOL Load( const char *, CPoint &startPoint, CSize &jieQuSize, float scale, CDC *pDC = NULL, int nX = -1, int nY = -1 );
	// ���ͼ��
	void KillImage( void );
	// ����ͼ��ͷ
	void ProcessImageHeader( void );
	// �����ɫ��
	void ProcessPalette( void );
	// �ֽڿ��
	int WidthBytes( int, int );
	// �����߼���ɫ��
	LOGPALETTE *CreateLogPalette( RGBQUAD *Palette, int NumColors );
	// �ж��Ƿ�Ϊ256���Ҷ�ͼ��
	BOOL IsGray256();
	// ͼ��Ӳ�ɫת��Ϊ�Ҷ�ͼ�񲢱����ڲ���ͼ��pDibObject��
	BOOL MakeGray256( BYTE mGrayType, CDibObject *&pDibObject );
	// ����Geotiff��ʽͼ��
	HGLOBAL LoadGetTif(const char *pszFilename, CPoint &startPoint, CSize &jieQuSize, float scale);
	// ��֤������ʾ�����Ƿ�����Ч����
	BOOL CheckRect(CRect &rect);
	// ͼ��λͼ���ݸ���
	void CopyBitmap(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight);
	// ͼ��λͼ���ݸ��ƣ�ֻ���ƻҶ�С��8�ĵ�
	void CopyBitmapPixel(unsigned char *pDestBits, int destWidth, int destHeight,
		unsigned char *pSrcBits, int srcWidth, int srcHeight);
	// ��˹�˲�
	void GaussianSmooth(unsigned char *pUnchImg, int nWidth, int nHeight, 
										double sigma, unsigned char * pUnchSmthdImg);
	// ����һά��˹�����˲���
	void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize);
	// ���㷽����
	void CDibObject::DirGrad(unsigned char *pUnchSmthdImg, int nWidth, int nHeight,
										 int *pnGradX , int *pnGradY);
	// �����ݶȵķ���
	void GradMagnitude(int *pnGradX, int *pnGradY, int nWidth, int nHeight, int *pnMag);
	//  ��ֹ�ݶ�ͼ�зǾֲ���ֵ�������
	void NonmaxSuppress(int *pnMag, int *pnGradX, int *pnGradY, int nWidth, 
										int nHeight,	unsigned char *pUnchRst);
	// �ҵ����еı߽�
	void Hysteresis(int *pnMag, int nWidth, int nHeight, double dRatioLow, 
							double dRatioHigh, unsigned char *pUnchEdge);
	// ����TraceEdge��Ҫ�ĵ���ֵ���Լ�Hysteresis����ʹ�õĸ���ֵ
	void EstimateThreshold(int *pnMag, int nWidth, int nHeight, int *pnThdHigh,int *pnThdLow, 
							unsigned char * pUnchEdge, double dRatioHigh, double dRationLow);
	// ���б߽�����
	void TraceEdge (int y, int x, int nLowThd, unsigned char *pUnchEdge, int *pnMag, int nWidth);
	// ��ȡ��·�߽��¼�ڲ���roadEdgeLineArray��
	void GetRoadEdgeLine(unsigned char *pUnchEdge, int nWidth, int nHeight,
		CArray<CLine*,CLine*> &roadEdgeLineArray,unsigned char *pUnchSmooth,int *pnGradMag);
	// ��ʼ����������
	void InitGroups(CGroup *&pGroups, int &groupNum, CArray<CLine*,CLine*> &lineArray);
	// �������pointArray�д洢�������Ƿ��е�·�߽��ߣ�����н�ȡ�������roadEdgeLineArray��
	void ExtractRoadEdgeLine(CArray<CPoint,CPoint> &pointArray,CArray<CLine*,CLine*> &roadEdgeLineArray,
		unsigned char *pUnchSmooth,int *pnGradMag,int nWidth,int nHeight);
	// ������ʸ���нǣ�lx1��ly1Ϊʸ��1��lx2��ly2Ϊʸ��2
	double GetDirCha(double lx1,double ly1,double lx2,double ly2);
	// ����ѽ�ȡ���߶�subPointArray�Ƿ��ǵ�·�߽磬����������������roadEdgeLineArray��
	void CheckRoadEdgeLine(CArray<CPoint,CPoint> &pointArray,CArray<CLine*,CLine*> &roadEdgeLineArray,
		unsigned char *pUnchSmooth,int *pnGradMag,int nWidth,int nHeight);
	// ���ָ�����Ƿ��Ǳ߽��
	BOOL IsEdgePoint(CPoint point1,int nWidth,int nHeight);
	// ��μǺű���
	void CengCiBianZu(CArray<CLine*,CLine*> &roadEdgeLineArray, CGroup *pGroup, int groupNum);
	// ��ͬһ�����еı߽��߱�Ϊһ��
	void CengCiBianZuLake(CArray<CLine*,CLine*> &roadEdgeLineArray, CGroup *pGroup, int groupNum);
	// ������߶��Ƿ���Ա�Ϊһ��
	BOOL CanBianZu(CLine *pLine1, CLine *pLine2, BOOL &parallel);
	// ���ָ�����Ƿ���ָ���߶����ڣ���ͨ������pos�����߶��ϵ����ڵ������
	BOOL XiangLin(CPoint point, CLine *pLine, double highJuLi, int &pos);
	// �õ�����֮��ľ���
	double GetJuLi(CPoint point1,CPoint point2);
	// �ϲ��߶���
	void HeBingGroup(CGroup *pGroup, int groupNum, int group1ID, int group2ID, BOOL parallel);
	// �������Ŷ�
	void CalBeliefDegree(CGroup *pGroups, int groupNum);
	// ��ȡ��·�߽��
	BOOL GetRoadEdgePoint(CGroup *pGroups, int groupNum, CArray<CLine*,CLine*> &roadEdgeLineArray,
		int *pnGradMag, unsigned char *pUnchSmooth, int width, int height);
	// ����ָ����ָ�������ڵĻҶ�
	double GetAveGray(CPoint point, unsigned char *pUnchSmooth, int width, int height, int length);
	// ���ݵ�·���ӵ���ٵ�·
	void TrackRoadByGroup(CGroup* pGroups, int groupNum, unsigned char *pUnchSmooth, int width, 
		int height, CArray<CLine*, CLine*> &roadArray);
	// �������������������������·
	void TrackGrayRoadByEdgePoint(CPoint point1, CPoint point2, unsigned char *pUnchSmooth, int width,
		int height, double highCost, CArray<CPoint, CPoint> &roadPointArray);
	// �������������������������·
	void TrackWhiteRoadByEdgePoint(CPoint point1, CPoint point2, unsigned char *pUnchSmooth, int width,
		int height, double highCost, CArray<CPoint, CPoint> &roadPointArray);
	// �ڿ���pointA�㴦�õ��Ҷ����㡢�Ҷȴδ��
	void GetGrayestPoint(CPoint pointA,CPoint pointB,unsigned char *pUnchSmooth,int width,
		int height,CPoint &grayestPoint,CPoint &grayerPoint);
	// �ڿ���pointA�㴦�õ��Ҷ���С�㡢�Ҷȴ�С��
	void GetWhitestPoint(CPoint pointA,CPoint pointB,unsigned char *pUnchSmooth,int width,
		int height,CPoint &whitestPoint,CPoint &whiterPoint);
	// �õ�����Ƚϵĸ����㣬�ھ���pointA�㲢��б�ʴ�ֱ���߶�(pointA,pB)��С�߶�����
	void GetParePoints(CPoint pointA,CPoint pointB,int extPointNum,int width,int height,
		CArray<CPoint,CPoint> &parePoints);
	// ��ɫ��·���٣��ӵ�һ������ڶ����㷽������
	void TrackRoadBySeedPoint(CPoint pointA,CPoint pointB,CPoint pointB1,unsigned char *pUnchSmooth,
				   int width,int height,double highCost,CArray<CPoint, CPoint> &roadPointArray);
	// �õ��߶�AB�ϵĸ�����,��¼���A��B
	void GetLinePoints(CPoint pointA,CPoint pointB,CArray<CPoint,CPoint> &linePoints);
	// �õ���һ�����ٵ㣬A��B��C
	CPoint GetNextTrackPoint(CPoint pointA,CPoint pointB,int trackLen,int width,int height);
	// ���λͼ����
	void ClearBitmap(unsigned char *pBitmap, int width, int height);
	// �������е�·
	void PaintAllRoad(unsigned char *pBits, int widthBytes, CArray<CLine*, CLine*> &roadArray);
	// ����������·
	void PaintOneRoad(unsigned char *pBits, int widthBytes, CArray<CPoint, CPoint> &roadPointArray);
	// �������
	void ClearLineArray(CArray<CLine*, CLine*> &array);
	// ֪ʶ��������龯
	void FindRoad(CArray<CLine*,CLine*> &roadArray, int width, int height);
	// ����ͷβ�����Ŀ����м�ϵ����Ŀ��ܵ�·
	BOOL HeadTailLink(CLine *pLine1, CLine *pLine2);
	// ͨ���߽�˵����Ƿ��������ĵ�·
	void CheckRoadByEdgePoint(CArray<CLine*, CLine*> &roadArray, int width, int height);
	// ����Ϊj���߶��Ƿ������ӵ�����Ϊi�ĵ�·�ϵĵ�·
	BOOL CheckRoadByRoad(CArray<CLine*, CLine*> &roadArray, int j, int i);
	// ��֤��·���ӵ��Ƿ�����Ҫ��
	BOOL CheckRoadSeed(CPoint roadSeed1, CPoint roadSeed2);

	// ��ȡ�߽��߼�¼�ڱ���edgeLineArray��
	void GetEdgeLine(unsigned char *pUnchEdge, int width, int height, int gray, CArray<CLine*,CLine*> &edgeLineArray);
	// ���߶����˵�������죬���Сȱ��
	void ExtendLine(CArray<CLine*, CLine*> &edgeLineArray,
		unsigned char *pUnchEdge, int width, int height);
	// �Զ˵��������
	void ExtendPoint(CPoint point, unsigned char *pUnchEdge, int width, int height);
	// ��pUnchEdge�����Ӹ����������
	void LinkPoints(CPoint pointA, CPoint pointB, unsigned char *pUnchEdge, int width);
	// Ѱ�Һ���
	void FindLake(unsigned char *pUnchSmooth, unsigned char *pUnchEdge, int width, int height);
	// ��ȡƽ���ҶȺͻҶȷ���
	void AnalyseGray(unsigned char *pUnchSmooth, int width, int x, int y,
					int subWidth, int subHeight, double &aveGray, double &grayFangCha);
	// ��ȡƽ���ҶȺͻҶȷ���
	void AnalyseGray(unsigned char *pUnchSmooth, int width, CArray<CPoint, CPoint> &pointArray,
		double &aveGray, double &grayFangCha);
	 // ����������չ��������
	void FillByPoint(unsigned char *pUnchSmooth,unsigned char *pUnchEdge,
				 int width,int height,CPoint point);
	// ����������չ��������
	void FillByPoint1(unsigned char *pUnchSmooth,unsigned char *pUnchEdge,
				 int width,int height,CPoint point);
	void FillByPoint(unsigned char *pUnchEdge, int width,int height,CPoint point);
	// ������֤�������Ҷȷ���С������
	double VerifyGrayArea(unsigned char *pUnchEdge,int width,int height,int grays[]);
	// ȥ���߽���ͨ����֤������,�߽��Ҷ�Ϊ0,ͨ����֤�������Ҷ�Ϊ1,ͨ����֤�ı߽��Ҷ�Ϊ2,������ѡ��
	void RemoveEdge(unsigned char *pUnchEdge, int width, int height, int gray);
	// �������߶��Ƿ����ڣ����������������
	BOOL LinesXiangLin(CLine *pLine1, CLine *pLine2);
	// �Ժ������������֤
	void CheckLake(CGroup *pGroups, int groupNum, CArray<CLine*, CLine *> &edgeLineArray,
						   unsigned char *pUnchEdge, int width, int height);
	// ͨ���߽�����������֤�Ƿ��Ǻ���
	BOOL CheckLakeByEdge(int groupID, CArray<CLine*, CLine*> &edgeLineArray);
	// ��ȡ�߶��ϵĽǵ㲢�����cornerArray������
	void GetLineCorner(CLine* pLine,CArray<CPoint,CPoint> &cornerArray);
	// ͨ���ǵ�Ѱ��ֱ��
	void CDibObject::FindLine(CArray<CPoint,CPoint> &cornerArray,
		CArray<CFindedLine,CFindedLine> &findedLineArray, CArray<CDegree,CDegree> &degreeArray);
	// ͨ��ֱ�߳�����߽��߳��ȵı�ֵ����
	BOOL CheckByLineLenRate(CArray<CFindedLine,CFindedLine> &findedLineArray,double edgeLineLen);
	// ͨ��ƽ���߼���
	BOOL CheckByParallelLine(CArray<CFindedLine,CFindedLine> &findedLineArray,double edgeLineLen);
	// ͨ��Ѱ�ҵ���ֱ�߼нǼ���
	BOOL CheckByDegree(CArray<CDegree,CDegree> &degreeArray);
	// �õ��㵽ֱ�ߵľ���
	double GetPointLineJuLi(double A,double B,double C,CPoint point);
	// �õ�һ������ֱ���ϵ�ͶӰ
	CPoint GetTouYingPoint(double A,double B,double C,CPoint point);
	// ��֤����֤�������(�Ҷ�Ϊ1)�Ƿ�������֤�ı߽��(�Ҷ�Ϊ2)�ڲ�
	BOOL CheckInsidePoint(CPoint point, int groupID, CArray<CLine*, CLine*> &edgeLineArray,
								  unsigned char *pUnchEdge, int width, int height);
	// ��ȡ�����߶ε���С����
	void GetSmallRectOfLine(CLine *pLine, CRect &rect);
	// ��ȡ�����������С����
	void GetSmallRectOfArea(CPoint point, unsigned char *pUnchEdge, int width, int height, CRect &rect);
	// ȥ�����򼰱߽�
	void RemoveAreaAndEdgeByPoint(CPoint point, unsigned char *pUnchEdge, int width, int height, int gray);
	// ��֤�������ӵ��Ƿ�����Ҫ��
	BOOL CheckLakeSeed(CPoint lakeSeed);
	// ��֤����εĵ����γɵ������Ƿ�����Ч����
	BOOL CheckPolygonPoints(CArray<CPoint,CPoint> &polygonPoints);
	// ������α߽�
	void DrawPolygonLine(CArray<CPoint,CPoint> &polygonPoints);
	// �ж�һ�����Ƿ��ڶ�����ڲ�
	BOOL InsidePolygon(CPoint point, CArray<CPoint,CPoint> &polygonPoints);
	// ���ӵ�����㷨�������
	void FillArea(CPoint point);

	// ������
	// �������е�·�߽���
	void PaintAllRoadEdgeLine(unsigned char *pUnchEdge, int nWidth,
		CArray<CLine*,CLine*> &roadEdgeLineArray);
	// ���黭����·�߽���
	void PaintRoadEdgeLineByGroup(unsigned char *pUnchEdge, int width,
		CArray<CLine*,CLine*> &roadEdgeLineArray, int groupID);
	// ����������·�߽���
	void PaintOneRoadEdgeLine(unsigned char *pUnchEdge, int nWidth,
		CArray<CLine*,CLine*> &roadEdgeLineArray, int index);
	// ���߽�����Ϣ��¼���ı���
	void RecordRoadEdgeLineToText(CArray<CLine*,CLine*> &roadEdgeLineArray);
	void RecordWaterEdgeLineToText(CArray<CLine*,CLine*> &waterEdgeLineArray);
	// ��������Ϣ��¼���ı���
	void RecordGroupsToText(CGroup *pGroup, int lineNum);
	// �����ҵ���ֱ��
	void PaintFindLine(CArray<CFindedLine,CFindedLine> &findedLineArray);
	// ������
public:
	void GetMultiPoints(CArray<CPoint,CPoint> &polygonPoints,CArray<CPoint,CPoint> &pointArray);
	//�ֹ���ȡ��·��ˮ��
	void RecordPoints(CArray<CPoint,CPoint> &Points);
	void RecordPoints_wateredge_human(CArray<CPoint,CPoint> &Points);
	void RecordPoints_waterbody_human(unsigned char *pUnchEdge, int width, int height);

	//���Զ���ȡ��·��ˮ��
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
	// ������
	unsigned char *pGUnchEdge;
	// ������

	//����
//public:
//	int m_nWidth,m_nHeight;
	//����
};

#endif