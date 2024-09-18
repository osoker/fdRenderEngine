//=======================================================================================================
/**  \file
 *   Filename: osConsole.cpp
 *   Desc:     Osok引擎中的console类.
 *
 *   His:      River created @ 5/23 2003
 *
 *   "一个团队里,不能让核心技术只掌握在一个人手中.永远都应该有一个替补队员."
 *  
 */
//=======================================================================================================
#include "stdafx.h"
# include "../include/osConsole.h"
# include "../../mfpipe/include/deviceInit.h"

# if 0

# define UNKNOWN_COMMAND     "  UNKNOWN  COMMAND!!!" 
# define SUCCESS_COMMAND     "  SUCCESS  COMMAND!!!"
# define FAILED_COMMAND      "  FAILED COMMAND!!!"
# define SUCCESS_SETVAR      "  SUCCESS SET VARIABLE!!!"
# define FAILED_SETVAR       "  FAILED SET VARIABLE!!!"
# define SUCCESS_GETVAR      "  SUCCESS GET VARIABLE!!!"
# define FAILED_GETVAR       "  FAILED GET VARIABLE!!!"

osc_console::osc_console()
{
	m_vecConsoleVar.resize( 24 );
	this->m_iCVarNum = 0;

	m_vecConsoleCmd.resize( 24 );
	this->m_iCCmdNum = 0;

	m_iFirstLine = 0;
	m_iTotalDispLines = 0;

	m_bActive = false;


}

osc_console::~osc_console()
{

}






/** \brief
*  Console的初始化函数.
*
*  所有console绘制工作都在当前的console的内部完成.
*/
bool osc_console::init_console( I_deviceManager* _dev,int _capdisLines )
{
	guard;// osc_console::init_console() );

	osc_d3dManager*  t_devMgr;
	RECT             t_rect;

	osassert( _dev );
	osassert( _capdisLines>0 );

	this->m_deviceMgr = _dev;

	t_devMgr = (osc_d3dManager*)_dev;

	this->m_dispRect.left = 0;
	this->m_dispRect.top = 0;
	t_rect = t_devMgr->get_windowBounds();
	m_dispRect.right = t_rect.right-t_rect.left;
	m_dispRect.bottom = t_rect.bottom - t_rect.top;

	// 使用全屏幕的3/4高度显示console.
	// 并且每一行的高度要确定,使用整个console可显示一定行数的命令
	// 或是结果.
	m_dispRect.bottom = (m_dispRect.bottom/4)*3;

	m_iCapDispLines = _capdisLines;
	m_iTotalDispLines = 0;
	m_vecConsoleDisp.resize( m_iCapDispLines );
	m_iPerLineHeight = int(m_dispRect.bottom/float(m_iCapDispLines+1));

	// debug code.
	char       t_c[128];
	s_string   t_str;

	sprintf( t_c,"%s%d","PerLine height:",m_iPerLineHeight );
	t_str = t_c;
	t_str += "    ";
	sprintf( t_c,"%s%d","Total height:",m_dispRect.bottom );
	t_str += t_c;
	add_dispString( t_str );

	m_devStringDisp.resize( m_iCapDispLines+1 );

	return true;

	unguard;
}

/** \brief
*  Console的变量注册函数.
*/
void osc_console::register_cvar( os_consoleVar& _var )
{
	guard;// osc_console::register_cvar() );

	int    t_i;

	for( t_i=0;t_i<m_iCVarNum;t_i++ )
	{
		if( m_vecConsoleVar[t_i].varName == _var.varName )
			return;
	}

	t_i = (int)m_vecConsoleVar.size();
	if( m_iCVarNum >= t_i )
		m_vecConsoleVar.resize( t_i*2 );


	m_vecConsoleVar[m_iCVarNum].varName = _var.varName;
	m_vecConsoleVar[m_iCVarNum].varType = _var.varType;
	m_vecConsoleVar[m_iCVarNum].varValPtr = _var.varValPtr;

	m_iCVarNum ++;

	return ;

	unguard;
}



