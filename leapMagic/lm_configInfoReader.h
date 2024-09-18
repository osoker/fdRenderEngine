///////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: lm_configInfoReader.h
 *
 *  Desc:     ����ħ��demo������������ļ���ıȡ
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
 *  ���ڶ���ini�����ļ����࣬ħ��demo�����е�ini��ͨ��������ڵĽӿ�������
 *  
 */
class lm_configInfoReader
{
private:
	char          m_szPackFileIni[128];

public:
	lm_configInfoReader(void);
	~lm_configInfoReader(void);
	
	//! �����ʼ�����������
	bool  read_engineInitVar( void );
	
	//! 3d�豸��ʼ�����������
	bool  read_d3dDevInitVar( os_deviceinit& _di );

	//! �����ʼ�����������
	bool  read_camInitData( os_cameraInit& _camInit );
	//! ������Ϸ���������������
	bool  read_cameraData( gc_camera* _cam );

	//! �����ͼ��ʼ����Ҫ�����ݡ�
	bool  read_mapInitData( os_sceneLoadStruct& _sl );

	//! ���������ʼ����Ҫ������
	bool  read_heroInsertData( lm_heroInsert* _hi );

	//! �õ����������������ٶ�
	float  read_heroWalkSpeed( void );

	//! �õ���������footmark��ص�����
	void   read_heroFootMarkInfo( lm_footMarkInfo* _info );

	//! �õ�ȫ�ֵĵ���ر�Ч��������
	void   read_cursorDrawInScene( os_sceneCursorInit& _cursor );

	//! �õ�Ҫ��ʼ����packIni����
	const char* get_packIniName( void ) { return m_szPackFileIni; } 

};

extern lm_configInfoReader*   g_ptrConfigInfoReader;
