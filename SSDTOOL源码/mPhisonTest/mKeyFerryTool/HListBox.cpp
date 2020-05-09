/**
 * @file HListBox.cpp
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
#include "StdAfx.h"
#include "HListBox.h"

CHListBox::CHListBox(void)
{
}

CHListBox::~CHListBox(void)
{
}
int CHListBox::AddString( LPCTSTR lpszItem )
 {
 int nResult = CListBox::AddString( lpszItem );

RefushHorizontalScrollBar();

return nResult;
 }

int CHListBox::InsertString( int nIndex, LPCTSTR lpszItem )
 {
 int nResult = CListBox::InsertString( nIndex, lpszItem );

RefushHorizontalScrollBar();

return nResult;
 }

void CHListBox::RefushHorizontalScrollBar( void )
 {
 CDC *pDC = this->GetDC();
 if ( NULL == pDC )
 {
    return;
 }

 int nCount = this->GetCount();
 if ( nCount < 1 )
 {
    this->SetHorizontalExtent( 0 );
    return;
 }

int nMaxExtent = 0;
 CString szText;
 for ( int i = 0; i < nCount; ++i )
 {
    this->GetText( i, szText );
    CSize &cs = pDC->GetTextExtent( szText );
    if ( cs.cx > nMaxExtent )
    {
     nMaxExtent = cs.cx;
    }
 }

this->SetHorizontalExtent( nMaxExtent );
 }