/** \brief 
*   为引擎中的console注册命令函数.
*/  
void osc_console::register_cmd( os_consoleCmd& _cmd )
{
	guard;// osc_console::register_cmd() );

	int     t_i;

	for( t_i=0;t_i<m_iCCmdNum;t_i++ )
	{
		if( m_vecConsoleCmd[t_i].cmdName == _cmd.cmdName )
			return;
	}

	t_i = (int)this->m_vecConsoleCmd.size();
	if( this->m_iCCmdNum >= t_i )
		m_vecConsoleCmd.resize( t_i*2 );

	
	m_vecConsoleCmd[m_iCCmdNum].cmdFuncPtr = _cmd.cmdFuncPtr;
    m_vecConsoleCmd[m_iCCmdNum].cmdName = _cmd.cmdName;

	m_iCCmdNum ++;

	return ;

	unguard;
}




/** \brief
*  Console的输入函数.
*
*  如果console激活并输入成功,返回真,否则返回false.
*/
bool osc_console::input_char( WORD  _c )
{
	guard;// osc_console::input_char() );

	bool    t_b;
	char    t_c;

	// 如果是Reture,则需要执行当前行的命令.
	if( _c == 192 )
	{
		active_console( !m_bActive );
		return true;
	}


	if( !m_bActive )
		return false;


	if( _c == 13 )
	{
		t_b = process_command( m_strInput );
		osassert( t_b );

		m_strInput = "";
		return true;
	}
	else if( ((_c>=65)&&(_c<=90))  ||
		     ((_c>=97)&&(_c<=122)) ||
			((_c>=48)&&(_c<=57) )     ) 
	{
		t_c = (char)_c;
		if( (t_c>= 65)&&(_c<=90) )
			t_c += 32;
		m_strInput.append( 1,t_c );

		return true;
	}	 

	// 输入单个的特殊字符.
	switch( _c )
	{
	case 8:
		if( !m_strInput.empty() )
			m_strInput.erase( m_strInput.end()-1 );
		return true;
	case 32:
		m_strInput.append( 1,(char)_c );
		return true;
	case 191:
		m_strInput.append( 1,'/' );
		return true;
	case 220:
		m_strInput.append( 1,'\\' );
		return true;
	case 186:
		m_strInput.append( 1,':' );
		return true;
	case 189:
		m_strInput.append( 1,'_' );
		return true;
	case 190:
		m_strInput.append( 1,'.' );
		return true;
	default:
		return false;

	}
		


	return false;

	unguard;
}


/** \brief
*  在显示队列中加入一行显示字串.
*/
void osc_console::add_dispString( s_string& _dispstr )
{
	guard;// osc_console::add_dispString() );

	if( m_iFirstLine == m_iCapDispLines )
		m_iFirstLine = 0;

	m_vecConsoleDisp[m_iFirstLine] = _dispstr ;
	
	if( m_iTotalDispLines<m_iCapDispLines )
		m_iTotalDispLines++;

	m_iFirstLine ++;

	unguard;
}

/** \brief
*  解释当前行的命令.
*
*  当前命令可能是已经注册的命令或是得到设置变量的命令.
*  把这串命令解释到类的内部变量中,供以后的函数使用.
*  如果不是Set和Get命令,在命令列表中也找不到当前的命令,
*  函数返回false;
*/
bool osc_console::parse_command( s_string& _cmd )
{
	guard;// osc_console::parse_command() );

	s_string::size_type   t_pos;
	s_string              t_str;
	int                   t_i;

	// process the set command.
	t_str = SET_VAR_CMD;
	t_str.append( 1,' ' );
	t_pos = _cmd.find( t_str );

	if( t_pos == 0 )
	{
		this->m_cmdType = CMD_SET;

		this->m_strVarName = _cmd;
		m_strVarName.erase( 0,t_str.length() );
		while( m_strVarName[0] == ' ' )
		{
			m_strVarName.erase(0,1 );
		}

		m_strPara = m_strVarName;
		t_pos = m_strVarName.find( ' ' );
		if( t_pos < m_strVarName.length() )
		{
			m_strVarName.erase( m_strVarName.begin() + t_pos,
			                    m_strVarName.end() );
		}
		else
			return false;

		m_strPara.erase( 0,m_strVarName.length() );
		while( m_strPara[0]  == ' ' ) 
			m_strPara.erase( 0,1 );
		t_pos = m_strPara.find( ' ' );
		if( t_pos< m_strPara.length() )
		{
			m_strPara.erase( m_strPara.begin() + t_pos,
			                 m_strPara.end() );
		}

		return true;
	}

	// process the get command.
	t_str = GET_VAR_CMD;
	t_str.append( 1,' ' );
	t_pos = _cmd.find( t_str );
	if( t_pos == 0 )
	{
		this->m_cmdType = CMD_GET;

		this->m_strVarName = _cmd;
		m_strVarName.erase( 0,t_str.length() );
		while( m_strVarName[0] == ' ' )
		{
			m_strVarName.erase( 0,1 );
		}

		t_pos = m_strVarName.find( ' ' );
		if( t_pos< m_strVarName.length() )
		{
			m_strVarName.erase( m_strVarName.begin() + t_pos,
			                 m_strVarName.end() );
		}

		return true;
	}

	// process the custom command.
	for( t_i=0;t_i<this->m_iCCmdNum;t_i++ )
	{
		t_str = this->m_vecConsoleCmd[t_i].cmdName;

		t_pos = _cmd.find( t_str );
		if( t_pos == 0 )
		{
			this->m_cmdType = CMD_CUSTOM;
			this->m_iIdxCusCmd = t_i;

			m_strPara = _cmd;
			m_strPara.erase( 0,t_str.length() );
			while( m_strPara[0]==' ' )
			{
				m_strPara.erase( 0,1 );
			}

			return true;
		}
	}


	return false;



	unguard;
}


