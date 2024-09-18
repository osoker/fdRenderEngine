//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: stringInterpret.cpp
 *
 *  His:      River created @ 2004-12-18
 *
 *  Des:      对客户端传进来要渲染的字串进行解晰，得到底层可以接受的字串格式。　
 *   
 *
 *  "自非圣人，外宁必有内忧"
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../../Terrain/include/fdFieldMgr.h"
# include "../include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include "../../mfpipe/include/osCharacter.h"
#include "../../mfpipe/include/Font.h"



//! 最小颜色解晰字串的长度。
# define    MIN_CISTRLEN   8


//! 得到一个解晰符单位的颜色,如果返回纯黑色纯透明的字串，则出错。
DWORD osc_szCI::get_strColor( const char* _sz,int _idx,int& _cnum )
{
	guard;
	char       t_ch;
	char       t_szKeyWord[12];
	int        t_iCidx = _idx;

	t_ch = _sz[t_iCidx];
	if( t_ch != osc_szCI::m_cSelCharS )
	{
		osDebugOut( " < 号导致以下字串格式出错:<%s>..\n",_sz );
		return 0;
	}
	t_iCidx ++;
	t_ch = _sz[t_iCidx];
	if( t_ch != osc_szCI::m_cSelChar )
	{
		osDebugOut( " & 号导致以下字串格式出错:<%s>..\n",_sz );
		return 0;
	}
	t_iCidx ++;

	int t_i = 0;
	_cnum = 2;
	while( _sz[t_iCidx] != ':' )
	{
		t_szKeyWord[t_i] = _sz[t_iCidx];
		t_iCidx ++;
		t_i ++ ;

		_cnum ++;

		// River mod @ 2007-3-23:加入容错机制,但字符显示可以出错.
		if( t_i > 8 )
			return 0xffffffff;

		osassertex( (t_i<=8),"字串颜色符号有问题...\n" );
	}
	_cnum ++;
	t_szKeyWord[t_i] = NULL;

	DWORD   t_dw = 0xffffffff;
	sscanf( t_szKeyWord,"%X",&t_dw );

	// 
	// 文字全透明则使用不透明。
	if( (t_dw & 0xff000000)== 0 )
		t_dw |= 0xff000000;

	return t_dw;

	unguard;
}


/** \brief
 *  返回值为当前的显示字串使用了多少个字符，包括解晰符。
 *
 *  算法描述：
 *  1: 得到当前字串部分用到的颜色。
 *  2: 
 *  
 */
