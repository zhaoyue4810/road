#ifndef __Group_H__
#define __Group_H__

class CGroup
{
public:
	CGroup();
	void CalBeliefDegree();  // 计算置信度

public:
	static double weightLen;  // 长度权值
	static double weightPara;  // 平行线权值
	static double weightEdge;  // 边界端点权值
	static double lowBeliefDegree;  // 最低置信度阀值
	int m_groupID;  // 分组编号
	double m_lineLength;  // 分组总线长
	int m_parallels;  // 分组包含的平行线数量
	int m_edgeLines;  // 分组包含的边界端点数量
	double m_beliefDegree;  // 该组置信度
	BOOL m_believed;  // 是否被舍弃
	CPoint m_seedPoint1, m_seedPoint2;  // 种子点
};

#endif