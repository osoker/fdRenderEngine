//=======================================================================================================
/*!  \file
*   Filename: light.cpp
*   Desc:     the light make the real world
*
*   His:      tzz created @ 2009-9-4
*
*  
*/
//=======================================================================================================
#include "stdafx.h"
#include "light.h"

#include <algorithm>

extern bool gUseTerrainPS;

static D3DXCOLOR GetDayTimeLerpColor(const D3DXCOLOR& _dayColor,
									 const D3DXCOLOR& _nightColor,
									 size_t _dayTime)
{
	D3DXCOLOR t_reColor;

	// interpolating the day color and the night color
	//
	if(_dayTime < 12 * 60){
		t_reColor = _nightColor + ( (float)_dayTime / (12.0f*60.0f) ) * (_dayColor - _nightColor);
	}else{
		t_reColor = _dayColor + ( (float)_dayTime / (12.0f*60.0f)  - 1.0f) * (_nightColor - _dayColor);
	}

	return t_reColor;
}

CPointLightObj::CPointLightObj(void):m_effectRange(0.0f),
									m_renderData(NULL),
									m_lightPos(0.0f,0.0f,0.0f),
									m_dayColor(1,1,1,1),
									m_nightColor(1,1,1,1)
{
	ZeroMemory(&m_lightAtten,sizeof(m_lightAtten));
	m_lightAtten.m_attenuation0 = 0.5f;
}
CPointLightObj::CPointLightObj(const D3DXVECTOR3 _pos):m_effectRange(0.0f),
														m_lightPos(_pos),
														m_renderData(NULL),
														m_dayColor(0,0,0,0),
														m_nightColor(0,0,0,0)
{
	ZeroMemory(&m_lightAtten,sizeof(m_lightAtten));
}

CPointLightObj::CPointLightObj(const CPointLightObj& _obj)
{
	m_lightPos		= _obj.m_lightPos;
	m_effectRange	= _obj.m_effectRange;
	m_dayColor		= _obj.m_dayColor;
	m_nightColor	= _obj.m_nightColor;
	m_lightAtten	= _obj.m_lightAtten;
	
	m_renderData	= _obj.m_renderData;
}

CPointLightObj::~CPointLightObj(void)
{
	
}

void CPointLightObj::ReadFromFile(CReadFile& _file,DWORD _version)
{
	guard;

	osassert(_file.IsOpen());

	_file.ReadBuffer(&m_lightPos,sizeof(m_lightPos));
	_file.ReadBuffer(&m_effectRange,sizeof(m_effectRange));
	_file.ReadBuffer(&m_dayColor,sizeof(m_dayColor));
	_file.ReadBuffer(&m_nightColor,sizeof(m_nightColor));
	_file.ReadBuffer(&m_lightAtten,sizeof(m_lightAtten));

	unguard;
}


void CPointLightObj::WriteToFile(FILE* _file)
{
	guard;

	osassert(_file);

	fwrite(&m_lightPos,1,sizeof(m_lightPos),_file);
	fwrite(&m_effectRange,1,sizeof(m_effectRange),_file);
	fwrite(&m_dayColor,1,sizeof(m_dayColor),_file);
	fwrite(&m_nightColor,1,sizeof(m_nightColor),_file);
	fwrite(&m_lightAtten,1,sizeof(m_lightAtten),_file);

	unguard;
}

void CPointLightObj::SetLight(LPDIRECT3DDEVICE9 _device,DWORD _index,size_t _dayTime)const
{
	guard;

	osassert(_device);
	osassert(_index >= 0 && _index < 8);
	osassert(_dayTime >= 0 && _dayTime <= 24 * 60);

	D3DLIGHT9 t_d3dLight;
	ZeroMemory(&t_d3dLight,sizeof(t_d3dLight));
	
	t_d3dLight.Type = D3DLIGHT_POINT;

	t_d3dLight.Position = m_lightPos;
	t_d3dLight.Range = m_effectRange;
	t_d3dLight.Attenuation0 = m_lightAtten.m_attenuation0;
	t_d3dLight.Attenuation1 = m_lightAtten.m_attenuation1;
	t_d3dLight.Attenuation2 = m_lightAtten.m_attenuation2;

	t_d3dLight.Diffuse = GetDayTimeLerpColor(m_dayColor,m_nightColor,_dayTime);
	t_d3dLight.Specular = t_d3dLight.Diffuse;
	
	VALIDATE_D3D(_device->SetLight(_index,&t_d3dLight));
	
	unguard;
}


