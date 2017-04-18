#include "stdheader.h"
#include "SSNodeSelector.h"
#include "Utils.h"
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_RANDOM_ALGORITHM 4

static struct _random_algorithm _algorithms[] ={
	{ 0, "up to down"},
	{ 1, "random"},
	{ 2, "low latancy first"},
	{ 3, "less errors first"},
	{ -1, NULL	}
};

//int CSSNodeSelector::nLastSelectedAlgorithm = 0;

CSSNodeSelector::CSSNodeSelector()
{

}

CSSNodeSelector::~CSSNodeSelector()
{

}
/** @brief 将ss节点的vector索引id保存到idlist, 除去disable的节点
*
* @return 返回个数
*/
int CSSNodeSelector::GenerateSSNodeIdList( int **idlist )
{
	CSSConfigInfo *pCfg = GetConfigInfo();
	int size = pCfg->ssNodeInfo.size();
	if( size <= 0 ) return 0;

	int nValidSize = 0;
	for( int i = 0 ; i < size ; i ++ )
	{
		if( pCfg->ssNodeInfo[i]->enable )
			nValidSize ++;
	}

	int *l = new int[nValidSize];
	for( int i = 0, idx = 0 ; i < size ; i ++ )
	{
		if( pCfg->ssNodeInfo[i]->enable )
		{
			l[ idx ++  ] = i;
		}
	}

	*idlist = l;

	return nValidSize;
}
CSSNodeInfo *CSSNodeSelector::_UpToDown()
{
	CSSConfigInfo *pCfg = GetConfigInfo();
	int *idlist = NULL;
	int size = GenerateSSNodeIdList( &idlist );
	if( size <= 0 ) return NULL;

	srand( time( NULL ) );
	int r = rand() % size;

	int nSelectId = idlist[r];

	delete []idlist;

	if( nSelectId < 0 || nSelectId >= pCfg->ssNodeInfo.size() ) return NULL;

	return pCfg->ssNodeInfo[nSelectId];
}
/*
CSSNodeInfo *CSSNodeSelector::_Random()
{
	CSSConfigInfo *pCfg = GetConfigInfo();
	int nCount = pCfg->ssNodeInfo.size();
	if( nCount <= 0 ) return NULL;
	if( nCount == 1 ) return pCfg->GetNodeBySerial( 0 );

	srand( time( NULL ) );

	int r = rand() % nCount;

	return pCfg->GetNodeBySerial( r );
}

CSSNodeInfo *CSSNodeSelector::_LowLatancyFirst()
{
	return NULL;
}
CSSNodeInfo *CSSNodeSelector::_LessErrorsFirst()
{
	return NULL;
}
*/
/** @brief 根据当前算法( 是否启用服务器均衡 )选择一条SS节点使用
*/
CSSNodeInfo *CSSNodeSelector::SelectNode()
{
	CSSConfigInfo *pCfg = GetConfigInfo();

	// 启用了随机选择
	if( pCfg->random )
	{
		return CSSNodeSelector::_UpToDown();

		/*
		if( pCfg->randomAlgorithm < 0 || pCfg->randomAlgorithm >= MAX_RANDOM_ALGORITHM )
			return pCfg->GetActiveNodeInfo();

		if( pCfg->randomAlgorithm == 0 )
			return CSSNodeSelector::_UpToDown();
		else if( pCfg->randomAlgorithm == 1 )
			return CSSNodeSelector::_Random();
		else if( pCfg->randomAlgorithm == 2 )
			return CSSNodeSelector::_LowLatancyFirst();
		else if( pCfg->randomAlgorithm == 3 )
			return CSSNodeSelector::_LessErrorsFirst();
		else 
			return pCfg->GetActiveNodeInfo();
			*/
	}
	else 
	{
		return pCfg->GetActiveNodeInfo();
	}

	return NULL;
}
/** @brief 获得随机算法列表
*/
/*
void CSSNodeSelector::GetRandomAlgorithms( vector<string> &algrothms )
{
	int i = 0;
	while( _algorithms[i].id != -1 )
	{
		algrothms.push_back( lm_u82u16_s( _( _algorithms[i].name ) ) );

		i++;
	} 

	return;
}
*/