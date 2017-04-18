#pragma once

/*!
 * @file SSNodeSelector.h
 * @date 2015/11/01 12:13
 *
 * @brief Shadowsocks 节点选择器. 会根据当前的选择算法选择一个SS节点出来使用
 *
 * 
 * @author Taro
 * Contact: sockscap64@gmail.com
 *
 *
 * @version 1.0
 *
 * @History
 * <author>      <time>      <version>         <desc>
 *
 * @TODO: long description
 *
 * @note
*/

#include "BaseDef.h"
#include <string>

using namespace std;

struct _random_algorithm{
	int id;
	char *name;
};

class CSSNodeSelector
{
public:
	CSSNodeSelector();
	~CSSNodeSelector();

protected:
	static int nLastSelectedAlgorithm;
	/** @brief 从上至下
	*/
	static CSSNodeInfo *_UpToDown();
	static int GenerateSSNodeIdList( int **idlist );
	/* @brief 随机
	*/
	//static CSSNodeInfo *_Random();
	/** @brief 低延迟优先
	*/
	//static CSSNodeInfo *_LowLatancyFirst();
	/** @brief 少错误优先
	*/
	//static CSSNodeInfo *_LessErrorsFirst();
public:
	/** @brief 根据当前算法( 是否启用服务器均衡 )选择一条SS节点使用
	*/
	static CSSNodeInfo *SelectNode();
	/** @brief 获得随机算法列表
	*/
	//static void GetRandomAlgorithms( vector<string> &algrothms );
};