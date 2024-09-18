///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_configInfoReader.h
 *
 *  Desc:     用于魔法demo所有配置相关文件的谋取
 * 
 *  His:      River created @ 2006-4-13 
 * 
 */
///////////////////////////////////////////////////////////////////////////////////////////

# pragma once

# include "stdafx.h"

struct lm_heroInsert;

struct lm_footMarkInfo;
class  gc_camera;

/** \brief
 *  用于读入ini配置文件的类，魔法demo内所有的ini都通过这个类内的接口来处理
 *  
 */
class lm_configInfoReader
{
private:
	char          m_szPackFileIni[128];

public:
	lm_configInfoReader(void);
	~lm_configInfoReader(void);
	
	//! 引擎初始化的相关数据
	bool  read_engineInitVar( void );
	
	//! 3d设备初始化的相关数据
	bool  read_d3dDevInitVar( os_deviceinit& _di );

	//! 读入初始化的相机数据
	bool  read_camInitData( os_cameraInit& _camInit );
	//! 读入游戏内相机的配置数据
	bool  read_cameraData( gc_camera* _cam );

	//! 读入地图初始化需要的内容。
	bool  read_mapInitData( os_sceneLoadStruct& _sl );

	//! 读入人物初始化需要的内容
	bool  read_heroInsertData( lm_heroInsert* _hi );

	//! 得到主角人物的相关移速度
	float  read_heroWalkSpeed( void );

	//! 得到主角人物footmark相关的数据
	void   read_heroFootMarkInfo( lm_footMarkInfo* _info );

	//! 得到全局的点击地表效果的配置
	void   read_cursorDrawInScene( os_sceneCursorInit& _cursor );

	//! 得到要初始化的packIni名字
	const char* get_packIniName( void ) { return m_szPackFileIni; } 

};

extern lm_configInfoReader*   g_ptrConfigInfoReader;