// the light manage connect the main engine and the light data...
//
CLightManger* g_ptrLightMgr = NULL;
CLightManger::CLightManger(void):m_bEnableLight(FALSE),
								m_enableLightNum(scm_maxEnableLightNum),
								m_refD3Device(NULL),
								m_currDayTime(0),
								m_currDayAmbientColor(1.0f,1.0f,1.0f,1.0f),
								m_currNightAmbientColor(0.2f,0.2f,0.2f,1.0f),
								m_currDayDirColor(1.0f,1.0f,1.0f,1.0f),
								m_currNightDirColor(0.2f,0.2f,0.2f,1.0f),
								m_constLightPtr(NULL)
{
	g_ptrLightMgr = this;
	ZeroMemory(&m_directLight,sizeof(m_directLight));

	m_directLight.Type		= D3DLIGHT_DIRECTIONAL;
	m_directLight.Range		= 10000.0f;
	m_directLight.Specular	= D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);

}
CLightManger::~CLightManger(void)
{
	ClearAllLight(); 
	g_ptrLightMgr = NULL;
}

//! clear all light;
void CLightManger::ClearAllLight(void)
{
	guard;
	
	std::for_each(m_lightContainer.begin(),m_lightContainer.end(),DelPointLightFunc());
	m_lightContainer.clear();

	m_constLightPtr = NULL;
	m_currDayAmbientColor = m_currNightAmbientColor = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f);

#ifdef SCENE_RENDER

	for(size_t i = 0;i < m_vectDrawCircle.size();i++){
		SAFE_RELEASE(m_vectDrawCircle[i].m_lpDrawCircleBuf_horz);
		SAFE_RELEASE(m_vectDrawCircle[i].m_lpDrawCircleBuf_vert);
	}
	m_vectDrawCircle.clear();

#endif //SCENE_RENDER
	
	unguard;
}

//! initialize the light
void CLightManger::Initlize(LPDIRECT3DDEVICE9 _dev)
{
	guard;

	osassert(_dev);

	m_refD3Device = _dev;

	unguard;
}

void CLightManger::SetCurrDayAmbientColor(const D3DXCOLOR& _color)
{
	m_currDayAmbientColor = _color;
}

void CLightManger::SetCurrNightAmbientColor(const D3DXCOLOR& _color)
{
	m_currNightAmbientColor = _color;
}

//! EnableLight
void CLightManger::EnalbeLight(BOOL _enable)
{
	guard;

	if(m_bEnableLight != _enable){
		m_bEnableLight = _enable;

		m_refD3Device->SetRenderState( D3DRS_LIGHTING, _enable );

		if(!_enable){
			VALIDATE_D3D(m_refD3Device->SetRenderState(D3DRS_AMBIENT,0xffffffff));
		}
	}	

	unguard;
}

