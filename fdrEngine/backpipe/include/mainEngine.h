//--------------------------------------------------------------------------------------------------------
/** \file
 *  Filename: mainEngine.h
 *
 *  Desc:     处理引擎主体框架,在此处实现得到引擎接口的函数.
 *
 *  His:      River created @ 4/16 2003
 *
 *  成功是一个讨厌的教员,它诱使聪明人认为他们不会失败,它不是一位
 *  引导我们走向未来的可靠的向导.
 *
 */
//--------------------------------------------------------------------------------------------------------
# pragma once

//! 因为sound.h中包含的相关头文件有内存分配相关的stdio.h，所以一定要放在
//! 前面，以让以后的osInterface.h把这些内存管理相关的函数或是宏重载


# include "../../mfpipe/include/deviceInit.h"

# include "../../input/InputManager.h" 

# include "../../tools/include/osConsole.h"

# include "../../network/include/fdSocket.h"

# include "../../Terrain/include/fdFieldMgr.h"

# include "../include/osResource.h"





//=====================================================================================
/** \brief 
 *  O.S.O.K 引擎的主体框架.
 *
 *  这个类本身并不向外部公开,她起到了组织引擎中主体类的作用: 她把所有要往外公开的
 *  接口组织到一起,并实现得到接口的函数.
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

	//! 引擎中用的网络相关接口。
	osc_fdSocket*          m_socketMgr;

private:
	/** \brief
	 *  为所有的接口实现类分配内存.
	 */
	void               Initialize( void );


	/** \brief
	 *  释放engine中分配的内存.
	 */
	void               release( void );

	//! 初始化全局的Shader相关信息。
	void               initialize_shader( void );

	//! 引擎是否初始化
	bool               m_bInit;

	static TiXmlDocument*  m_shaderCfgDoc;

public:
	osc_engine();
	~osc_engine();

	/** \brief
	 *  得到3d DeviceManager的接口.
	 */
	I_deviceManager*   get_3dDeviceMgr( void );

	/** \brief
	 *  得到Input Manager的接口.
	 */
	I_inputMgr*        get_inputMgr( void );


	/** \brief
	 *  得到Resource Manager的接口集.
	 */
	I_resource*        get_resourceMgr( void );

	/** \brief
	 *  得到场景的接口集。
	 */
	I_fdScene*         get_sceneMgr( void );
	
	/** \brief
	*  得到全局的网络相关接口。
	*/
	I_socketMgr*       get_socketMgr( void );

	//! 释放引擎中的资源。
	void               release_engine( void ) { this->release() ; } 

	static s_string    get_shaderFile( const char* name );


public:
	//@{
	// 引擎中最常用Shader的模板id，使用这些id可以更容易的创建shader.
	
	//! 地表纹理shader.
	static int         m_iTileShaId;
	
	//! 普通物品shader.
	static int         m_iObjectShaId;

	//! skinMesh物品的Shader.
	static int         m_iCharacterShaId;

	//! 公告板类型。
	static int         m_iBillBoardShaId;

	//! 天空类型。
	static int         m_iSkyShaId;

	//! 加入了环境贴图的skinMeshShader.
	static int         m_iEnvCharacterShaId;

	//! 加入了第二层流动纹理的skinMeshShader.
	static int         m_iSecCharacterShaId;

	//! River @ 2010-2-25:加入人物纹理自发光
	static int         m_iSecCharacterShineId;

	//! River @ 2010-3-11: 加入人物的更多层贴图效果。
	static int         m_iSecCharRotEnvId;
	static int         m_iSecCharShineRotId;
	static int         m_iSecCharShineEnvId;


	//! 第二层纹理的凹凸贴图skinMeshShader
	static int		   m_iBumpCharacterShaID;
	//@}

};








