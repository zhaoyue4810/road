#ifndef __STRUCTS_H__
#define __STRUCTS_H__

// ����ṹ��,���ڴ���ҵ���ֱ�ߵ������Ϣ
struct CFindedLine  // ֱ��Ϊy=kx+b
{
	CPoint startPoint;      // �������ֱ�ߵ�ǰ�˵�
	CPoint endPoint;        // �������ֱ�ߵĺ�˵�
	double length;          // ֱ�߳���
	double degree;          // ֱ����x��н�
};

struct CDegree
{
	CFindedLine findedLine1;
	double degree;
	CFindedLine findedLine2;
};

#endif