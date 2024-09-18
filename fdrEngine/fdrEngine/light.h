//=======================================================================================================
/*!  \file
*   Filename: light.h
*   Desc:     the light make the real world
*
*   His:      tzz created @ 2009-9-4
*
*  
*/
//=======================================================================================================
#pragma once

// #ifdef SCENE_EDITOR 
// 	#include "../../gameTool/MapEditor/interface/fileOperate.h"
// #else
// 	#ifdef SCENE_RENDER
// 		#include "../../gameTool/MapEditor/interface/fileOperate.h"
// 	#else
		#include "../interface/fileOperate.h"
// 	#endif //SCENE_RENDER
// #endif


#ifndef VALIDATE_D3D
	# define VALIDATE_D3D(exp)	if(FAILED(exp)){ \
		osassert(false);\
	}
#endif //VALIDATE_D3D

typedef VOID WINAPI ErrorOutPutFunc(LPCSTR _errorInfo);

class CPointLightObj
{
public:

	struct Attenuation
	{
		//! the linear attenuation
		float				m_attenuation0;

		//! the power 1 attenuation
		float				m_attenuation1;

		//! the power 2 attenuation
		float				m_attenuation2;
	};

private:
	//! the position of point light
	D3DXVECTOR3			m_lightPos;

	//! the day color
	D3DXCOLOR			m_dayColor;

	//! the night color
	D3DXCOLOR			m_nightColor;

	//! the effect range
	float				m_effectRange;

	//! the light attenuation
	Attenuation			m_lightAtten;

	//! the user data (to connect the scene render) needn't write to file
	void*				m_renderData;

public: 
	CPointLightObj(void);
	CPointLightObj(const D3DXVECTOR3 _pos);
	CPointLightObj(const CPointLightObj& _obj);
	~CPointLightObj(void);

public:

	//! get/set the light pos
	const D3DXVECTOR3& GetLightPos(void)const{return m_lightPos;}
	D3DXVECTOR3 GetLightPos(void){return m_lightPos;}
	void SetLightPos(const D3DXVECTOR3& _pos){m_lightPos = _pos;}

	//! set/get the effect range
	void SetEffectRange(float _range){m_effectRange = _range;}
	float GetEffectRange(void)const{return m_effectRange;}

	//! set the attenuation
	void SetAttenuation(float _attenuation0,float _attenuation1,float _attenuation2)
	{
		m_lightAtten.m_attenuation0 = _attenuation0;
		m_lightAtten.m_attenuation1 = _attenuation1;
		m_lightAtten.m_attenuation2 = _attenuation2;
	}

	Attenuation GetLightAtten(void){return m_lightAtten;}
	const Attenuation& GetLightAtten(void)const{return m_lightAtten;}

	//! set and get the render data
	void* GetRenderData(void)const{return m_renderData;}
	void SetRenderData(void* _data){m_renderData = _data;}

	//! set and get the day and night color
	void SetDayColor(D3DXCOLOR _color){m_dayColor = _color;}
	const D3DXCOLOR& GetDayColor(void)const{return m_dayColor;}
	D3DXCOLOR GetDayColor(void){return m_dayColor;}

	void SetNightColor(D3DXCOLOR _color){m_nightColor= _color;}
	const D3DXCOLOR& GetNightColor(void)const{return m_nightColor;}
	D3DXCOLOR GetNightColor(void){return m_nightColor;}




public:

	//! read from the file
	void ReadFromFile(CReadFile& _file,DWORD _version);

	//! read from the file
	void WriteToFile(FILE* _file);

	//! Set the d3d light 
	//! \param _dayTime			: the 24 hours multiply 60
	void SetLight(LPDIRECT3DDEVICE9 _device,DWORD _index,size_t _daySecondTime)const ;

};

// the light manage connect the main engine and the light data...
//
class CLightManger
{
public:
	
	//! the current version
	static const DWORD				scm_version = 2;

	//! max light number
	static const size_t				scm_maxLightNumber = 500;

	//! max enable light
	static const size_t				scm_maxEnableLightNum = 8;

	//! the max geo chunck effect point number
	static const size_t				scm_maxEffectGeoChunckNum = 4;

	//! the max skin mesh effect point number
	static const size_t				scm_maxEffectSkinMeshNum = 2;
    
private:

	//! the header 
	struct FileHeader
	{	
		//! the magic tex
		char				m_magicTex[4];

		//! file version
		DWORD				m_version;

		//! the light number...
		size_t				m_lightNum;

	};

	//! this data if fill to character.fx
	struct PointLightEffData
	{
		//! the light pos;
		D3DXVECTOR3			m_lightPos;

		//! the light color
		D3DXCOLOR			m_lightColor;