//! can effect the light
BOOL CLightManger::CanEffectLight(const CPointLightObj* _light,const D3DXVECTOR3& _focus,float _range)
{
	guard;

	D3DXVECTOR3 t_dir = _focus - _light->GetLightPos();
	const float t_distance = _light->GetEffectRange() + _range;

	return D3DXVec3LengthSq(&t_dir) < t_distance * t_distance ;
			
	unguard;
}
BOOL CLightManger::CanEffectLight(const CPointLightObj* _light,const D3DXVECTOR4& _retangle)
{
	guard;

	const D3DXVECTOR2 t_retangleCenter(_retangle.x + (_retangle.z - _retangle.x) / 2.0f,
										_retangle.y + (_retangle.w - _retangle.y) / 2.0f);

	D3DXVECTOR2 t_dir(t_retangleCenter.x - _light->GetLightPos().x,t_retangleCenter.y - _light->GetLightPos().z);

	return D3DXVec2LengthSq(&t_dir) < (_retangle.z - _retangle.x)*(_retangle.z - _retangle.x);

	unguard;
}
bool CLightManger::EffectSort_less::operator ()(const CPointLightObj* _left,const CPointLightObj* _right)
{
	const D3DXVECTOR3 t_dir1 = _left->GetLightPos() - m_focus;
	const D3DXVECTOR3 t_dir2 = _right->GetLightPos() - m_focus;
	return D3DXVec3LengthSq(&t_dir1) - (_left->GetEffectRange() * _left->GetEffectRange()) 
				< D3DXVec3LengthSq(&t_dir2) - (_right->GetEffectRange() * _right->GetEffectRange()) ;
}
//! focus center render
D3DXCOLOR CLightManger::FocusCenterRender(D3DXVECTOR3 _focus,float _radius,D3DXVECTOR3& _dirLight,D3DXCOLOR& _dirColor)
{
	guard;

	D3DXCOLOR t_ambientColor(1.0f,1.0f,1.0f,1.0f);

	if(m_bEnableLight){

		size_t t_enableNum = 0;

		osassert(m_refD3Device);
		
		VALIDATE_D3D(m_refD3Device->SetRenderState( D3DRS_LIGHTING, TRUE));

		t_ambientColor = GetDayTimeLerpColor(m_currDayAmbientColor,m_currNightAmbientColor,m_currDayTime);		
		VALIDATE_D3D(m_refD3Device->SetRenderState(D3DRS_AMBIENT,t_ambientColor));

		// set the const light for the local hero' view
		//
		if(m_constLightPtr && t_enableNum < m_enableLightNum){
		
			if(CanEffectLight(m_constLightPtr,_focus,_radius)){
				m_constLightPtr->SetLight(m_refD3Device,t_enableNum,m_currDayTime);

				VALIDATE_D3D(m_refD3Device->LightEnable(t_enableNum,TRUE));

				t_enableNum++;
			}
		}

		// set the direction light
		//
		D3DXVECTOR3 t_sunDir,t_moonDir;
		GetSunMoonDir(&t_sunDir,&t_moonDir);
		if(t_sunDir.y < 0.0f){
			// simulate the sun light
			//
			_dirLight = t_sunDir;
			_dirColor = GetDayTimeLerpColor(m_currDayDirColor,m_currNightDirColor,m_currDayTime);
		}else{
			// simulate the moon light
			//
			_dirLight = t_moonDir;

			const D3DXCOLOR t_moonDayColor(m_currNightDirColor.r * 0.2f,m_currNightDirColor.g * 0.2f,m_currNightDirColor.b * 0.2f,1.0f);
			const D3DXCOLOR t_moonNightColor(m_currDayDirColor.r * 0.2f,m_currDayDirColor.g * 0.2f,m_currDayDirColor.b * 0.2f,1.0f);

			_dirColor = GetDayTimeLerpColor(t_moonDayColor,t_moonNightColor,m_currDayTime);
		}

		m_directLight.Direction.x = _dirLight.x;
		m_directLight.Direction.y = _dirLight.y;
		m_directLight.Direction.z = _dirLight.z;

		m_directLight.Diffuse = _dirColor;

		VALIDATE_D3D(m_refD3Device->SetLight(t_enableNum,&m_directLight));
		VALIDATE_D3D(m_refD3Device->LightEnable(t_enableNum,TRUE));

		t_enableNum++;
		

		// prepare the effect light data....
		//
		m_vectRenderPointSort.erase(m_vectRenderPointSort.begin(),m_vectRenderPointSort.end());
				
		for(LightContainer::iterator it = m_lightContainer.begin();
			it != m_lightContainer.end() && t_enableNum < m_enableLightNum;it++){

			const CPointLightObj* t_pointLight = *it;
			
			if(CanEffectLight(t_pointLight,_focus,_radius)){
				
				m_vectRenderPointSort.push_back(t_pointLight);
				
			}
		}

		// sort and effect by distance...
		//
		std::sort(m_vectRenderPointSort.begin(),m_vectRenderPointSort.end(),EffectSort_less(_focus));
		for(size_t i = 0; i< m_vectRenderPointSort.size() && t_enableNum < scm_maxEnableLightNum;i++){

			const CPointLightObj* t_pointLight = m_vectRenderPointSort[i];
			// effected
			//
			t_pointLight->SetLight(m_refD3Device,t_enableNum,m_currDayTime);

			// open the light
			//
			VALIDATE_D3D(m_refD3Device->LightEnable(t_enableNum,TRUE));

			t_enableNum++;
		}

		//close the another light
		for(size_t i = t_enableNum;i < m_enableLightNum;i++){
			VALIDATE_D3D(m_refD3Device->LightEnable(i,FALSE));
		}

	}else{
		VALIDATE_D3D(m_refD3Device->SetRenderState(D3DRS_AMBIENT,D3DCOLOR_RGBA(255,255,255,255)));
	}


	return t_ambientColor;

	unguard;
}
//! search point light by the position
CPointLightObj* CLightManger::SearchLight(const D3DXVECTOR3& _pos)
{
	guard;

	LightContainer::iterator t_it = 
		std::find_if(m_lightContainer.begin(),m_lightContainer.end(),FindPointLightByPos(_pos));

	if(t_it != m_lightContainer.end()){
		return (*t_it);
	}

	return NULL;

	unguard;
}
CPointLightObj* CLightManger::AddPointLight(void)
{
	guard;

	CPointLightObj* t_newPoint = new CPointLightObj();

	m_lightContainer.push_back(t_newPoint);

#ifdef SCENE_RENDER
	CreateCircleVertexBuf(t_newPoint);
#endif //SCENE_RENDER

	return t_newPoint;

	unguard;
}
//! add a point light
CPointLightObj* CLightManger::AddPointLight(const CPointLightObj& _obj)
{
	guard;
	
	CPointLightObj* t_newPoint = new CPointLightObj(_obj);

	m_lightContainer.push_back(t_newPoint);

#ifdef SCENE_RENDER
	CreateCircleVertexBuf(t_newPoint);
#endif //SCENE_RENDER

	return t_newPoint;	

	unguard;
}
CPointLightObj* CLightManger::AddPointLight(const D3DXVECTOR3& _pos)
{
	guard;
	
	CPointLightObj* t_point = SearchLight(_pos);
	if(!t_point){
		t_point = AddPointLight();
		t_point->SetLightPos(_pos);	
	}	
	return t_point;

	unguard;
}
CPointLightObj* CLightManger::SearchLight(void* _renderData)
{
	guard;

	using namespace std;

	LightContainer::iterator t_it = find_if(m_lightContainer.begin(),m_lightContainer.end(),
		FindPointLightByRenderData(_renderData));

	if(t_it != m_lightContainer.end()){
		return (*t_it);
	}

	return NULL;

	unguard;
}
//! delete point light
void CLightManger::DelPointLight(void* _renderData)
{
	guard;

	using namespace std;

	LightContainer::iterator t_it = find_if(m_lightContainer.begin(),m_lightContainer.end(),
													FindPointLightByRenderData(_renderData));

	if(t_it != m_lightContainer.end()){
		CPointLightObj* t_deleteMe = *t_it
			;
#ifdef SCENE_RENDER
		DeleteCircleVertexBuf(t_deleteMe);
#endif //SCENE_RENDER
		
		delete t_deleteMe;
		m_lightContainer.erase(t_it);
	}

	unguard;
}
void CLightManger::SetConstLightAttr(const D3DXCOLOR& _color,const D3DXVECTOR3& _pos,float _range)
{
	guard;

	if(!m_constLightPtr){
		m_constLightPtr = new CPointLightObj();
		m_lightContainer.push_front(m_constLightPtr);
		m_constLightPtr->SetAttenuation(1.0f,0.2f,0.0f);
	}

	m_constLightPtr->SetLightPos(_pos);
	m_constLightPtr->SetDayColor(_color);
	m_constLightPtr->SetNightColor(_color);
	m_constLightPtr->SetEffectRange(_range);


	unguard;
}
void CLightManger::EffectObject::operator()(const CPointLightObj* _point)
{
	guard;

	osassert(m_fillData);

	bool t_effect = false;

	if(m_bEffectChunk){
		t_effect = /*CLightManger::CanEffectLight(_point,m_focus,m_radius) && */ CLightManger::CanEffectLight(_point,m_retangle);
	}else{
		t_effect = CLightManger::CanEffectLight(_point,m_focus,m_radius)			// the focus effected
				&& CLightManger::CanEffectLight(_point,m_skinMeshPos,0);	// the skinMeshPos effect	
	}

	if(t_effect){
		m_vectEffectPoint.push_back(_point);
	}
	
	unguard;
}