int osc_szCI::get_disStr( os_stringDisp* _resSD,int& _strIdx,
					int& _xOffset,int& _yOffset,os_colorInterpretStr* _srcCI,int _cidx,int _TextSize )
{
	guard;

	int     t_iChrNum,t_iCIdx,t_iBkx;
	static  char t_szBuf[1024];
	int     t_iWideCharNum;
	int     t_iCnum;

	

	osassert( _cidx >= 0 );
	t_iChrNum = _cidx;
	_resSD[_strIdx].color = osc_szCI::get_strColor
		( _srcCI->get_ciStr(),_cidx,t_iCnum );
	// 如果字串的格式出错，返回
	if( _resSD[_strIdx].color == 0 )
		return -1;

	t_iChrNum += t_iCnum;

	t_iCIdx = 0;
	t_iBkx = _xOffset;
	t_iWideCharNum = 0;

	int t_iTotalCharNum = strlen( _srcCI->get_ciStr() );
	int t_i;
	bool t_bMultibyte = false;
	for( t_i=0;t_i<t_iTotalCharNum;t_i ++ )
	{
		t_szBuf[t_iCIdx] = _srcCI->get_ciStr()[t_iChrNum];
		t_iChrNum ++;

		//
		// 发现当前解晰单位的结束符。
		if( (t_szBuf[t_iCIdx] == osc_szCI::m_cSelChar) )
		{
			if( _srcCI->get_ciStr()[t_iChrNum] == osc_szCI::m_cSelCharE )
			{
				t_iChrNum ++;
				t_szBuf[t_iCIdx] = NULL;
				break;
			}
		}

		osassertex( (t_szBuf[t_iCIdx] != NULL),(char*)_srcCI->get_ciStr() );

# if 0
		// 记录汉字的字符数目。
		if( t_szBuf[t_iCIdx] & 0x80 )
			t_iWideCharNum ++;
# else
		// 记录汉字的字符数目。
		if( ::IsDBCSLeadByte( t_szBuf[t_iCIdx] ) )
		{
			if( t_bMultibyte )
				t_bMultibyte = false;
			else
			    t_bMultibyte = true;
		}
		else
			t_bMultibyte = false;
# endif

		t_iCIdx ++;

		//
		// 如果到达了边界，则自动换行
		// 如果发现了换行符，也需要在此换行。
		_xOffset += (_TextSize/2);
		if( _srcCI->m_iMaxPixelX>0 )
		{
			if( (t_szBuf[t_iCIdx-1] == '\n' ) ||
				(_xOffset>(_srcCI->m_iMaxPixelX-_TextSize/2)) )
			{
				// 如果当前处于汉字字符的中间。
# if 0
				if( (t_iWideCharNum % 2)==1 )
# else
		        if( t_bMultibyte )		
# endif 
				{
					t_iCIdx --;
					t_iChrNum --;

					// River added @ 2006-7-28:为了使用循环可以顺利完成
					t_i --;

					t_iWideCharNum = 0;
					t_bMultibyte = false;
				}

				// River @ 2007-3-26: 如果同时到达右边界和出现回车,略去回车符
				t_szBuf[t_iCIdx] = _srcCI->get_ciStr()[t_iChrNum];
				if( t_szBuf[t_iCIdx] == '\n' )
					t_iChrNum ++;


				t_szBuf[t_iCIdx] = NULL;
				_resSD[_strIdx] = t_szBuf;
				_resSD[_strIdx].x = t_iBkx;
				_resSD[_strIdx].y = _yOffset;
				_resSD[_strIdx].charHeight = _TextSize;
				

				t_iCIdx = 0;
				_xOffset = _srcCI->m_iPosX;
				t_iBkx = _xOffset;
				_strIdx ++;

				osassert( _strIdx < MAX_STRDIS_PERCI );//fixme: 

				_resSD[_strIdx].color = _resSD[_strIdx-1].color;
				_yOffset += _TextSize;
				_yOffset += _srcCI->m_iRowDis;

			}
		}
	}

	
	_resSD[_strIdx] = t_szBuf;
	_resSD[_strIdx].x = t_iBkx;
	_resSD[_strIdx].y = _yOffset;
	_resSD[_strIdx].charHeight = _TextSize;

	return t_iChrNum;


	unguard;
}


//! 全局的颜色关键字配置文件。
char          osc_szCI::m_cSelChar =  '&';
char          osc_szCI::m_cSelCharS = '<';
char          osc_szCI::m_cSelCharE = '>';


//! 解释颜色解晰字串到可以显示的字串。　
int  osc_szCI::interpret_strDisp( os_stringDisp* _resSD,
								 os_colorInterpretStr* _srcCI,int& _iresNum ,int _fontType)
{
	guard;

	//CWFont* pFont = gFontManager.GetFont(_fontType);
	_resSD->charHeight = _srcCI->charHeight;
	int        t_iStrLen,t_iChrIdx;

	
	t_iStrLen = (int)strlen( _srcCI->get_ciStr() );
	if( t_iStrLen == 0 )
		return 0;
	//osassert( t_iStrLen>MIN_CISTRLEN );
	if( t_iStrLen<=MIN_CISTRLEN )
	{
		osDebugOut( "interpret_strDisp:<%s>\n", _srcCI->get_ciStr() );
		osassert( false );
	}



	t_iChrIdx = 0;
	_iresNum = 0;


	int   t_iPixelXOffset = _srcCI->m_iPosX;
	int   t_iPixelYOffset = _srcCI->m_iPosY;
	while( 1 )
	{
		if( (t_iStrLen - t_iChrIdx)<MIN_CISTRLEN )
			break;

		osassert( _iresNum < MAX_STRDIS_PERCI );
       t_iChrIdx = osc_szCI::get_disStr( _resSD,_iresNum,
			t_iPixelXOffset,t_iPixelYOffset,_srcCI,t_iChrIdx,_srcCI->charHeight );

	   // 解析字串的过程出错
	   if( -1 == t_iChrIdx )
		   break;

		_iresNum ++;
	}

	_srcCI->m_iResRowNum = _iresNum;

	if( (_srcCI->m_iMaxPixelX>0)&&
		((t_iPixelYOffset+_srcCI->charHeight)>_srcCI->m_iMaxPixelY) )
	{
		// 如果不是强制显示，直接返回。
		if( !_srcCI->m_bForceDis )
			return -1;

		for( int t_i=0;t_i<_iresNum;t_i ++ )
		{
			if( (_resSD[t_i].y+_srcCI->charHeight)>_srcCI->m_iMaxPixelY )
				return t_i;
		}

		return -1;
			
	}
	else
		return _iresNum;
	
	unguard;
}

