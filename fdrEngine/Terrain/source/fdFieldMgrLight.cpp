//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgrLight.cpp
 *
 *  His:      River created @ 2004-5-9
 *
 *  Des:      场景中的光照和动态光相关的函数处理。
 *   
 *  "圣人不治已病，治未病，不能已乱，治未乱"
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# include "stdafx.h"
# include "../include/fdFieldMgr.h"
# include "../../backPipe/include/osCamera.h"
# include  "../../mfpipe/include/osMesh.h"
# include  "../../mfpipe/include/deviceInit.h"
# include  "../../mfpipe/include/osCharacter.h"


//! 对场景中的顶点颜色数据使用环境光系数进行处理。
void osc_TGManager::reset_ambientCoeff( void )
{
	guard;


	for( int t_i=0;t_i<this->m_iActMapNum;t_i++ )
	{
		osassert( this->m_arrPtrTG[t_i] );



		m_arrPtrTG[t_i]->set_ambientLCoeff( g_fAmbiR,
			g_fAmbiG,g_fAmbiB );


	}



	return;

	unguard;
}

void osc_TGManager::start_sceneLight(float _times,float frequency,int flashtype,float flashTime,osColor _color /*= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f)*/)
{
	m_SceneFlashLightMgr->start_sceneLight(_times,frequency,flashtype,flashTime,_color);

}
void osc_TGManager::stop_sceneLight()
{
	m_SceneFlashLightMgr->stop();
}
/** \brief
*  改变场景的环境光系数。
*
*  \param _r,_g,_b  场景中环境光的系数，使用这三个值缩放场景中的环境光。
*
*  ATTENTION :  加入对物品对应的环境光的处理。全局只有这一个地方可以修改全局光。
*/
void osc_TGManager::change_amibentCoeff( float _r,float _g,float _b )
{
	osassert( (_r>=0)&(_b>=0)&&(_g>=0) );


	g_fAmbiR = _r;
	g_fAmbiG = _g;
	g_fAmbiB = _b;


	// 先恢复被动态光处理过的格子的颜色。
	//
	// 对每一个tg进行帧间的初始化。
	for( int t_i=0;t_i<m_iActMapNum;t_i++ )
	{
		osassert( m_arrPtrTG[t_i] );
		
		m_arrPtrTG[t_i]->frame_setTG();			

	}

	reset_ambientCoeff();

	// 物品的环境光相关。

	// skinMesh的环境光相关。
	m_skinMeshMgr->reset_ambient();
}


//! 处理animation Mesh的动态光数据。
bool osc_TGManager::process_aniMeshDlight( void )
{
	guard;

	osc_dlight*    t_ptrDL;
	os_dLightData   t_dlight;

	t_dlight.m_sAmbientLight.a = 1.0f;
	t_dlight.m_sAmbientLight.r = g_fAmbiR;
	t_dlight.m_sAmbientLight.g = g_fAmbiG;
	t_dlight.m_sAmbientLight.b = g_fAmbiB;

	// 
	// 对每一个动态，处理当前动态光对ambientMesh的影响。
	for( int t_i=0;t_i<MAX_LIGHTNUM;t_i++ )
	{
		t_ptrDL = &m_dlightMgr.m_vecDlight[t_i];
		if( !t_ptrDL->m_bInuse )
			continue;

		t_dlight.m_fAttenuationDis = t_ptrDL->m_fRadius;
		t_dlight.m_sDLight = t_ptrDL->m_dlightColor;
		t_dlight.m_vec3DLightPos = t_ptrDL->m_vec3Pos;
		
		m_middlePipe.process_amDlight( t_dlight );

	}

	return true;

	unguard;
}


/** \brief
*  处理动态光。
*/
BOOL osc_newTileGround::process_dLight( osc_dlight* _dl ,int _lightID)
{
	return true;
	//return m_sMapTerrain.SetDynamic_Light(_dl,_lightID);
}

/** \brief
*  使用给出的灯光系数对场景中每一个顶点的灯光进行缩放。
*/
void osc_newTileGround::set_ambientLCoeff( float _r,float _g,float _b )
{
	guard;
//	m_sMapTerrain.SetambientLCoeff(_r,_g,_b);
	unguard;
}