//! sort the light and effect the effect
int CLightManger::SortLightAndEffect(CLightManger::PointLightEffData t_data[],const size_t _dataLen,const D3DXVECTOR3& _focus)
{
	guard;

	int t_effectNum = 0;

	std::sort(m_vectRenderPointSort.begin(),m_vectRenderPointSort.end(),EffectSort_less(_focus));

	for(size_t i = 0;i < _dataLen && i < m_vectRenderPointSort.size();i++){
		const CPointLightObj* t_pointLight = m_vectRenderPointSort[i];

		t_data[i].m_attenuation = t_pointLight->GetLightAtten();
		t_data[i].m_lightColor	= GetDayTimeLerpColor(t_pointLight->GetDayColor(),
														t_pointLight->GetNightColor(),
														m_currDayTime);
		t_data[i].m_lightPos	= t_pointLight->GetLightPos();

		t_data[i].m_effectRange = t_pointLight->GetEffectRange();

		++ t_effectNum;
	}

	return t_effectNum;

	unguard;
}
//! effect the skin mesh of main engine
int CLightManger::EffectSkinMesh(const D3DXVECTOR3& _focus,float _radius,const D3DXVECTOR3& _skinMeshPos,LPD3DXEFFECT _eff)
{
	guard;

	int t_effectNum = 0;
	PointLightEffData t_data[scm_maxEffectSkinMeshNum];
	ZeroMemory(t_data,sizeof(t_data));

	if(IsEnableLight()){
		osassert(m_refD3Device);
		VALIDATE_D3D(m_refD3Device->SetRenderState( D3DRS_LIGHTING, TRUE));

		std::for_each(m_lightContainer.begin(),m_lightContainer.end(),
				EffectObject(t_data,_focus,_radius,false,m_vectRenderPointSort,_skinMeshPos));
		
		t_effectNum = SortLightAndEffect(t_data,scm_maxEffectSkinMeshNum,_focus);
	}

	osassert(_eff);
	VALIDATE_D3D(_eff->SetValue("pointLight",(void*)&t_data,sizeof(t_data)));

	return t_effectNum;
	
	unguard;
}
//! effect the chunk
int CLightManger::EffectChunk(const D3DXVECTOR3& _focus,float _radius,const D3DXVECTOR4& _chunkRetangle,LPD3DXEFFECT _eff)
{
	guard;

	int t_effectNum = 0;

	PointLightEffData t_data[scm_maxEffectGeoChunckNum];
	ZeroMemory(t_data,sizeof(t_data));

	if(IsEnableLight()){
		osassert(m_refD3Device);
		VALIDATE_D3D(m_refD3Device->SetRenderState( D3DRS_LIGHTING, TRUE));

		std::for_each(m_lightContainer.begin(),m_lightContainer.end(),
				EffectObject(t_data,_focus,_radius,true,m_vectRenderPointSort,_focus,_chunkRetangle));

		t_effectNum = SortLightAndEffect(t_data,scm_maxEffectGeoChunckNum,_focus);
	}

	osassert(_eff);

	if( gUseTerrainPS )
		VALIDATE_D3D(_eff->SetValue("pointLight",(void*)&t_data,sizeof(t_data)));

	return t_effectNum;

	unguard;
}

