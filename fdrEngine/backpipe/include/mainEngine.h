//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: mainEngine.h
 *
 *  Desc:     ��������������,�ڴ˴�ʵ�ֵõ�����ӿڵĺ���.
 *
 *  His:      River created @ 4/16 2003
 *
 *  �ɹ���һ������Ľ�Ա,����ʹ��������Ϊ���ǲ���ʧ��,������һλ
 *  ������������δ���Ŀɿ�����.
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

//! ��Ϊsound.h�а��������ͷ�ļ����ڴ������ص�stdio.h������һ��Ҫ����
//! ǰ�棬�����Ժ��osInterface.h����Щ�ڴ������صĺ������Ǻ�����


# include "../../mfpipe/include/deviceInit.h"

# include "../../input/InputManager.h" 

# include "../../tools/include/osConsole.h"

# include "../../network/include/fdSocket.h"

# include "../../Terrain/include/fdFieldMgr.h"

# include "../include/osResource.h"





//=====================================================================================
/** \brief 
 *  O.S.O.K �����������.
 *
 *  ����౾�������ⲿ����,��������֯�����������������: ��������Ҫ���⹫����
 *  �ӿ���֯��һ��,��ʵ�ֵõ��ӿڵĺ���.
 *  
 */
//=====================================================================================
class osc_engine
{
private:
	//!  3D Manager's pointer.
	osc_d3dManager*        m_3dDeviceMgr;
	//!  Input Manager's pointer.
	osc_inputmanager*      m_inputMgr;

	//!  Resource Manager's pointer.
	osc_resourceManager*   m_resMgr;

	//!  FD field manager's pointer.
	osc_TGManager*         m_sceneMgr;

	//! �������õ�������ؽӿڡ�
	osc_fdSocket*          m_socketMgr;

private:
	/** \brief
	 *  Ϊ���еĽӿ�ʵ��������ڴ�.
	 */
	void               Initialize( void );


	/** \brief
	 *  �ͷ�engine�з�����ڴ�.
	 */
	void               release( void );

	//! ��ʼ��ȫ�ֵ�Shader�����Ϣ��
	void               initialize_shader( void );

	//! �����Ƿ��ʼ��
	bool               m_bInit;

	static TiXmlDocument*  m_shaderCfgDoc;

public:
	osc_engine();
	~osc_engine();

	/** \brief
	 *  �õ�3d DeviceManager�Ľӿ�.
	 */
	I_deviceManager*   get_3dDeviceMgr( void );

	/** \brief
	 *  �õ�Input Manager�Ľӿ�.
	 */
	I_inputMgr*        get_inputMgr( void );


	/** \brief
	 *  �õ�Resource Manager�Ľӿڼ�.
	 */
	I_resource*        get_resourceMgr( void );

	/** \brief
	 *  �õ������Ľӿڼ���
	 */
	I_fdScene*         get_sceneMgr( void );
	
	/** \brief
	*  �õ�ȫ�ֵ�������ؽӿڡ�
	*/
	I_socketMgr*       get_socketMgr( void );

	//! �ͷ������е���Դ��
	void               release_engine( void ) { this->release() ; } 

	static s_string    get_shaderFile( const char* name );


public:
	//@{
	// ���������Shader��ģ��id��ʹ����Щid���Ը����׵Ĵ���shader.
	
	//! �ر�����shader.
	static int         m_iTileShaId;
	
	//! ��ͨ��Ʒshader.
	static int         m_iObjectShaId;

	//! skinMesh��Ʒ��Shader.
	static int         m_iCharacterShaId;

	//! ��������͡�
	static int         m_iBillBoardShaId;

	//! ������͡�
	static int         m_iSkyShaId;

	//! �����˻�����ͼ��skinMeshShader.
	static int         m_iEnvCharacterShaId;

	//! �����˵ڶ������������skinMeshShader.
	static int         m_iSecCharacterShaId;

	//! River @ 2010-2-25:�������������Է���
	static int         m_iSecCharacterShineId;

	//! River @ 2010-3-11: ��������ĸ������ͼЧ����
	static int         m_iSecCharRotEnvId;
	static int         m_iSecCharShineRotId;
	static int         m_iSecCharShineEnvId;


	//! �ڶ�������İ�͹��ͼskinMeshShader
	static int		   m_iBumpCharacterShaID;
	//@}

};








