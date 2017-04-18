#pragma once

/** @brief 用于统计SOCKET传输速度
*/
#define MAX_AVE_SPEED 2

class CSpeedAnalytics
{
public:
	CSpeedAnalytics();
	virtual ~CSpeedAnalytics();
	void reset();
	void PushData(int len);
	/** @brief 返回bytes/s的速度*/
	unsigned int GetSpeed();
	/** @brief 返回kb/s的速度*/
	unsigned int GetKBSpeed();
protected:
	unsigned int m_lastspeed;
	unsigned int m_lastspeedval;
	unsigned int m_firstts;
	unsigned int m_lastts;
	unsigned int m_counting[MAX_AVE_SPEED];
};
