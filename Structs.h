#ifndef __STRUCTS_H__
#define __STRUCTS_H__

// 定义结构体,用于存放找到的直线的相关信息
struct CFindedLine  // 直线为y=kx+b
{
	CPoint startPoint;      // 用于拟合直线的前端点
	CPoint endPoint;        // 用于拟合直线的后端点
	double length;          // 直线长度
	double degree;          // 直线与x轴夹角
};

struct CDegree
{
	CFindedLine findedLine1;
	double degree;
	CFindedLine findedLine2;
};

#endif