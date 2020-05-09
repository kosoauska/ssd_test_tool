/**
 * @file HListBox.h
 * @brief 滚动条改进列表控件
 * @author Wang Pu
 * @version 0.0.0.1
 * @date 2012-05-07
 */
/* <pre><b>Copyright (c)</b></pre>
 * 
 * <pre><b>SCSemicon Co.,Ltd. 2009-2012. All rights reserved</b></pre>
 * <pre><b>email: </b>pu.wang@scsemicon.com</pre>
 * <pre><b>company: </b>http://www.scsemicon.com/</pre>
 * 
 */
#pragma once
#include "afxwin.h"

/**
 * @brief 滚动条改进列表控件
 */
class CHListBox :
	public CListBox
{
public:
	CHListBox(void);
public:
	~CHListBox(void);

	// 覆盖该方法以便添加水平滚动条
	/**
	 * @brief 添加一个项
	 *
	 * @param lpszItem 项
	 *
	 * @return 项索引
	 */
 int AddString( LPCTSTR lpszItem );
 /**
  * @brief 插入一个项
  *
  * @param nIndex 插入位置
  * @param lpszItem 项
  *
  * @return 项索引
  */
 int InsertString( int nIndex, LPCTSTR lpszItem );

// 计算水平滚动条宽度
 void RefushHorizontalScrollBar( void );
};
