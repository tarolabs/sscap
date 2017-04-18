#pragma once

/*!
 * @file SSManager.h
 * @date 2015/10/19 13:35
 *
 * @brief shadowsocks的管理类, 实现所有的ss管理罗辑, 如启动, 停止, 切换服务器.
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
#include "Listener.h"

class CListener;
class CSSManager : public CListener
{
public:
	CSSManager();
	virtual ~CSSManager();

public:
	/** @brief 开始服务
	*/
	BOOL StartServices( );
	/** @brief 停止服务
	*/
	void StopServices();
public:
};