//! unEffect the skinMesh
void CLightManger::UnEffectSkinMesh(LPD3DXEFFECT _eff)
{
	guard;
	
	if(IsEnableLight()){
		PointLightEffData t_data[scm_maxEffectSkinMeshNum];
		ZeroMemory(t_data,sizeof(t_data));

		osassert(_eff);
		VALIDATE_D3D(_eff->SetValue("pointLight",(void*)&t_data,sizeof(t_data)));
	}

	unguard;
}


//! get the sun/moon position
void CLightManger::GetSunMoonDir(D3DXVECTOR3* _sunDir,D3DXVECTOR3* _moonDir)const 
{
	guard;

	if(_sunDir || _moonDir){
		const float pi = 3.1415926f;
		const float piDiv = 3.1415926f / 2.0f;
		const float h = 1.0f;

		const float t_rate = 0.6f;

		const float t_quarterTime = (24.0f * 60.0f) / 4.0f;
		const float t_currTime = (float)GetCurrentDayTime();

		float t_sunAngle;
		if(t_currTime < t_quarterTime){
			t_sunAngle = (1.0f - (t_currTime / t_quarterTime) * t_rate) * (-piDiv);
		}else if(t_currTime >= t_quarterTime && t_currTime < t_quarterTime * 2.0f){
			t_sunAngle = (piDiv * (1.0f - t_rate)) + ((t_currTime  - t_quarterTime) / t_quarterTime * t_rate) * piDiv;
		}else if(t_currTime >= t_quarterTime * 2.0f && t_currTime < t_quarterTime * 3.0f){
			t_sunAngle = (t_currTime - t_quarterTime * 2.0f) / t_quarterTime * t_rate * (piDiv) + piDiv;
		}else{
			t_sunAngle = (piDiv * (1.0f - t_rate)) + pi + (t_currTime - t_quarterTime * 3.0f) / t_quarterTime * t_rate * piDiv;
		}
		
		float t_cos;
		float t_sin;

		__asm
		{
			fld		t_sunAngle
			fsincos
			fstp	t_cos
			fstp	t_sin
		}
		
		t_cos = t_cos * h;
		t_sin = t_sin * h;

		if(_sunDir){
			_sunDir->x = -t_cos;
			_sunDir->y = -t_sin;
			_sunDir->z = 0.0f;
		}

		if(_moonDir){
			_moonDir->x = t_cos;
			_moonDir->y = t_sin;
			_moonDir->z = 0.0f;
		}
	}

	unguard;
}
void CLightManger::DrawPointLineCircle(void)
{
	guard;

	if(!IsEnableLight()) return ;

	osassert(m_refD3Device);

	for(size_t i = 0;i < m_vectDrawCircle.size();i++){

		CPointLightObj* t_pointLight = m_vectDrawCircle[i].m_pointLight;

		if(t_pointLight->GetEffectRange() > 0.1f){

			LPDIRECT3DVERTEXBUFFER9 t_horz = m_vectDrawCircle[i].m_lpDrawCircleBuf_horz;
			LPDIRECT3DVERTEXBUFFER9 t_vert = m_vectDrawCircle[i].m_lpDrawCircleBuf_vert;

			osassert(t_pointLight);
			osassert(t_horz);
			osassert(t_vert);

			D3DXMATRIX t_mat;
			D3DXMatrixIdentity(&t_mat);
			D3DXMatrixTranslation(&t_mat,t_pointLight->GetLightPos().x,t_pointLight->GetLightPos().y,t_pointLight->GetLightPos().z);
			D3DXMATRIX t_matScale;
			D3DXMatrixIdentity(&t_matScale);
			D3DXMatrixScaling(&t_matScale,t_pointLight->GetEffectRange(),t_pointLight->GetEffectRange(),t_pointLight->GetEffectRange());

			D3DXMatrixMultiply(&t_mat,&t_matScale,&t_mat);
			VALIDATE_D3D(m_refD3Device->SetTransform(D3DTS_WORLD,&t_mat));
			
			VALIDATE_D3D(m_refD3Device->SetTexture(0,NULL));
			VALIDATE_D3D(m_refD3Device->SetFVF(scm_MyFVF));
			VALIDATE_D3D(m_refD3Device->SetStreamSource(0,t_horz,0,sizeof(Vertex)));
			VALIDATE_D3D(m_refD3Device->DrawPrimitive(D3DPT_LINESTRIP,0,scm_cricleSegment));

			VALIDATE_D3D(m_refD3Device->SetStreamSource(0,t_vert,0,sizeof(Vertex)));
			VALIDATE_D3D(m_refD3Device->DrawPrimitive(D3DPT_LINESTRIP,0,scm_cricleSegment));

		}
	}

	unguard;
}
//! create the vertex buffer
void CLightManger::CreateCircleVertexBuf(CPointLightObj* _point)
{
	guard;

	osassert(_point);
	osassert(m_refD3Device);

	const float t_pi = 3.1415926f;

	LPDIRECT3DVERTEXBUFFER9		t_lpDrawCircleBuf_horz;
	LPDIRECT3DVERTEXBUFFER9		t_lpDrawCircleBuf_vert;


	// horz vertex buffer create
	//
	VALIDATE_D3D(m_refD3Device->CreateVertexBuffer(sizeof(Vertex) * (scm_cricleSegment + 1),
												D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
												scm_MyFVF,
												D3DPOOL_DEFAULT,
												&t_lpDrawCircleBuf_horz,
												NULL));

	Vertex* t_vertexBuf = NULL;
	VALIDATE_D3D(t_lpDrawCircleBuf_horz->Lock(0,0,(void**)&t_vertexBuf,NULL));
	
	for(size_t i = 0;i < scm_cricleSegment + 1;i++){
		t_vertexBuf[i].m_diffuse = 0xffffffff;
		const float t_rad = ( (float) i / (float)scm_cricleSegment) * (t_pi ) * 2.0f;
		t_vertexBuf[i].m_pos.x	= sin(t_rad);
		t_vertexBuf[i].m_pos.y	= 0.0f;
		t_vertexBuf[i].m_pos.z	= cos(t_rad);
	}

	VALIDATE_D3D(t_lpDrawCircleBuf_horz->Unlock());

	// vertical vertex buffer create
	//
	VALIDATE_D3D(m_refD3Device->CreateVertexBuffer(sizeof(Vertex) * (scm_cricleSegment + 1),
												D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
												scm_MyFVF,
												D3DPOOL_DEFAULT,
												&t_lpDrawCircleBuf_vert,
												NULL));

	t_vertexBuf = NULL;
	VALIDATE_D3D(t_lpDrawCircleBuf_vert->Lock(0,0,(void**)&t_vertexBuf,NULL));

	for(size_t i = 0;i < scm_cricleSegment + 1;i++){
		t_vertexBuf[i].m_diffuse = 0xffffffff;
		const float t_rad = ( (float) i / (float)scm_cricleSegment) * (t_pi ) * 2.0f;
		t_vertexBuf[i].m_pos.x	= 0.0f;
		t_vertexBuf[i].m_pos.y	= sin(t_rad);
		t_vertexBuf[i].m_pos.z	= cos(t_rad);
	}

	VALIDATE_D3D(t_lpDrawCircleBuf_vert->Unlock());

	m_vectDrawCircle.push_back(DrawCircle(t_lpDrawCircleBuf_horz,t_lpDrawCircleBuf_vert,_point));

	unguard;
}

