//=======================================================================================================
/**  \file
 *   Filename: osConsole.h
 *   Desc:     Osok引擎中的console类,这个console在引擎外部只能注册变量,
 *             所有的console命令都是引擎自己的命令,只能使用,不能加入新的命令.
 *
 *   His:      River created @ 5/23 2003
 *
 *   "一个团队里,不能让核心技术只掌握在一个人手中.永远都应该有一个替补队员."
 *  
 */
//=======================================================================================================
# pragma once

# include "../../interface/osInterface.h"

# if 0

//@{
/** \brief
 *  得到和设置变量的命令.
 */
# define SET_VAR_CMD   "set"
# define GET_VAR_CMD   "get"
//@}




//@{
/** \brief
 *  定义console中常用的vector.
 */
typedef std::vector<os_consoleVar>  VEC_consoleVar;
typedef std::vector<os_consoleCmd>   VEC_consoleCmd;
typedef std::vector<os_stringDisp>   VEC_strDisp;
//@}

/** \brief
 *  console中的命令类型.
 */
enum ose_CmdType
{
	CMD_SET,
	CMD_GET,
	CMD_CUSTOM
};



/** \brief
 *  OSOK引擎的console类,这个运行了osok引擎console的所有的功能.
 *
 *  通过ResourceManager向外部公开接口.
 *  Console 的功能介绍:
 *  1: 保存内部的命令和变量.
 *  2: 外部可以注册变量.
 *  3: 处理命令和变量的改变.
 *  4: 处理输入.
 *  5: 显示console.
 *  
 *  Console不区分大小写,显示时一全部都使用大写(或小写).
 *
 */
class osc_console
{
private:
	//! console variable的vector.
	VEC_consoleVar    m_vecConsoleVar;
	int               m_iCVarNum;

	//! console command 的vector.
	VEC_consoleCmd    m_vecConsoleCmd;
	int               m_iCCmdNum;


	//! 用于显示console的命令和结果的字串.
	VEC_string        m_vecConsoleDisp;
	//! vector的第几行做为第一个显示在console的中字串.
	int               m_iFirstLine;
	//! 总共要显示多少行的console结果.
	int               m_iTotalDispLines;  
	//! 当前console可以显示多少行的字串.
	int               m_iCapDispLines;

	//! 每一个行的高度是多少个象素.
	int               m_iPerLineHeight;

	//! 用于当前的字串输入.如果输入字符为13,执行这个字串命令.
	s_string          m_strInput;

	//! 输入当前的console显示到文件中去.
	FILE*             m_stream;

	//! 当前的console是不是激活.
	bool              m_bActive;

	//! 当前Console显示的位置.
	RECT              m_dispRect;


	//! 用于在3d设备上显示文字的结构.
	VEC_strDisp       m_devStringDisp;

	//! 需要和3d device交互的变量.比如显示console.
	I_deviceManager*  m_deviceMgr;


	//@{
	//! 用于解释当前行的命令的变量..
	s_string          m_strCmd;
	s_string          m_strVarName;
	s_string          m_strPara;
	ose_CmdType       m_cmdType;
	int               m_iIdxCusCmd;
	//@}

private:
	/** \brief
	 *  Console的处理函数,每一次输入新行都需要调用这个函数.
	 */


	/** \brief
	 *  在显示队列中加入一行显示字串.
	 */
	void              add_dispString( s_string& _dispstr );

	/** \brief
	 *  使用已经提取好的参数设置一个变量.
	 */
	bool              set_cvar( void );

	/** \brief
	 *  使用已经提取好的参数输入到console一个变量的值.
	 */
	bool              get_cvar( void );

	/** \brief
	 *  处理整行输入的函数,输入字串转化为命令后由其它函数进行处理.
	 */
	bool              process_command( s_string& _cmdstr );


	/** \brief
	 *  激活当前的console.
	 */
	void             active_console( bool _act );

	/** \brief
	 *  解释当前行的命令.
	 *
	 *  当前命令可能是已经注册的命令或是得到设置变量的命令.
	 *  把这串命令解释到类的内部变量中,供以后的函数使用.
	 *  如果不是Set和Get命令,在命令列表中也找不到当前的命令,
	 *  函数返回false;
	 */
	bool             parse_command( s_string& _cmd );


public:
	osc_console();
	~osc_console();

	/** \brief
	 *  Console的初始化函数.
	 *
	 *  所有console绘制工作都在当前的console的内部完成.
	 *  所以我们需要可以绘制console的设备.
	 *
	 */
	bool             init_console( I_deviceManager* _dev,int _capdisLines );




	/** \brief
	 *  Console的变量注册函数.
	 */
	void             register_cvar( os_consoleVar& _var );


	/** \brief 
	 *   为引擎中的console注册命令函数.
	 */  
	void             register_cmd( os_consoleCmd& _cmd );


	/** \brief
	 *  Console的输入函数.
	 *
	 *  如果console激活并输入成功,返回真,否则返回false.
	 *  
	 */
	bool            input_char( WORD _c );

	/** \brief
	 *  console的显示函数.
	 *
	 *  先画console的透明的背景,然后再画要在console上显示的字串.
	 */
	bool            draw_console( void );



};


//////////////////////////////////////////////////////////////////////////////////////////
//
// Console 类的Inline 函数.
//
//////////////////////////////////////////////////////////////////////////////////////////

/** \brief
*  激活当前的console.
*/
inline void osc_console::active_console( bool _act )
{
	m_bActive = _act;
}


# endif 