/** \brief
*  使用已经提取好的参数设置一个变量.
*/
bool osc_console::set_cvar( void )
{
	guard;// osc_console::set_cvar() );

	int         t_i;

	for( t_i = 0;t_i<m_iCVarNum;t_i++ )
	{
		if( m_vecConsoleVar[t_i].varName == m_strVarName )
		{
			switch( m_vecConsoleVar[t_i].varType )
			{
			case CVAR_FLOAT:
				*(float*)m_vecConsoleVar[t_i].varValPtr = (float)atof( this->m_strPara.c_str() );
				return true;
			case CVAR_INT:
				*(int*)m_vecConsoleVar[t_i].varValPtr = atoi( this->m_strPara.c_str() );
				return true;
			case CVAR_BOOL:
				if( m_strPara == "false" )
					*(bool*)m_vecConsoleVar[t_i].varValPtr = false;
				else if( m_strPara == "true" )
					*(bool*)m_vecConsoleVar[t_i].varValPtr = true;
				else
					*(bool*)m_vecConsoleVar[t_i].varValPtr = atoi( this->m_strPara.c_str() );
				return true;
			default:
				osassert( false );
			}
		}
	}


	return false;

	unguard;
}

/** \brief
*  使用已经提取好的参数输入到console一个变量的值.
*/
bool osc_console::get_cvar( void )
{
	guard;// osc_console::get_cvar() );

	s_string      t_str;
	char          t_c[128];
	int           t_i;

	for( t_i = 0;t_i<m_iCVarNum;t_i++ )
	{
		if( m_vecConsoleVar[t_i].varName == m_strVarName )
		{
			t_str = m_strVarName;
			t_str.append( 4,' ' );
			t_str += "Var TYPE IS<";
			switch( m_vecConsoleVar[t_i].varType )
			{
			case CVAR_FLOAT:
				t_str += "float>";
				break;
			case CVAR_INT:
				t_str += "int>";
				break;
			case CVAR_BOOL:
				t_str += "bool>";
				break;

			}

			t_str.append(4,' ' );
			t_str += "Var VALUE is:<";
			switch( m_vecConsoleVar[t_i].varType )
			{
			case CVAR_FLOAT:
				sprintf( t_c,"%f%c",*(float*)m_vecConsoleVar[t_i].varValPtr,'>' );
				t_str += t_c;
				break;
			case CVAR_INT:
				sprintf( t_c,"%d%c",*(int*)m_vecConsoleVar[t_i].varValPtr,'>' );
				t_str += t_c;
				break;
			case CVAR_BOOL:
				if( *(bool*)m_vecConsoleVar[t_i].varValPtr )
				{
					sprintf( t_c,"true>" );
				}
				else
				{
					sprintf( t_c,"false>" );
				}
				t_str += t_c;
				break;
			}

			add_dispString( t_str );

			return true;
		}

	}
	
	return false;

	unguard;
}



