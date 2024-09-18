//===============================================================================
/*! \file
*	Filename:	fdGalaxy.cpp	
*	Desc:		galaxy shader effect define file
*	His:		tzz	2008-6-4
*/
//===============================================================================

#include "../include/fdGalaxy.h"

extern osc_shaderMgr*	g_shaderMgr;


//====================================================================================================================
//
//	CExplode class
//
//====================================================================================================================
//--------------------------------------------------------------------------------------------------------------------
//Name: constructor and destructor
//Desc: ...
//--------------------------------------------------------------------------------------------------------------------
CExplode::CExplode(void):m_fCurrEleTime(0.0f),
						m_ptrMiddPipe(NULL)
					
{
	strcpy(m_szShaderName,"galaxy_explode");
	strcpy(m_szIniFile,"ini\\galaxy_explode.ini");
}
CExplode::~CExplode(void)
{

}

//--------------------------------------------------------------------------------------------------------------------
//Name: CreateExplode
//Desc: create the explode shader
//--------------------------------------------------------------------------------------------------------------------
void CExplode::CreateExplode(osc_middlePipe* _ptrMidPipe)
{
	guard;

	osassert(g_shaderMgr && _ptrMidPipe);

	//assign the middle pipe
	m_ptrMiddPipe = _ptrMidPipe;

	//create the effect id
	m_terraRect.m_iShaderId = g_shaderMgr->add_shadertomanager(m_szShaderName,m_szIniFile);	
	osassert(m_terraRect.m_iShaderId >=0);

	unguard;
}