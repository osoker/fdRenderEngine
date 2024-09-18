//=======================================================================================================
/**  \file
 *   Filename: osConsole.h
 *   Desc:     Osok�����е�console��,���console�������ⲿֻ��ע�����,
 *             ���е�console����������Լ�������,ֻ��ʹ��,���ܼ����µ�����.
 *
 *   His:      River created @ 5/23 2003
 *
 *   "һ���Ŷ���,�����ú��ļ���ֻ������һ��������.��Զ��Ӧ����һ���油��Ա."
 *  
 */
//=======================================================================================================
# pragma once

# include "../../interface/osInterface.h"

# if 0

//@{
/** \brief
 *  �õ������ñ���������.
 */
# define SET_VAR_CMD   "set"
# define GET_VAR_CMD   "get"
//@}




//@{
/** \brief
 *  ����console�г��õ�vector.
 */
typedef std::vector<os_consoleVar>  VEC_consoleVar;
typedef std::vector<os_consoleCmd>   VEC_consoleCmd;
typedef std::vector<os_stringDisp>   VEC_strDisp;
//@}

/** \brief
 *  console�е���������.
 */
enum ose_CmdType
{
	CMD_SET,
	CMD_GET,
	CMD_CUSTOM
};



/** \brief
 *  OSOK�����console��,���������osok����console�����еĹ���.
 *
 *  ͨ��ResourceManager���ⲿ�����ӿ�.
 *  Console �Ĺ��ܽ���:
 *  1: �����ڲ�������ͱ���.
 *  2: �ⲿ����ע�����.
 *  3: ��������ͱ����ĸı�.
 *  4: ��������.
 *  5: ��ʾconsole.
 *  
 *  Console�����ִ�Сд,��ʾʱһȫ����ʹ�ô�д(��Сд).
 *
 */
class osc_console
{
private:
	//! console variable��vector.
	VEC_consoleVar    m_vecConsoleVar;
	int               m_iCVarNum;

	//! console command ��vector.
	VEC_consoleCmd    m_vecConsoleCmd;
	int               m_iCCmdNum;


	//! ������ʾconsole������ͽ�����ִ�.
	VEC_string        m_vecConsoleDisp;
	//! vector�ĵڼ�����Ϊ��һ����ʾ��console�����ִ�.
	int               m_iFirstLine;
	//! �ܹ�Ҫ��ʾ�����е�console���.
	int               m_iTotalDispLines;  
	//! ��ǰconsole������ʾ�����е��ִ�.
	int               m_iCapDispLines;

	//! ÿһ���еĸ߶��Ƕ��ٸ�����.
	int               m_iPerLineHeight;

	//! ���ڵ�ǰ���ִ�����.��������ַ�Ϊ13,ִ������ִ�����.
	s_string          m_strInput;

	//! ���뵱ǰ��console��ʾ���ļ���ȥ.
	FILE*             m_stream;

	//! ��ǰ��console�ǲ��Ǽ���.
	bool              m_bActive;

	//! ��ǰConsole��ʾ��λ��.
	RECT              m_dispRect;


	//! ������3d�豸����ʾ���ֵĽṹ.
	VEC_strDisp       m_devStringDisp;

	//! ��Ҫ��3d device�����ı���.������ʾconsole.
	I_deviceManager*  m_deviceMgr;


	//@{
	//! ���ڽ��͵�ǰ�е�����ı���..
	s_string          m_strCmd;
	s_string          m_strVarName;
	s_string          m_strPara;
	ose_CmdType       m_cmdType;
	int               m_iIdxCusCmd;
	//@}

private:
	/** \brief
	 *  Console�Ĵ�����,ÿһ���������ж���Ҫ�����������.
	 */


	/** \brief
	 *  ����ʾ�����м���һ����ʾ�ִ�.
	 */
	void              add_dispString( s_string& _dispstr );

	/** \brief
	 *  ʹ���Ѿ���ȡ�õĲ�������һ������.
	 */
	bool              set_cvar( void );

	/** \brief
	 *  ʹ���Ѿ���ȡ�õĲ������뵽consoleһ��������ֵ.
	 */
	bool              get_cvar( void );

	/** \brief
	 *  ������������ĺ���,�����ִ�ת��Ϊ������������������д���.
	 */
	bool              process_command( s_string& _cmdstr );


	/** \brief
	 *  ���ǰ��console.
	 */
	void             active_console( bool _act );

	/** \brief
	 *  ���͵�ǰ�е�����.
	 *
	 *  ��ǰ����������Ѿ�ע���������ǵõ����ñ���������.
	 *  ���⴮������͵�����ڲ�������,���Ժ�ĺ���ʹ��.
	 *  �������Set��Get����,�������б���Ҳ�Ҳ�����ǰ������,
	 *  ��������false;
	 */
	bool             parse_command( s_string& _cmd );


public:
	osc_console();
	~osc_console();

	/** \brief
	 *  Console�ĳ�ʼ������.
	 *
	 *  ����console���ƹ������ڵ�ǰ��console���ڲ����.
	 *  ����������Ҫ���Ի���console���豸.
	 *
	 */
	bool             init_console( I_deviceManager* _dev,int _capdisLines );




	/** \brief
	 *  Console�ı���ע�ắ��.
	 */
	void             register_cvar( os_consoleVar& _var );


	/** \brief 
	 *   Ϊ�����е�consoleע�������.
	 */  
	void             register_cmd( os_consoleCmd& _cmd );


	/** \brief
	 *  Console�����뺯��.
	 *
	 *  ���console�������ɹ�,������,���򷵻�false.
	 *  
	 */
	bool            input_char( WORD _c );

	/** \brief
	 *  console����ʾ����.
	 *
	 *  �Ȼ�console��͸���ı���,Ȼ���ٻ�Ҫ��console����ʾ���ִ�.
	 */
	bool            draw_console( void );



};


//////////////////////////////////////////////////////////////////////////////////////////
//
// Console ���Inline ����.
//
//////////////////////////////////////////////////////////////////////////////////////////

/** \brief
*  ���ǰ��console.
*/
inline void osc_console::active_console( bool _act )
{
	m_bActive = _act;
}


# endif 