void CLightManger::DeleteCircleVertexBuf(CPointLightObj* _point)
{
	guard;

	for(size_t i = 0;i < m_vectDrawCircle.size();i++){
		if(m_vectDrawCircle[i].m_pointLight == _point){
			SAFE_RELEASE(m_vectDrawCircle[i].m_lpDrawCircleBuf_horz);
			SAFE_RELEASE(m_vectDrawCircle[i].m_lpDrawCircleBuf_vert);

			m_vectDrawCircle.erase(m_vectDrawCircle.begin() + i);

			break;
		}
	}

	unguard;
}

//! read from the file
BOOL CLightManger::ReadFromFile(const char* _filename,ErrorOutPutFunc _outputFunc)
{
	guard;

	osassert(_filename);
	osassert(_outputFunc);

	ClearAllLight();

	CReadFile t_file;

	if(!t_file.OpenFile(_filename)){
		(*_outputFunc)("光源文件打开失败。");
		return FALSE;
	}

	FileHeader t_fileHeader;
	t_file.ReadBuffer(&t_fileHeader,sizeof(t_fileHeader));
	
	if(t_fileHeader.m_magicTex[0] != 'L'
		|| t_fileHeader.m_magicTex[1] != 'I'
		|| t_fileHeader.m_magicTex[2] != 'T'
		|| t_fileHeader.m_magicTex[3] != 0xa){

		(*_outputFunc)("文件头已经损坏，读取失败");

		return FALSE;
	}

	if(t_fileHeader.m_lightNum > scm_maxLightNumber){
		
		(*_outputFunc)("文件头已经损坏，导致最大光源数量过大。");

		return FALSE;
	}

	t_file.ReadBuffer(&m_currDayAmbientColor,sizeof(m_currDayAmbientColor));
	t_file.ReadBuffer(&m_currNightAmbientColor,sizeof(m_currNightAmbientColor));

	if(t_fileHeader.m_version >= 2){
		t_file.ReadBuffer(&m_currDayDirColor,sizeof(m_currDayDirColor));
		t_file.ReadBuffer(&m_currNightDirColor,sizeof(m_currNightDirColor));
	}else{
		// version 1 with direction color information
		//
		ZeroMemory(&m_currDayDirColor,sizeof(m_currDayDirColor));
		ZeroMemory(&m_currNightDirColor,sizeof(m_currNightDirColor));
	}
	
	for(size_t i = 0 ; i < t_fileHeader.m_lightNum;i++){
		CPointLightObj* t_obj = AddPointLight();
		t_obj->ReadFromFile(t_file,scm_version);
	}

	(*_outputFunc)(va("\n%s读取成功",_filename));

	return TRUE;

	unguard;
}