/** \brief
*  处理整行输入的函数,输入字串转化为命令后由其它函数进行处理.
*/
bool osc_console::process_command( s_string& _cmdstr )
{
	guard;// osc_console::process_command() );

	bool             t_b;
	s_string         t_str;


	t_b = parse_command( _cmdstr );
	if( !t_b )
	{
		t_str = UNKNOWN_COMMAND;
		add_dispString( t_str );

		add_dispString( _cmdstr );

		return true;
	}

	if( this->m_cmdType == CMD_CUSTOM )
	{
		t_b = m_vecConsoleCmd[m_iIdxCusCmd].cmdFuncPtr( m_strPara.c_str() );
		if( t_b )
		{
			t_str = SUCCESS_COMMAND;
			add_dispString( t_str );
		}
		else
		{
			t_str = FAILED_COMMAND;
			add_dispString( t_str );
		}

		add_dispString( _cmdstr );

		return true;
	}

	if( m_cmdType == CMD_SET )
	{
		t_b = set_cvar();
		if( !t_b ) 
		{
			t_str = FAILED_SETVAR;

			add_dispString( t_str );

			add_dispString( _cmdstr );
		}
		else
		{
			t_str = SUCCESS_SETVAR;
			
			add_dispString( t_str );

			add_dispString( _cmdstr );
		}

	}
	if( m_cmdType == CMD_GET )
	{
		t_b = get_cvar();
		if( !t_b )
		{
			t_str = FAILED_GETVAR;

			add_dispString( t_str );

			add_dispString( _cmdstr );
		}
		else
		{
			add_dispString( _cmdstr );
		}
	}

	return true;

	unguard;
}



/** \brief
*  console的显示函数.
*
*  先画console的透明的背景,然后再画要在console上显示的字串.
*/
bool osc_console::draw_console( void )
{
	guard;// osc_console::draw_console() );


	int           t_i,t_idpos,t_disPtr;
	static char   s_curInput[128];

	if( !m_bActive )
		return true;

	//
	// Draw the background.
	//

	//
	// Draw the text.
	//
	t_idpos = 0;
	t_disPtr = 0;
	
	// 显示当前正在输入的行.
	strcpy( s_curInput,">: " );
	strcat( s_curInput,m_strInput.c_str() );
	m_devStringDisp[t_disPtr].charHeight = m_iPerLineHeight;
	m_devStringDisp[t_disPtr].color = RGB( 255,255,255 );
	m_devStringDisp[t_disPtr] = s_curInput;
	m_devStringDisp[t_disPtr].x = 0;
	m_devStringDisp[t_disPtr].y = t_idpos;	

	t_idpos += m_iPerLineHeight;
	t_disPtr ++;

	if( m_iFirstLine > 0 )
	{
		for( t_i=m_iFirstLine-1;t_i>=0;t_i-- )
		{
			m_devStringDisp[t_disPtr].charHeight = m_iPerLineHeight;
			m_devStringDisp[t_disPtr].color = RGB( 255,255,255 );
			m_devStringDisp[t_disPtr] = m_vecConsoleDisp[t_i].c_str();
			m_devStringDisp[t_disPtr].x = 0;
			m_devStringDisp[t_disPtr].y = t_idpos;

			t_idpos += m_iPerLineHeight;
			t_disPtr ++;
		}
	}

	if( (m_iTotalDispLines == m_iCapDispLines) )
	{
		for( t_i=m_iTotalDispLines-1;t_i>=m_iFirstLine;t_i-- )
		{
			m_devStringDisp[t_disPtr].charHeight = m_iPerLineHeight;
			m_devStringDisp[t_disPtr].color = RGB( 255,255,255 );
			m_devStringDisp[t_disPtr] = m_vecConsoleDisp[t_i].c_str();
			m_devStringDisp[t_disPtr].x = 0;
			m_devStringDisp[t_disPtr].y = t_idpos;

			t_idpos += m_iPerLineHeight;
			t_disPtr ++;
		}
	}

	t_i = m_deviceMgr->disp_string( &m_devStringDisp[0],m_iTotalDispLines+1 );
	if( t_i>= 0 )
		return true;
	else
		return false;


	unguard;
}

# endif 