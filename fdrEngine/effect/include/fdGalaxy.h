//===============================================================================
/*! \file
*	Filename:	fdGalaxy.h	
*	Desc:		add some shader effect to the galaxy
*	His:		tzz	2008-6-4
*/
//===============================================================================

#pragma once


# include "../../interface/osInterface.h"

# include "../../mfpipe/include/osShader.h"               
# include "../../mfpipe/include/middlePipe.h"


/*! \class osc_explode
*	explode shader effection
*/
class CExplode
{
private: //constant variable

	

private:
	/*! \struct
	*	inner struct to define vertex of terra 
	*	surface rectangle
	*/
	struct{
		osVec3D		m_vecPos;
		DWORD		m_dwColor;
		osVec2D		m_vecUV;
	};
	/*
private: //our private data

	//! id of shader effect
	//int					m_iEffectId;

	//! elapse time from start the app
	float				m_fCurrEleTime;

	//! current terra surface polygon(rectangle)
	os_polygon			m_terraRect;

	//! middle pipe for shader of osok engine
	osc_middlePipe*		m_ptrMiddPipe;

	//! explode name to hashed (interpreting for size in osc_shaderMgr::create_shaderFromINI(...)
	char				m_szShaderName[64]; 

	//! explode .ini filename(interpreting for size in osc_shaderMgr::create_shaderFromINI(...)
	char				m_szIniFile[64];
	*/

public: //constructor and destructor
	/*! \fn constructor and destructor
	*	...
	*/
	CExplode(void);
	~CExplode(void);

public: //some public function

	/*! \fn void CreateExplode(osc_middlePipe* _ptrMidPipe)
	*	create the explode shader
	*	
	*	\param	_ptrMidPipe	: middle pipe class (interpreting in middlePipe.h\cpp)
	*	\return void
	*/
	void CreateExplode(osc_middlePipe* _ptrMidPipe);


};