		//! the effect range
		float				m_effectRange;

		//! the attenuation
		CPointLightObj::Attenuation
							m_attenuation;


	};

	typedef std::list<CPointLightObj*>	LightContainer;
	//! current light contain
	LightContainer					m_lightContainer;

	//! current enable light number
	size_t							m_enableLightNum;

	//! is enable light number
	BOOL							m_bEnableLight;

	//! D3D device 
	LPDIRECT3DDEVICE9				m_refD3Device;

	//! current day time (0 ~ 24* 60)
	size_t							m_currDayTime;

	//! current day ambient color
	D3DXCOLOR						m_currDayAmbientColor;

	//! current night ambient color
	D3DXCOLOR						m_currNightAmbientColor;

	//! current day direction color
	D3DXCOLOR						m_currDayDirColor;

	//! current night direction color
	D3DXCOLOR						m_currNightDirColor;

	//! the constance light for local hero's view
	CPointLightObj*					m_constLightPtr;

	//! the Direction light 
	D3DLIGHT9						m_directLight;

	//! render sort vector
	std::vector<const CPointLightObj*>	m_vectRenderPointSort;

	//@{ editor struct and variables
	const static int				scm_cricleSegment = 18;

	const static DWORD				scm_MyFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

	//! vertex 
	struct Vertex
	{
		D3DXVECTOR3		m_pos;
		DWORD			m_diffuse;
	};

	struct DrawCircle
	{
		LPDIRECT3DVERTEXBUFFER9		m_lpDrawCircleBuf_horz;
		LPDIRECT3DVERTEXBUFFER9		m_lpDrawCircleBuf_vert;
		CPointLightObj*				m_pointLight;

		DrawCircle(LPDIRECT3DVERTEXBUFFER9 _buf_horz,
			LPDIRECT3DVERTEXBUFFER9 _buf_vert,
			CPointLightObj* _pointLight):m_lpDrawCircleBuf_horz(_buf_horz),
										m_lpDrawCircleBuf_vert(_buf_vert),
										m_pointLight(_pointLight){}
																			
	};

	//! draw point line vertex buffer
	std::vector<DrawCircle>	m_vectDrawCircle;
	//@}

public:

	CLightManger(void);
	~CLightManger(void);

public:

	//! initialize the light
	void Initlize(LPDIRECT3DDEVICE9 _dev);

	//! Enable Light
	void EnalbeLight(BOOL _enable);
	BOOL IsEnableLight(void)const{return m_bEnableLight;}

	//! get/set the Enable light number
	void SetEnabledLightNumber(size_t _num){m_enableLightNum = _num;}
	size_t GetEnabledLightNumber(void)const{return m_enableLightNum;}

	//! set the current daytime
	void SetCurrentDayTime(size_t _dayTime){m_currDayTime = _dayTime;}
	size_t GetCurrentDayTime(void)const{return m_currDayTime;}

	//! get the effect light number
	size_t GetEffectLightNum(void)const{return m_enableLightNum;}

	//! clear all light;
	void ClearAllLight(void);

	//! focus center render
	D3DXCOLOR FocusCenterRender(D3DXVECTOR3 _focus,float _radius,D3DXVECTOR3& _dirLight,D3DXCOLOR& _dirColor);

	//! add a point light
	CPointLightObj* AddPointLight(void);
	CPointLightObj* AddPointLight(const CPointLightObj& _obj);
	CPointLightObj* AddPointLight(const D3DXVECTOR3& _pos);

	//! delete point light
	void DelPointLight(void* _renderData);

	//! search point light by the render data
	CPointLightObj* SearchLight(void* _renderData);
	//! search point light by the position
	CPointLightObj* SearchLight(const D3DXVECTOR3& _pos);

	//! set current Day/Night ambient color
	void SetCurrDayAmbientColor(const D3DXCOLOR& _color);
	void SetCurrNightAmbientColor(const D3DXCOLOR& _color);

	//! get the current day and night ambient color
	const D3DXCOLOR& GetCurrDayAmbientColor(void)const{return m_currDayAmbientColor;}
	D3DXCOLOR GetCurrDayAmbientColor(void) {return m_currDayAmbientColor;}
	
	const D3DXCOLOR& GetCurrNightAmbientColor(void)const{return m_currNightAmbientColor;}
	D3DXCOLOR GetCurrNightAmbientColor(void) {return m_currNightAmbientColor;}

	const D3DXCOLOR& GetCurrNightDirColor(void)const{return m_currNightDirColor;}
	D3DXCOLOR GetCurrNightDirColor(void) {return m_currNightDirColor;}

