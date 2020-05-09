/**
 * @file HListBox.h
 * @brief �������Ľ��б�ؼ�
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
 * @brief �������Ľ��б�ؼ�
 */
class CHListBox :
	public CListBox
{
public:
	CHListBox(void);
public:
	~CHListBox(void);

	// ���Ǹ÷����Ա����ˮƽ������
	/**
	 * @brief ���һ����
	 *
	 * @param lpszItem ��
	 *
	 * @return ������
	 */
 int AddString( LPCTSTR lpszItem );
 /**
  * @brief ����һ����
  *
  * @param nIndex ����λ��
  * @param lpszItem ��
  *
  * @return ������
  */
 int InsertString( int nIndex, LPCTSTR lpszItem );

// ����ˮƽ���������
 void RefushHorizontalScrollBar( void );
};
