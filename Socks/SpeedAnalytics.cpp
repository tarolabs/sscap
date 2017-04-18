#include "stdheader.h"
#include "SpeedAnalytics.h"

CSpeedAnalytics::CSpeedAnalytics()
{
	reset();
}
CSpeedAnalytics::~CSpeedAnalytics()
{

}
void CSpeedAnalytics::reset()
{
	m_firstts = 0;
	m_lastts = 0;
	m_lastspeed = 0;
	m_lastspeedval = 0;
	memset(m_counting, 0, sizeof(m_counting));
}
void CSpeedAnalytics::PushData(int len)
{
	unsigned int now = (unsigned int) time(NULL);
	
	if (now - m_lastts > MAX_AVE_SPEED) {
		memset(m_counting, 0, sizeof(m_counting));
		m_firstts = m_lastts = now;
	} else {
		while (m_lastts + 1 <= now) {
			m_counting[(m_lastts + 1) % MAX_AVE_SPEED] = 0;
			m_lastts++;
		}
	}
	if (!m_firstts)
		m_firstts = now - 1;
	m_counting[now % MAX_AVE_SPEED] += len;
}
unsigned int CSpeedAnalytics::GetSpeed() 
{
	unsigned int now = (unsigned int) time(NULL);
	if (now != m_lastspeed) {
		//PushData(0);
		m_lastspeed = now;
		unsigned int sum = 0, i;
		unsigned int seconds = 0;
		for (i = 0; i < MAX_AVE_SPEED; i++)
		{
			sum += m_counting[i];

			if( m_counting[i] > 0 )
				seconds ++;
		}
		if( seconds == 0 ) seconds  = 1;
		i = min((unsigned int) MAX_AVE_SPEED, seconds );
		m_lastspeedval = sum / i;
	}
	return m_lastspeedval;
}

/** @brief 返回kb/s的速度*/
unsigned int CSpeedAnalytics::GetKBSpeed()
{
	unsigned int bytes_per_s = GetSpeed();

	if( bytes_per_s <= 0 ) return 0;

	return bytes_per_s / 1024;
}