	const D3DXCOLOR& GetCurrDayDirColor(void)const{return m_currDayDirColor;}
	D3DXCOLOR GetCurrDayDirColor(void) {return m_currDayDirColor;}

	//! set the constance light attribute for the local hero's view
	void SetConstLightAttr(const D3DXCOLOR& _color,const D3DXVECTOR3& _pos,float _range);

	//! set the direction light day and night color
	void SetCurrDayDirColor(const D3DXCOLOR& _color){m_currDayDirColor = _color;}
	void SetCurrNightDirColor(const D3DXCOLOR& _color){m_currNightDirColor = _color;}

	//! effect the skin mesh of main engine
	int EffectSkinMesh(const D3DXVECTOR3& _focus,float _radius,const D3DXVECTOR3& _skinMeshPos,LPD3DXEFFECT _eff);

	//! unEffect the skinMesh
	void UnEffectSkinMesh(LPD3DXEFFECT _eff);

	//! effect the chunk
	int EffectChunk(const D3DXVECTOR3& _focus,float _radius,const D3DXVECTOR4& _chunkRetangle,LPD3DXEFFECT _eff);

	//! get the sun/moon position
	void GetSunMoonDir(D3DXVECTOR3* _sunDir,D3DXVECTOR3* _moonDir)const ;

	//! Draw point circle line
	void DrawPointLineCircle(void);
	
public:
	
	//! read from the file
	BOOL	ReadFromFile(const char* _filename,ErrorOutPutFunc _outputFunc);

	//! write to file
	void	WriteToFile(const char* _filename);

private:

	//! create the vertex buffer
	void CreateCircleVertexBuf(CPointLightObj* _point);
	//! Delete the vertex buffer
	void DeleteCircleVertexBuf(CPointLightObj* _point);

	//! sort the light and effect the effect
	int SortLightAndEffect(CLightManger::PointLightEffData t_data[],const size_t _dataLen,const D3DXVECTOR3& _focus);

	//! can effect the light
	static BOOL	CanEffectLight(const CPointLightObj* _light,const D3DXVECTOR3& _focus,float _range);
	static BOOL	CanEffectLight(const CPointLightObj* _light,const D3DXVECTOR4& _retangle);

	//! find the render data
	class FindPointLightByRenderData
	{
		void* m_renderData;
	public:
		FindPointLightByRenderData(void* _renderData):m_renderData(_renderData){}

		bool operator()(CPointLightObj* _light)
		{
			return _light->GetRenderData() == m_renderData;
		}
	};

	//! find the render data
	class FindPointLightByPos
	{
		D3DXVECTOR3 m_position;

	public:
		FindPointLightByPos(const D3DXVECTOR3& _position):m_position(_position){}

		bool operator()(CPointLightObj* _light)
		{
			return _light->GetLightPos() == m_position;
		}
	};

	//! delete the data
	class DelPointLightFunc
	{
		
	public:
		DelPointLightFunc(){};
		void operator()(CPointLightObj* _point)
		{
			delete _point;
		}
	};

	//! effect the skin mesh and chunk function
	class EffectObject
	{
		bool				m_bEffectChunk;
		PointLightEffData*	m_fillData;
		D3DXVECTOR3			m_focus;
		float				m_radius;
		D3DXVECTOR3			m_skinMeshPos;
		D3DXVECTOR4			m_retangle;

		//! copy array to sort and effect 
		typedef std::vector<const CPointLightObj*> EffectPointContain;
		EffectPointContain&	m_vectEffectPoint;
			

	public:

		//! effect Object
		EffectObject(PointLightEffData* _fillData,
					const D3DXVECTOR3& _focus,
					float _radius,
					bool _bEffectChunk,
					EffectPointContain& _vect,
					const D3DXVECTOR3 _skinMeshPos = D3DXVECTOR3(0.0f,0.0f,0.0f),
					const D3DXVECTOR4& _retangle = D3DXVECTOR4(0.0f,0.0f,0.0f,0.0f)):m_fillData(_fillData),
																	m_focus(_focus),
																	m_radius(_radius),
																	m_bEffectChunk(_bEffectChunk),
																	m_retangle(_retangle),
																	m_skinMeshPos(_skinMeshPos),
																	m_vectEffectPoint(_vect)
		{
			m_vectEffectPoint.clear();
		}

		void operator()(const CPointLightObj* _point);
	};

	//! effect render sort
	class EffectSort_less : std::less<CPointLightObj*>
	{
		D3DXVECTOR3			m_focus;

	public:
		EffectSort_less(const D3DXVECTOR3& _focus):m_focus(_focus){}

		bool operator()(const CPointLightObj* _left,const CPointLightObj* _right);
	};
};

extern CLightManger* g_ptrLightMgr;