//! write to file
void CLightManger::WriteToFile(const char* _filename)
{
	guard;

	osassert(_filename);

	FileHeader t_fileHeader;
	t_fileHeader.m_magicTex[0] = 'L';
	t_fileHeader.m_magicTex[1] = 'I';
	t_fileHeader.m_magicTex[2] = 'T';
	t_fileHeader.m_magicTex[3] = 0xa;

	t_fileHeader.m_version = scm_version;

	t_fileHeader.m_lightNum = m_lightContainer.size();

	FILE* t_file = fopen(_filename,"wb");
	osassert(t_file);

	fwrite(&t_fileHeader,1,sizeof(t_fileHeader),t_file);
	fwrite(&m_currDayAmbientColor,1,sizeof(m_currDayAmbientColor),t_file);
	fwrite(&m_currNightAmbientColor,1,sizeof(m_currNightAmbientColor),t_file);
	fwrite(&m_currDayDirColor,1,sizeof(m_currDayDirColor),t_file);
	fwrite(&m_currNightDirColor,1,sizeof(m_currNightDirColor),t_file);

	for(LightContainer::iterator it = m_lightContainer.begin();
		it != m_lightContainer.end() ;it++){
		
		CPointLightObj* t_point  = *it;
		t_point->WriteToFile(t_file);
	}

	fclose(t_file);

	unguard;
}