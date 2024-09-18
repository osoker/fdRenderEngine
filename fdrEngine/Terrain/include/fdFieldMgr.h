//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldMgr.h
 *
 *  His:      River created @ 2003-12-23
 *
 *  Des:      ���ڹ����ͼ���ӵ��࣬ʹ�õ�ǰ��������õ���ǰӦ�õ�����һ�ŵ�ͼ��
 *   
 * ����ּ���֮�ǣ����������������ǽ֮��Ҳ���� 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# ifndef __FDFIELDMGR_INCLUDE__
# define __FDFIELDMGR_INCLUDE__

# pragma warning( disable : 173 )

//! ������ͼ�������õ�����ص����ݽṹ��
# include "fdTGQuadtree.h"
# include "../include/fdFieldStruct.h"
# include "fdNewFieldStruct.h"
# include "fdTerrainDecal.h"
# include "../../mfpipe/include/Font.h"
# include "../../effect/include/fdRain.h"
# include "../../effect/include/fdlensflare.h"
# include "../GeoMipTerrain/CeoMipTerrainAll.h"
# include "../../effect/include/fdSpecialEffect.h"
# include "../../effect/include/SceneLight.h"
# include "../../fdrEngine/light.h"

//@{ tzz add for galaxy space hole and other effection

# include "../../fdrengine/sceneGrow.h"
# include "../../fdrengine/sceneHDR.h"


# if GALAXY_VERSION


# include "../../fdrengine/GridCurve.h"

# include "../../fdrengine/coronaEff.h"
# include "../../fdrengine/blood.h"

# endif //GALAXY_VERSION
//@}

# include "QuadTree.h"
# include <list>

class osHDRLight;
extern float gOSAPLAYERSpeed ;
//! �ͻ���������ʹ�ö�����cursor���֡�
# define  MAX_SCECURSOR_NUM  4


//! ��������������֮����������20����Ʒ��������������Ŀ��
//!  �򳬳�������Ʒ������͸����
# define  MAXOBJNUM_TOCHAR   32

//! ��Ⱦ���ʱ���õ���Զ������.
# define  SKYRENDER_FARDIS 1200.0f


//! �Ƿ�ʹ��Lens Flare
# define USE_LENSFLARE


//! �����Զ�����ײ��Ϣ���ļ�ͷ�Ͱ汾��.
# define  CCOL_HEADER    "ccl"
# define  CCOL_VERSION    0x100

//! ����skinMesh������.
class      osc_skinMeshMgr;
class      osc_boltMgr;

struct     os_sceneTexName
{
	char   m_szTexname[32];
};



/** \brief
 *  ������������Ʒ���࣬���ھ�̬����Ʒ�������ڵ�һ�ν���ĳ�����ʱ��.
 *  �����е���Ʒ���������ڴ档
 */
struct     os_sceneObjectName
{
	char   m_szObjName[32];
	DWORD  m_dwObjType;
};

//! �������������е���Ʒ���ڴ�����
struct     os_sceneOjbMemData
{
	//! �����Ʒ�ڳ����е������Ǿ�̬��Ʒ�������˾�̬��Ʒ���ڴ�
	int    m_iSize;
	BYTE*  m_arrOjbMemFile;
};

//! �������֧��10��ˮ��߶�.
# define MAX_WATERHEIGHT   10

//! ���������õ�������,����ȫ�������������ֵȵ�.
struct     os_wholeSceneData
{
	int                m_iTexNum;
	os_sceneTexName*    m_arrTexnameList;

	VEC_int            m_iShaderId;

	//! ��ǰ������Ӧ�ı��������ļ���.
	char               m_szBgMusic[32];

	//! ��ǰ�����ж�Ӧ������Object�ļ���
	int                m_iObjectNum;
	os_sceneObjectName*  m_arrObjNameList;

	//! ��ǰ����ˮ��ĸ߶�
	float              m_fWaterHeight;

private:
	//! ����һ������,�ɴ��������������ڶ���߶ȵ�ˮ��
	float              m_vecWaterHeight[MAX_WATERHEIGHT];
	int                m_iWHNum;
public:
	os_wholeSceneData();
	~os_wholeSceneData();

	//! �������������е���Ʒ���ڴ�
	bool              init_wholeSceneData( void );

	//! push water height.
	void              push_waterHeight( float _wh );

	//! �õ���ǰ���ѧ��ˮ��߶�.
	float             get_waterHeight( void );

	//! 
	void              release( void );


};

# include "fdTGLoadReleaseMgr.h"

typedef std::vector<os_terrainTexSnd>  VEC_terrainTexSnd;

extern osMatrix  g_sMatReflectView;
extern osMatrix  g_sMatReflectProj;


//! River @ 2010-6-24:ȫ�ֵı�����ȷ���Ƿ���Ҫ��Ⱦ��Ļ�Ⲩ.
extern BOOL      g_bRenderScreenWave;

/** \brief
 *  ��֯������ͼ���ࡣTileGroundManager.
 *  
 *  �������ȷ����һ����ͼ��Ҫ���뵽�ڴ棬��һ����ͼ��Ҫ���ڴ���
 *  �ͷ��Խ�ʡ�ڴ�ռ䡣
 *  
 */
class osc_TGManager : public I_fdScene 
{
	friend class osc_newTileGround;
	friend class osc_mapLoadThread;
	friend class osc_tgLoadReleaseMgr;
	friend class osc_mapTerrain;
	friend struct os_aniNode;

public:

	void register_updateLoadingStepCallbackFunc( UPDATE_LOADINGSTEPCALLBACKIMM _pFunc );
	static UPDATE_LOADINGSTEPCALLBACKIMM m_pUpdate_uiLoadingStep;
	
	// HDR Light
	void init_hdrLight();
	void set_hdrRenderTarget();
	void set_hdrParameter( float _fmx, float _fmy, float _diffuse, float _exposure );
	void set_hdrFm( float _fm );
	void set_hdrDiffuse( float _diffuse );
	void set_hdrExposure( float _exposure );
	static void ReleaseStatic(void);


	//! River added @ 2009-7-23:ȫ�ֵõ�ˮ��ĸ߶�
	float       get_waterSurfaceHeight( void );



private:
	//! HDR light
	osHDRLight*    m_pHDRLight;

	//! �ӵ�ͼ�ĵ�����û��������еõ����Բ����ĵ�ͼָ�롣
	osc_TileGroundPtr*  m_arrPtrTG;
	//! ��ǰ֡��Ҫ�����ĵ�ͼ��Ŀ��
	int               m_iActMapNum;

	//!��ȷ�ཻ�����ı���
	osVec3D           m_vec3PickPos;


	/** \brief
	 *  ���ι�����
	 */
	osc_tgLoadReleaseMgr*  m_ptrTerrainMgr;



	//! ��ǰ��ͼ�����֣�Ŀ¼��ǰ��ı�ʶ����������������ֺ�.map��չ��
	s_string         m_strMapName;


	//! ��ǰ������ڵ�ͼ���������������Ӧ����һ����ʼֵ��
	//! ��ʼֵΪ0,0.
	os_tileIdx        m_vec2CamInIdx;

	//@{ 
	/** \brief
	 *  X,Z�����ϵ�ͼ����������ֵ,��ֵΪ��ʾ���ӵ�ǣ��ֵ.
	 *  ��64Ϊ��׼,Ϊ64�ı���.
	 */
	int              m_iMaxX;
	int              m_iMaxZ;

	//! ��x,z����������TileGround��Ŀ��
	int              m_iMaxXTgNum;
	int              m_iMaxZTgNum;
	//@}

	/** \brief
	 *  �����������������м�ܵ���
	 */
	osc_middlePipe    m_middlePipe;

	//! �����б�����豸��������ָ��,���ָ���ɳ������뺯������.
	static I_deviceManager*  m_deviceMgr;

	//! �����б�����������������ʵ����
	osc_skinMeshMgr*     m_skinMeshMgr;


	//@{
	/** ��Ч��صĹ�����,MeshMgrʹ�õ���ȫ�ֵı���
	osc_meshMgr*     g_ptrMeshMgr;
	*/

	//! �����д���billBoard�Ĺ�������
	osc_bbManager*        m_bbMgr;
	//! �����е����ӳ�����������ʹ�������������������Ⱦ���ӡ�
	osc_particleMgr       m_particleMgr;

	//@}


	//! �����д���bolt��mgr.
	osc_boltMgr*          m_boltMgr;
	
	//! ��ǰ�����еĶ�̬���������
	osc_dlightMgr        m_dlightMgr;

	//! ��ǰ�����й�����յĹ�����.
	osc_fdSkyMgr        m_skyMgr;

	//@{
	// Windy mod @ 2005-9-14 17:13:16
	//Desc: lens flare
	os_Lensflare *		m_pLensflare;
	//@}
	//! ���������,�������޸ķ����
	SceneLightMgr*		m_SceneFlashLightMgr;

	//! �����е���Ч����
	osc_rain            m_effectRain;
	//! �����е�ѩЧ��
	osc_snowStorm		m_effectSnow;

	//! �����е���ЧԪ��
	osc_specEffMgr*      m_sEffectMgr;

	//! �����ִ��õ�����ʱ��������
	os_stringDisp*       m_ptrDstr;

	//! �ر���Decal����ʾ����
	osc_decalMgr         m_sDecalMgr;

	//@{
	//
	//! ��������Ҫ��ʾ�Ķ�άͼƬ�����ݽṹ��
	VEC_screenPicDis     m_vecScrPicDis;
	int                 m_iCurScrDisNum;
	
	//! �Գ����еĶ�άͼƬ������Ⱦ��
	void                render_ScrPic( bool _LensPic = false,bool _renderPic = true );
	//@}


	//! ���ڵ�ѡ�����ڵ�Chunk���ӵİ�����
	osc_fdChunkPick     m_sChunkPick;

	//! �����ڻ���
	static LPD3DXLINE          m_ptrD3dLine;


	//! ���ŵ�ͼ�ķ�͸����ƷҲ�Ƿ���һ����Ⱦ
	VEC_meshRender       m_vecOpacityObj;
	//! ÿһ����Ⱦ��ɺ������ֵΪ�㡣
	int                 m_iOpacityObjNum;
	bool                flush_opacityObj( void );
	//! ��Ⱦ�������Ʒ
	bool                flush_opacityObjNoMaterial( ID3DXEffect* _pEffect,osMatrix& _ViewProjMat);


	//@{
	// �������ŵ�ͼ��Alpha��Ʒ����һ����Ⱦ.
	/** \brief
	 *  ��ǰ֡�洢����Ҫ��Ⱦ��͸���������Ϣ��
	 *��
	 *  ����Ⱦ֮ǰ��Ҫ����ÿһ����������������Զ����������
	 *  
	 */
	VEC_meshRender       m_vecAlphaObj;
	//! ÿһ����Ⱦ��ɺ������ֵΪ�㡣
	int                 m_iAlphaObjNum;
	//@} 

	/** \brief
	 *  �ѵ�ͼ�ϵ�͸�������������Ⱦ��
	 *  
	 *  \param bool _opacity ��Ⱦ͸�����е�͸�����ֻ��ǲ�͸�����֡�
	 *                       ��������Ⱦ����������Ҷ����Ⱦ�ĸ�����ȷ����һ������Ⱦ��Ҷ��
	 *                       ��͸�����֣��ڶ�����Ⱦ��Ҷ��͸�����֡�ʹ��alphaTest������
	 *                       ��ͬ����Ⱦ��River @ 2008-10-8
	 *  
	 *  render_TGsObject��ʱ�������͸�����壬��ŵ�һ�������У��ȴ��������ȥ������
	 *  
	 */
	bool              flush_alphaObj( bool _opacity );


	/** \brief
	 *  ��ǰ�����Ƿ�����˵�ͼ��
	 */
	bool              m_bLoadedMap;

	/** \brief
	 *  ���ڼ�¼�����������֮�����Ʒ���������ݡ�
	 *
	 *  ���нṹ�е�m_iX��ʾ��Ʒ����tileGround��TGMgr�е�������
	 *  m_iY��ʾtileGround����Ʒ��������
	 */
	os_tileIdx        m_vecCamChrObj[MAXOBJNUM_TOCHAR];
	int               m_iCamChrObjNum;

	//! ������������.
	os_wholeSceneData  m_sSceneTexData;

	
	//! �ر���������Ч��ص���Ϣ
	VEC_terrainTexSnd  m_vecTerrainTexSnd;
	bool               load_terrainTexSnd( const char* _mapName );

	/** \brief
	 *  ��lst�ļ��е��볡���������Ϣ.
	 *
	 *  \param _mapName Ҫ������Ϣ�ĵ�ͼ����,����"vilg"�ʹ���map\\vilgĿ¼�µ�map
	 */
	bool             load_sceneInfoFile( const char* _mapName );


	//! ��Ⱦ�ڵ���Ʒ�б�
	void             render_shelterObj( bool _alpha = true );


	/** \brief
	 *  �õ�������������������Ʒ���б�,�Եõ�����Ʒ���б���д���
	 *
	 *  ��ÿһ����Ʒ���䵽ÿһ��tileGround.
	 */
	bool             process_camChrObjList( void );


	//! ������Ч���Ĵ���
	void             render_rain( void );
	//! ��ѩ windy add 7.11
	void			render_snow(void);
	//! ����ȫ�ֵ�����Ⱦ״̬.
	void             fog_rsSet( void );

	//! ��ǰ֡����Ⱦ���е�ס����İ�͸����Ʒ.
	bool             m_bHasAlphaShelter;


private:
	//! �����б��浱ǰ�ɼ�����ͼ��
	VEC_tileIdx         m_vecVisualSet;
	int                 m_iVTNum;

	//! ��������Զ����͸��Ч����Tile���ϡ�
	VEC_tileIdx         m_vecVASet;
	int                m_iVATNum;


	//! �����е�ǰ֡��Ⱦ����������Ŀ��
	int                m_iRenderTriNum;


	//! ÿһ֡���浱ǰ��Ļ���������������άλ�ã��ɿͻ������ṩ�Ĳ��ձ�����
	int                m_iCenterX,m_iCenterY;
	//! ��ǰ�����������ڵ���άλ�á�
	osVec3D             m_vec3HeroPos;

	//@{ 
	//! �����е�����������ݽṹ��
	os_weather          m_sCurrentWeather;
	//! ���õ��ĳ����������ݽṹ.
	os_weather          m_sNewWeather;
	//! �Ƿ������������任��״̬.
	bool               m_bWeatherChanging;
	//! ��������Ҫ��ʱ��.
	float              m_fWeatherChangeTime;
	//! �������任�����ڹ�ȥ��ʱ��.
	float              m_fTimeFromChangeW;   

	//! windy 7.17 add
	//OSSoundItem*		m_TGSoundList;

	std::vector<OSSoundItem>	m_TGSoundList;
	std::vector<os_aabbox>		m_TGSoundAABBoxList;

	QuadTree<OSSoundItem>		mSoundQuadTree;
	std::vector<OSSoundPlayingItem>		mCurrentPlaySoundItemList;

	void frame_moveSound(osVec3D* _camPos,float _timeMs);
	//! ������Ч��������صĽӿ�
	static I_soundManager*  m_ptrSoundMgr;

	//std::vector<osVec3D>	mPassLineList;
	std::vector<std::vector<osVec3D> >  mPassLineListList;
	//std::vector<osVec3D>	mUnPassLineList;
	// windy add
	static int mrefMapId;
	//! river added @ 2009-6-29: refraction target.
	static int m_iRefractionMapId;
	static LPDIRECT3DSURFACE9  m_pRefractionSurface;
	static LPDIRECT3DTEXTURE9  m_pRefractionTexture;

	//int mRefTargetId;
	//int	mRefMapSurfaceId;
	//static IDirect3DTexture9* mpRefTexPtr;
	static LPDIRECT3DSURFACE9 mpRefMapSurface;
	static LPDIRECT3DTEXTURE9 m_pRefMapTexture;
	static LPDIRECT3DSURFACE9 m_pRefMapDepth;

	static LPDIRECT3DSURFACE9 m_pDepthStencilBackSur;

	/*std::vector<int>	m_VecSoundOffset;
	std::vector<int>	m_VecSoundCnt;*/

	CLightManger			  m_lightManager;

# if __QTANG_VERSION__
	//! River added @ 2009-5-19:Ϊ�˵�ͼ����ײ��Ϣ����������ϲ�һ�¡�
	char*      m_pMapCollInfo;
# endif 


	//! �ѵ�ǰ������������Ϣ�ռ�����ǰ���������ݽṹ��.
	void               gather_curWeather( void );

	/** \brief
	 *  �����жϵ�ǰ�������仯�Ƿ�󵽿������赱ǰ���ؾֱ���.
	 *
	 *  ��Ϊ���軷�����õ��ļ������Ƚϴ����������ı仯̫С��
	 *  ��ȵ��仯��һ���ĳ̶Ⱥ��ٱ仯
	 *
	 *  \param _etime ��ǰ֡��ȥ��ʱ�䡣
	 */
	bool               change_sceneAmbient( float _etime );

	//! ��������ۺϡ�
	void               frame_moveWeatherChanging( float _etime );

	//@} 

	//! �õ�λ�������봫��tg�߶��ཻ�ĸ߶����ݣ����ڸ���ȷ�ĵõ�������Ʒ����ײ��Ϣ
	float              get_intersectHeight( osc_TileGroundPtr  _tg,
		                     int _objNum,const int* _objList,osVec3D& _pos );

public:
	//! �õ�������ĳ��λ���ϸ��ر���Ʒ��صĸ߶���Ϣ,���û����Ʒ,��߶���ϢΪ��С�ĸ�ֵ.
	float              get_objHeightInfo( osVec3D& _pos ); 

	//! River added @ 2007-6-8:����ر�߶ȵ���Ϣ�����˴��ĸ߶ȣ�Ϊ���ر�߶ȣ�����Ⱦ�ر�����ݸ߶ȡ�
	bool               get_earthHeight( osVec3D& _pos );

	// River @ 2010-6-23:�����ڱ�ĵط�
	static LPDIRECT3DSURFACE9 mBackUpSurface;


	//@{
	// �����еĶ�̬����ص�����.
private:


	//! �Գ����еĶ�����ɫ����ʹ�û�����ϵ�����д���
	void               reset_ambientCoeff( void );


	//! ����animation Mesh�Ķ�̬�����ݡ�
	bool               process_aniMeshDlight( void );

	//@} 

	//! ��һ����������õ���ǰ���Ӷ�Ӧ����Ⱦpolygon.
	BOOL             get_rPolygon( osVec3D* _vec,int _tx,int _tz );

	/** \brief
	 *  ��ǰ��polygon�ı��ܷ������������ཻ��
	 *
	 *  \param _vs        �����polygon�Ŀ�ʼ���㡣
	 *  \param _idx       Ҫ�жϺ�polygon����һ�����Ƿ��ཻ��
	 *  \param _ray       ���ߵķ���
	 *  \param _rayStart  ���ߵĿ�ʼλ�á�
	 */
	bool               ray_intersectPolyEdge( osVec3D* _vs,
		                   int _idx,osVec2D& _ray,osVec2D& _rayStart );

# if WATER_ALPHA
	//! ��Ⱦ��������ˮ�ڽӵĵ�ͼtile��.
	void               render_waterTile( void );
# endif 

	//! ��Ⱦ��������Ʒ�Ĳ�͸�����֡�
	bool               render_opacityObjPart( bool _bridgeDraw = false );
	//! ��Ⱦ��������Ʒ��͸�����֡�
	bool               render_transparentObjPart( void );

private:


	/** \brief
	 *  �õ��µ�ͼ�����֡�
	 *
	 *  \param _nMname ������ļ�����Ҳʹ������ַ���ָ�뷵��ֵ��
	 *  \param _idr ����6�����򣬴�0��ʼ: x,xy,y,-x,-xy,-y
	 */
	bool                get_newMapName( char* _nMname,int _dir );


	/** \brief
	 *  �õ���ǰ�������������潻�����ڵ�TG.
	 */
	osc_TileGroundPtr      get_curCamIn_tground( void );


	/** \brief
	 *  ���ó������õ���Materialֵ��
	 *
	 *  \param _alpha ��ǰmaterial�õ���alphaֵ��
	 */
	void              set_material( float _alpha );

	//! ��ǰ����ײ���ӻ�Cursorʱ,ʹ�õر����ݻ���ʹ�ø߶�����.
	bool              weather_useTileHeight( int _x,int _y,osVec3D* _pos );

	//! ����һ����ײ�����ڳ����е�λ�ã��õ���ǰ��ײ���Ӷ�Ӧ���ĸ��������ݡ�
	bool              get_colVertexData( osVec3D* _pos,int _x,int _y );

	//
	// 
private:

	/** \brief
	 *  ������Ⱦ��պ�Զ���ĺ���.
	 */
	void                render_skyDome( bool _reflect = false );



	/** \brief
	*  �ж�һ�������Ƿ��һ��quad�ı����ཻ,�����ݴ���Ĳ���������õ������λ��
	*  ����Ľ��㡣
	*
	*  \param const osVec3D* _quadVer ������ı��ζ��㣬Ϊ���������Σ������������ɶ�������
	*                                 (0,2,1),(1,2,3) ���� 
	*  \param osVec3D& _pickVec     ����ཻ�����ұȴ���ľ���������������ء�
	*  \param osVec3D& _camPos      �����λ�á�
	*  \param float& _dis           �˴��ཻ����ǰ�õ���������������ľ��롣
	*/
	bool               pick_nearestTri( const osVec3D* _quadVer,const os_Ray &_ray,
		                           osVec3D& _pickVec,osVec3D& _camPos,float& _dis );


	//@{
	//! ����ѡ����ĺ������Էŵ�һ���µ�cpp�ļ���
	/** \brief
	 *  ��rayStart��ʼ������Chunk���ڵ�������Ʒ������ཻ���򷵻�true.
	 *
	 *  \param int _cnum �����chunk��Ŀ
	 *  \param const os_tileIdx* �����chunk�б�
	 */
	bool                  get_raypickBridge( osVec3D& _rayS,osVec3D& _rayDir,float& _resLength  );
		                           
	/** \brief
	 *  �µ�geomipmap�еõ���ǰ��������ڸ��ӵ�������
	 *
	 *  \param osVec3D& _pickVec �õ��ķ��ؽ����
	 *  \param os_tileIdx& _sidx ���������Ļ���ꡣ
	 *  \return bool             ������ر��н��㣬����true��
	 */
	bool				  get_newpickTileIdx(osVec3D & _pickVec,os_tileIdx& _sidx );

	//@}


	//! �������ڵ�ÿһ�������İ�Χ����
	void                  draw_soundBox( void );

public:
	osc_TGManager();
	~osc_TGManager();


	// windy mod
	void	UpDate_reflectionMap();

	//  River @ 2009-12-30: ������Ⱦ��ά�������õ��������Ч
	//! �߲�ͼ�����Ⱦ������ս��ʱ����Ч�����decal��
	void                  render_topLayer( I_camera* _cam,os_FightSceneRType _rtype );

	//! River added @ 2009-6-29:����refraction map.
	void    update_refractionMap( void );

	//! ��Ⱦ���������Ϣ
	void    UpDate_DepthMapAndRenderFog();
	/** \brief
	 *  ��ʼ��������ֻ�г�ʼ�������󣬲ſ��Ե����ʹ�ó���ָ���������Ĺ�����
	 *
	 *  ����һ�εõ�����ָ���ʱ�򣬵������������
	 */
	bool                init_sceneWork( void );



	/**********************************************************************/
	//
	//  �����ĳ�ʼ��.
	//
	/**********************************************************************/	

	/** \brief
	 *  ʹ�����ǵĳ�������ṹ���볡����ͼ��
	 *
	 */
	virtual bool          load_scene( os_sceneLoadStruct* _sl );

# if __QTANG_VERSION__
	//! ���ϲ����õ�ͼ����ײ��Ϣ,��ʵ��Щ������ȫ�����ɿͻ�����ɣ�Ϊ������ǰ�Ľӿڡ�
	virtual bool          set_sceneCollInfo( char* _coll );
# endif 


	//! �Ƿ�����˵�ͼ?
	virtual bool          isLoadMap(){ return m_bLoadedMap; }

	
	virtual osVec2D       get_mapSize();
	



# if __COLHEI_MODIFY__
	//! �����ͼ�ĸ߶Ⱥ���ײ������Ϣ��
	bool                 load_colheiAdjData( const char* _chName,os_TileCollisionInfo* _arr );
# endif 

	/** \brief
	 *  ����keyAniMesh.
	 *  
	 *  ���м�ܵ����𴴽���
	 */
	int                  create_keyAniMesh( os_aniMeshCreate& _mc );
	//! �ͷ�kenAniMesh.
	void                 release_keyAniMesh( int _id );


	/** \brief
	 *  Ϊ�����ͼ�е�˲���������Ľӿڡ�
	 *
	 *  \param _mapName ˲�ƺ��ͼ�����֣�����ʹ�ú͵�ǰ��ͼͬ�������֡�
	 *  \param _xCord,_yCord ˲�ƺ���������ꡣ
	 */
	virtual bool          spacemove_process( const char* _mapName,int _xCord,int _yCord );



	/** \brief
	 *  �ͷ�һ������.
	 *
	 *  �ͷ�������������е�����,�����ͼ���͵Ȳ�����Ҫ�����������仯��Ҫ
	 *  ʹ�����������
	 *  
	 */
	virtual bool          release_scene( bool _rlseScrPic = true );


	/** \brief
	 *  �õ��ɼ���? 
	 *
	 *  
	 */

	/** \brief
	 *  ������FrameMove.
	 *
	 *  
	 */
	virtual bool         frame_move( os_sceneFM* _fm );

	/** \brief
	 *  �õ�������ĳһ����ͼ����ײ��Ϣ���ݿ顣
	 * 
	 *  \param _tgIdx     �����е�ͼ����������(0,0)��ʼ��
	 *  \return os_TileCollisionInfo** ���ص���os_TileCollisionInfo�Ķ�ά���顣
	 *                                 ����os_TileCollisionInfo��128][128] 
	 */
	virtual const os_TileCollisionInfo* get_sceneColData( os_tileIdx& _tgIdx );
	/** \brief
	*  �õ�������ĳһ����ͼ�ĸ߶���Ϣ���ݿ顣
	*  windy add 4-27
	*  \param _tgIdx     �����е�ͼ����������(0,0)��ʼ��
	*  \return float*	���ص���65X6�ĸ߶���Ϣ����
	*/
	virtual const float*				get_sceneColHeightData(os_tileIdx& _tgIdx );

	//! ��һ����ʼ���һ��dir,������ر����������Ʒ��صĳ�����Ϣ��
	float                 get_pickRayLength( BOOL& _bridgeInter,
		                   osVec3D& _rayS,osVec3D& _rayDir,osVec3D& _pickVec );

	/** \brief
	 *  ��Ⱦ��ǰ�ĵ�ͼ��
	 *  
	 *  \param _scrPic �����ǰ����Ϊ�棬��ֻ��Ⱦ��Ļ�ڵĶ�άͼƬ��������Ⱦ����
	 *                 �Ӷ�άͼƬ��
	 *  \param _LensPic ��ʾ����Ⱦ�Ķ�άͼƬ��LensFlare����Ⱦ״̬��
	 *  \param _rtype   ��Ⱦ������ģʽ��Ϊ�غ�����Ϸ������
	 *                  �˲���ΪOS_COMMONRENDER����ͨ����Ⱦ
	 *                  ����ΪOS_RENDER_LAYER,�ֲ���Ⱦ������Ⱦ����������Ⱦ��ս���������Ч
	 *                  
	 */
	virtual bool          render_scene( bool _scrPic = false,
		                    bool _LensPic = false,os_FightSceneRType _rtype = OS_COMMONRENDER ); 

	/** \brief
	 *  �õ���ǰ֡��Ⱦ����������Ŀ��
	 */
	virtual int           get_renderTriNum( void );

	/** \brief
	 *  �õ���ǰ�����е�skinMesh�õ��������ε���Ŀ��
	 *
	 *  skinMesh��Ҫ�����������Ⱦ�����Դ˴��õ���ֵ��Ϊ�������Ƭ��Ŀ��
	 */
	virtual int           get_skinMeshTriNum( void );

	/** \brief
	 *  �ı䳡����������
	 *
	 *  \param _weather  �µĳ����������ݽṹ��
	 *  \param _itime    �ı�������Ҫ�������ۺ�ʱ��,����Ϊ��λ��
	 *  
	 */
	virtual void          change_sceneWeather( os_weather& _weather,float _itime );

	/** \brief
	 *  �ı䳡����Զ����ʾosaģ��.
	 *  
	 *  \param _terr Զɽ��ģ���ļ���.
	 */
	virtual void          change_sceneFarTerr( const char* _terr );


	/** \brief
	 *  �õ���ǰ��������ڸ��ӵ�������
	 *
	 *  �������ֵ�������Ŵ��ͼ�ϵĸ��ӵ�����ֵ�������ɷ�����ȷ�ϲ���Ч��
	 *  
	 */
	virtual bool          get_pickTileIdx( os_pickTileIdx& _dix,
		                    os_tileIdx& _sidx,osVec3D* _intersectPos = NULL );	


	/** \brief
	 *  �������м���billBoard.
	 *
	 *  �����е�Billboard�ڳ�ʼ����ʱ��̶�λ�ã��Ժ��ܸı䡣
	 *  
	 *  ATTENTION: ����Ǽ�����ѭ�����ŵĹ���壬����ɾ����
	 *             ���й�������Ҫ�Ǽ�������һ�εĹ���嶯����������ɺ󣬻��Լ�ɾ���Լ���
	 */
	virtual int           create_billBoard( const char *_bfile,osVec3D& _pos,
		                     bool _createIns = true,float _sizeScale = 1.0f,bool _topLayer = false  );

	//! �ڳ����ڴ�������Զ������Ӱ��Ĺ����,�����Ĳ����������Ĳ���һ��
	virtual int           create_hugeBillBoard( const char* _bfile,osVec3D& _pos,
		                    bool _createIns = true,float _sizeScale = 1.0f );

	//! �ͷ�һ���������Ʒ��
	virtual void          release_billBoard( int _id );

	//! ��֤billBoardId����Ч�ԡ�
	virtual bool          validate_bbId( int _id );


	/** \brief
	 *  ����billBoard��λ�á�һ�������еĹ���壬���ܿ���λ�á�
	 *
	 *  \param _id ��Ҫ���õĹ�����id��
	 *  \param _pos  Ҫ���õĹ�����λ��
	 */
	virtual void          set_billBoardPos( int _id,osVec3D& _pos );

	//! ���ù����ʹ�õ���ת��
	void                  set_bbRotAxis( int _id,osVec3D& _axis );

	/** \brief
	 *  ��ʱ���µ�ǰid�����ʵ����λ��
	 *
	 *  ���ڸ����ڳ��������FrameMove������׼ȷ�õ������λ�õ������
	 *  �����������õ��Ĺ���塣
	 */
	virtual void          update_bbPos( int _id,I_camera* _camptr );



	/** \brief
	 *  ���ù����Ĳ���ʱ�䡣
	 *  
	 *   \return ������Ҫ���ù�������������ʱ�䡣
	 *   \param _timeScale Ҫ���õĹ����Ĳ���ʱ������������ʱ��ı��������Ϊ��ֵ��
	 *                �ӿ�ֻ���ع�������������ʱ�䣬�����ò���ʱ�䡣
	 */
	virtual float         set_bbPlayTime( int _id,float _timeScale );


	/** \brief
	 *  ɾ��billBoard.
	 *
	 *  \param _id ɾ��ָ��id��billBoard.
	 */
	virtual void          delete_billBoard( int _id,bool _finalRelease = false );

	// River @ 2010-12-24: ��id�õ�billBoard���ļ���.
	const char*           get_bbNameFromId( int _id );


	//! ���ص�ǰ��billBoard.
	virtual void          hide_billBoard( int _id,bool _hide );
	//! ���ŵ�ǰ�Ĺ����.river added @ 2010-1-7:
	virtual void          scale_billBoard( int _id,float _rate );


	/** \brief
	 *  �ͻ��˵���ײ��⺯��.
	 *  
	 *  ����ͻ�����ײ��ⲻ��ͨ��������Ҫ���ɷ���������ȷ�ϣ�
	 *  �����ͨ�������ɷ�����ȷ�Ϻ����һϵ�еĶ����Ͳ����� 
	 *  
	 */
	virtual bool          collision_detection( const os_tileIdx& _tile );


	/** \brief
	 *  ����һ����ͼ���꣬�õ���ǰ��ͼ�������ĵ��ڶ�Ӧ����ά�������ǰ���겻�ڳ����ڣ�
	 *  ���Ӧ����ά�����yֵΪ0.0f.
	 */
	virtual bool          get_tilePos( int _x,int _y,osVec3D& _pos,BOOL _precision = FALSE  );

	virtual void	get_collision_info(const osVec3D& _pos,float _radius = 2);
	//! �õ�ÿһ����ײ����ϸ�ֳ�15�ݵĸ�����ά�ռ�λ��.
	virtual bool          get_detailTilePos( int _x,int _y,osVec3D& _pos );

	virtual int			  get_materialIdx(osVec2D& _pos,os_terrainTexSnd& _texsnd );
	/** \brief
	 *  ���������ڵ�һ��xzֵ���õ��˴��ĵر�߶�
	 *
	 *  \param osVec3D& _pos x,z��ֵ��_pos�ڵ�x,zֵ����.���صĸ߶�ֵ��_pos.y��
	 *  \param BOOL _precision �Ƿ�ȷ��ײ,��������Ʒ�߶���Ϣ,ֻ�����������ƶ�ʹ�˱���Ϊtrue
	 */
	virtual bool         get_detailPos( osVec3D& _pos,BOOL _precision = FALSE );

	/** \brief 
	*	����һ���������꣬����һ��DWORD��ֵ�����������������Ĺ���ͼ����ɫ��
	*
	*	\param osVec3D& _pos		x,z ������,Ϊ��������
	*/
	virtual DWORD		get_lightMapColor(const osVec3D& _pos);

	/** \brief
	 *  �ɴ��������ռ����㵱ǰ����ռ������һ����ͼ�ϣ��ڵ�ͼ����һ��ͼ����.
	 *  
	 *  \param  _res     ʹ��os_pickTileIdx�ṹ�������ǵõ���ֵ.
	 *  \param  _pos   ����������ռ�Ķ���λ�á�
	 */
	virtual bool                   get_sceneTGIdx( 
		                             os_pickTileIdx& _res,const osVec3D& _pos );

	/** \biref
	 *  �õ���ǰ�����ڵĵ�ͼ����Ŀ������.
	 * 
	 *  �ͻ�����Ѱ·�㷨ʱ���õ���Щ����.
	 *  
	 *  \return int     �����������ڵĵ�ͼ����Ŀ.
	 *  \param  _tgidx  ����ķ��ص�ͼ������ָ��ṹ,��෵��4��._tgidxӦ����os_tileIdx[4].
	 *  
	 */
	virtual int                   get_inViewTGIdx( os_tileIdx* _tgidx );

	//! �õ���ǰ�����п��Կ����ĸ��ӵ���Ŀ��
	virtual int                   get_inViewTileNum( void );

	//! ��һ��characterָ���ƽ���ǽ����Ⱦ������
	virtual void            push_behindWallRenderChar( I_skinMeshObj* _ptr );

	//! ��һ��characterָ���ǽ����Ⱦ������ɾ��
	virtual void            delete_behindWallRenderChar( I_skinMeshObj* _ptr );


	/** \brief
	 *  ���ڴ����Ͳ����ϲ���Ҫ��SkinMeshObject.
	 *
	 *  \param _smi ���ڳ�ʼ���´�����skinMeshObject.
	 *  \return     �������ʧ�ܣ�����NULL.
	 */
	virtual I_skinMeshObj*  create_character( os_skinMeshInit* _smi,
		                      bool _sceneSm = false,bool _waitForFinish = false );

	//@{
	//  �����й����صĽӿڡ�
	/** \brief
	 *  ʹ���ļ���������ꡣ
	 */
	virtual int            create_cursor( const char* _cursorName );
	//! ������������ʾ�Ĺ�ꡣ
	virtual void           set_cursor( int _id, int _centerX = 0, int _centerY = 0 );
	virtual int            get_cursor();
	virtual void           hide_cursor( bool _hide = false );
	//@} 
	

	//! ���ܵ����ƽ�һ��͸����Ⱦ��Ʒ.
	void                  push_alphaObj( os_meshRender& _mr );

	//! ���ܵ����ƽ�һ����͸������Ⱦ��Ʒ
	void                  push_opacityObj( os_meshRender& _mr );


	/** \brief
	 *  �õ���ͼ�е�һ�������Ƿ��ڿ��ӷ�Χ�ڡ�
	 *
	 *  ��tgMgrʹ���Լ��ķ����õ���
	 */
	bool                  is_ptInView( osVec3D& _vec );

	/** \brief
	 *  ���sphere,�鿴�Ƿ��ڳ�����.
	 *
	 *  \param _lod  �Ƿ�ʹ��lodѡ������sphere��������ӷ�Χ��,���ʹ�ã�
	 *               ��Զ��С����Ʒ���ɼ���
	 */
	bool                  is_sphereInView( os_bsphere& _sphere,bool _lod = false );

	/** \brief
	 *  �õ���ǰ�������������frustum.
	 */
	os_ViewFrustum*        get_camFrustum( void );

	/** \brief
	 *  �õ��м�ܵ�ָ��ĺ�����
	 *  
	 */
	osc_middlePipe*         get_middlePipe( void )         { return &m_middlePipe; }

	/** \brief
	 *  �õ������еĻ��������ݡ�
	 */
	DWORD                  get_ambientLight( void );


	//@{
	// ��̬����صĽӿڡ�
	/** \brief
	 *  ����һ����̬�⡣
	 *
	 *  \return int  �������-1,�򴴽�ʧ�ܡ�
	 */
	virtual int           create_dynamicLight( os_dlightStruct& _dl );

	/** \brief
	 *  �õ�һ����̬��ĸ��������
	 */
	virtual void          get_dynamicLight( int _id,os_dlightStruct& _dl );


	/** \brief
	 *  FrameMoveһ����̬�⡣
	 */
	virtual void          set_dynamicLight( int _id,os_dlightStruct& _dl );

	//! ���ö�̬���λ�á�
	virtual void          set_dLightPos( int _id,osVec3D& _pos );

	/** \brief
	 *  ɾ��һ����̬�⡣
	 */
	virtual void          delete_dynamicLight( int _id,float _fadeTime = 0.0f );

	/** \brief
	 *  �ı䳡���Ļ�����ϵ����
	 *
	 *  \param _r,_g,_b  �����л������ϵ����ʹ��������ֵ���ų����еĻ����⡣
	 */
	virtual void          change_amibentCoeff( float _r,float _g,float _b );

	//@}


	//@{
	// ����Ӱ��صĽӿڣ�����,npc�͹���������һ����ɫ��СȦ��
	virtual int           create_fakeShadow( osVec3D& _pos,float _scale = 1.0f );
	virtual void          set_shadowPos( int _id,osVec3D& _pos );
	virtual void          delete_shadowId( int _id );
	virtual void          hide_shadow( int _id,bool _hide = true );
	//@} 

	//! ������ɾ�������ر��ϵ�Decal
	virtual int           create_decal( os_decalInit& _dinit );
	virtual void          delete_decal( int _id );
	virtual void          reset_decalPos( int _id,osVec3D& _pos );


	//@{
	//! �����п�ʼ�ͽ�������Ч����
	virtual void          start_sceneRain( os_rainStruct* _rs = NULL );
	virtual void          stop_sceneRain( void );
	/** \brief
	 *  �����ڿ�ʼ�ͽ�����ѩ��Ч��
	 *
	 *  \param BOOL _start �������ʼ��ѩ�����Ϊfalse,������ѩ��Ч��
	 *  \param int _grade  ��Ϊ0,1,2�������𡣼���Խ�ߣ���ѩ��Խ�ࡣ
	 */
	virtual void          scene_snowStorm( BOOL _start = TRUE,int _grade = 0 );

	//@} 


	//{@ windy 6-21 add
	//! ��ʼ��������,
	/*!\param _times ,����ʱ�䳤��
	\param frequency һ����������.
	\param flashTime ÿ������,��ռʱ�� [0-1]֮����
	\param _color ������ɫ*/
	virtual void	start_sceneLight(float _times,float frequency,int flashtype,
		                float flashTime,osColor _color = D3DXCOLOR(1.0f,1.0f,1.0f,1.0f));
	virtual void stop_sceneLight();
	//@}


	//@{
	//  ���볡���еĶ�ά��ĻͼƬ��ʾ��Ҫ�Ľӿڡ�
	//  ��άͼƬ����ʾ��Ҫ�ɿͻ������Լ��������򣬰���Ҫ�ŵ��±ߵ�ͼƬ�����ƽ��е�����
	//  �н�����ʾ��
	
	/** \brief
	 *  ����һ����ά��ʾ��Ҫ������
	 *��
	 *  �����л���ʱ�����е�������ɳ��������Զ����ͷš�
	 */
	virtual int            create_scrTexture( const char* _texname );

	//! �õ�һ������
	virtual LPDIRECT3DTEXTURE9   get_sceneScrTex( int _id );

	//! testע��һ������,����id
	int			register_texid( LPDIRECT3DTEXTURE9 _pTexture );
	void				unregister_texid( int _texid );
	int		register_targetAsTex( int _targetIdx );

	//! ����һ������
	int					create_texture( int _width, int _height, LPDIRECT3DTEXTURE9* _pTexture );

	//syq_mask ������ȾĿ��
	int                        create_renderTarget( int _width, int _height, D3DFORMAT _format = D3DFMT_A8R8G8B8 );

	//syq_mask �����ȾĿ��: 
	LPDIRECT3DSURFACE9         get_renderTarget( int _id );
	LPDIRECT3DTEXTURE9      get_renderTexture( int _id );

	/** \brief
	*  �ͷ�һ����ͼ��
	 */
	virtual void           release_scrTexture( int _texid );


	/** \brief
	 *  ����Ҫ��ʾ�����ݽṹ�ƽ��������н�����ʾ��
	 *
	 *  ������ݱ��ƽ��е������е����ݽṹ�У�����ʾ�곡���е���άԪ�غ�
	 *  ��ʼ��ʾ��Щ��ά��Ԫ�ء�
	 */
	virtual void           push_scrDisEle( const os_screenPicDisp* _dis );
	//@}



	//@{
	// 
	// �����ж�����صĺ���.�˴��Ķ����ǹ�������.
	// �����ļ��ĸ�ʽΪ*.osa��ʽ�Ķ���.
	/** \brief
	 *  �����м��붯���ļ�,�������Ķ����ļ����ڵ�ǰ�������Ұ��,����Ҫ��Ⱦ�¼����
	 *  ����.
	 *
	 *  \param _mc �����������ݵĽṹ,�������ṹ,�������ǵĶ�������.
	 */
	virtual int           create_osaAmimation( os_aniMeshCreate* _mc,bool _topLayer = false );
	

	//! ��ʼ����
	virtual void          start_animationMesh( int _id );

	//! fade����fadeIn��ǰ��osa����.
	virtual void          start_fadeOsa( int _id,
		                    float _fadeTime,bool _fadeOut = true,float _minFade = 0.0f );

	//! ���ص�ǰ��osa����
	virtual void          hide_osa( int _id,bool _hide );


	//! ��Ҫ��֤ÿһ��������Ʒid�ĺϷ��ԡ�
	virtual bool          validate_aniMeshId( int _id );


	//! ����ĳһ��������λ�á�
	virtual void          set_aniMeshPos( int _id,osVec3D& _pos );

	//! ����һ����������ת����.
	virtual void          set_aniMeshRot( int _id,float _angle );
	//! ���ŵ�ǰ��osa����
	virtual void          scale_aniMesh( int _id,float _rate,float _time );


	//! ��ʱ����osa��λ��,��Ҫ����������Ч��
	void                  update_osaPosNRot( int _id );

	//! ����osa����Ⱦ״̬��ʹ��һ����Ⱦ��osa��ʱ���ܹ�����Ⱦ
	void                  update_osaRenderState( int _id );
	//! ����particle����Ⱦ״̬��ʹ��һ����Ⱦ��par��ʱ���ܱ���Ⱦ
	void                  update_parRenderState( int _id );



	/** \brief
	 *  ����osa�����Ĳ���ʱ�䡣
	 *  
	 *   \return ������Ҫ����osa��������������ʱ�䡣
	 *   \param _timeScale Ҫ���õ�osa�����Ĳ���ʱ������������ʱ��ı��������Ϊ��ֵ��
	 *                �ӿ�ֻ���ع�������������ʱ�䣬�����ò���ʱ�䡣
	 */
	virtual float         set_osaPlayTime( int _id,float _timeScale );


	/** \brief
	 *  ����һ���������������
	 *
	 *  ����ӿ���Ҫ����һЩ��Ч�Ĳ��ţ���Ҫ����Ϸ���������������ݡ�
	 */
	virtual void          set_aniMeshTrans( int _id,osMatrix& _transMat );

	//! ɾ��һ������.
	virtual void          delete_animationMesh( int _id,bool _sceC = false );

	const char*           get_aniNameFromId( int _id );
	//@} 

	//! �ϲ㲥���������Ч�Ľӿ�
	virtual DWORD         create_effect( const char* _ename,
		                    osVec3D& _pos,float _angle = 0.0f,
							BOOL _instance = TRUE,float _scale = 1.0f,
							float* _effectPlaytime = NULL );

	//! �õ�effect��Ӧ��boundingBox,����boxsize�Ǿ���scale���size.��box����ת
	virtual bool          get_effectBox( DWORD _id,osVec3D& _pos,osVec3D& _boxSize );

	//! �õ�effect�İ�Χ�У�����ת����İ�Χ��.
	virtual const os_bbox*   get_effectBox( DWORD _id );


	//! �ڲ�ʹ�õĽӿ�,���ڴ��������ڵ���Ч,�����ڵ���Ч,��������̬��.
	DWORD                 create_effect( const char* _ename,
		                    osVec3D& _pos,float _angle,float _scale,bool _inScene );

	//! River @ 2010-1-14:�ϲ���Ե�һ��effect��keyTime.
	virtual float          get_effectKeyTime( DWORD _id );

	//! ���ϲ�ɾ��һ����Ч
	virtual void          delete_effect( DWORD _id,bool _finalRelease = false );
	//! ����Ч��������,���ڶԳ����ڵĵ��߽������Ŷ������������Ž�����osa�����͹���壬���ӳ������ܶ�̬����
	virtual void          scale_effect( int _id,float _rate,float _time );
	//! ����Ч����fadeOut
	virtual void          fade_effect( int _id,float _fadetime,bool _fadeOut = true );

	//! River @ 2010-8-24:����Ч�Ĳ����ٶȽ������š�
	virtual void          scale_effectPlaySpeed( int _id,float _speedRate );


	/** \brief
	 *  ������Ч��alphaֵ,ֻ��osa��Ч������ȷ������alphaֵ��alphaֵ����0.0��1.0f֮�䡣
	 *
	 *  \param int _id   ��Ч��Ӧ��id.
	 *  \param float _alpha ��ЧҪʹ��alphaֵ
	 */
	virtual void          set_effectAlpha( int _id,float _alpha );


	//! ���ػ���ʾ��ǰid��effect
	virtual void		  hide_effect(int _id,bool _bHide = TRUE );


	//! �ϲ�������Ч��λ�ú���ת����
	virtual void          set_effectPos( DWORD _id,osVec3D& _pos,
		                    float _angle,float _speed = 1.0f,bool _forceUpdate = false );
	//! ��֤һ��effect�Ƿ��Ѿ����ڲ���ɾ��
    virtual BOOL          validate_effectId( DWORD _id );

	//@{
	//  �ͻ����õ����ض����ݣ�����ֻ������벢���ϲ���ã������ڲ�������
	//! �õ���Ч���ŵ�λ������ 0:��Χ�б������š� 1:����λ�ò��š�2:������λ�ò��š�
	DWORD    get_effectPlayPosType( int _id );
	//! �õ���Ч���ŵİ�Χ�б��������ص�ֵ��0.0��1.0֮�䡣
	float    get_effectPlayBoxScale( int _id );
	//@}


	/** \brief
	 *  ����һ��������ʾ�����λ�õı�־��
	 */
	virtual void	      draw_cursorInScene( os_sceneCursorInit& _scInit );




	//@{
	//  ������صĺ����ӿ�.����,���ú�ɾ��.
	//! ����һ��particleʵ��.
	virtual int           create_particle( os_particleCreate& _pcreate,BOOL _inScene,bool _topLayer = false );
	//! ���ص�ǰ��particle.
	virtual int           hide_particle( int _id,bool _hide );

	bool                   is_loopPlayParticle( int _id );

	//! ����particle��λ��.
	virtual void          set_particlePos( int _idx,osVec3D& _pos );
	virtual void          set_particlePos( int _idx,osVec3D& _pos,bool _forceUpdate );

	//! �������ӷ�ɢ�Ŀ�ʼ�ͽ����߶ε�
	virtual void          set_particleLineSeg( int _id,osVec3D& _start,osVec3D& _end );



	/** \brief
	 *  �������Ӷ����Ĳ���ʱ�䡣
	 *  
	 *   \return ������Ҫ�������Ӷ�������������ʱ�䡣
	 *   \param _timeScale Ҫ���õ����Ӷ����Ĳ���ʱ������������ʱ��ı��������Ϊ��ֵ��
	 *                �ӿ�ֻ���ع�������������ʱ�䣬�����ò���ʱ�䡣
	 */
	virtual float         set_parPlayTime( int _id,float _timeScale );


	//! ���particleId�Ŀ����ԣ���������ã����ϲ���Ҫ����ǰ��particle Id.
	virtual bool          validate_parId( int _id );

	//! ɾ��һ��particle.
	virtual void          delete_particle( int _idx,bool _finalRelease = false );
	//@} 

	//! ������Чɾ���봴������
	const char*           get_parNameFromId( int _id );


	//@{
	//  �绡��ص���Ч��
	/** \brief
	 *  ����һ���绡��
	 *
	 *  ����ͬʱʹ����������绡��Ӧһ��id
	 */
	virtual int           create_eleBolt( os_boltInit* _binit,int _bnum );
	//! �������û���
	virtual void          reset_eleBolt( os_boltInit* _bset,int _idx );
	//! ɾ��һ���绡��Դ��
	virtual void          delete_eleBolt( int _id );
	//@}



	//@{
	/** \brief
	 *  Draw text on the screen.  
	 *
	 *  ���������������ʾ������Ϣ����Ҫ�ĺ���,����֧��Unicode.�����������
	 *  ����beginScene��endScene����֮��.
	 */
	virtual int          disp_string( os_stringDisp* _dstr,int _dnum,int _fontType = 0,bool _useZ = false ,bool _bShadow = false, RECT* _clipRect = NULL );

	//! ��������Ļ����ʾ�ִ���������disp_string����һ�¡�
	virtual int          disp_stringImm( os_stringDisp* _dstr,int _fontType = 0,os_screenPicDisp* _cursor = NULL );


	//! ��Ⱦ����ɫ�������ܵ������ִ���
	virtual int          disp_string( os_colorInterpretStr* _str,int _fontType = 0,bool _useZ = false, bool _bShadow = false, RECT* _clipRect = NULL );
	//@}
	//@{
	// Windy mod @ 2005-9-20 15:05:38
	//Desc: ����Lens Flare�Ľӿ�
	 void	start_LensFlare() ;
	 void	stop_LensFlare() ;

	///���ļ��м���lens flare
	
	void	clear_LensFlare() ;

	void	set_RealPos(osVec3D & _vec3Pos);
	# if __LENSFLARE_EDITOR__
	///�����Ǳ༭���Ľӿ�
	virtual void	Load_LensFlare(const char * _fname) ;
	virtual void	set_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart) ;
	virtual void	get_LensFlarePart(int nPartID,LFMask _Mask,FlarePart & _flarePart) ;

	virtual int		get_LensFlareNum();

	virtual void	add_LensFlarePart();
	virtual void	del_LensFlarePart(int iPartID);

	virtual void	save_LensFlare(const char * _fname) ;
#endif
	//@}





# if __EFFECT_EDITOR__
	//! �õ���ǰ������������
	virtual void          get_sceneWeather( os_weather& _weather );

	//! ǿ�ƽ���һ������.
	virtual void          stop_animationMesh( int _id );

	//! ����ĳһ��������صĲ���.
	virtual bool          set_animationMesh( int _id,os_aniMeshCreate* _mc );
# endif 

	//@{
	//  �޸ĵ�ͼ�ĸ߶���Ϣ����ײ��Ϣ�Ľӿڡ�
# if __COLHEI_MODIFY__
	/** \brief
	 *  �޸�������������λ�õĸ߶���Ϣ��os_sceneFM�ڵ�m_iCx,m_iCy������ʾ��λ�á�
	 *
	 *  \param float _mod  Ҫ�޸ĵĸ߶ȣ������ɸ���
	 */
	virtual bool          modify_heightInHeroPos( float _mod );

	//! ���������λ������Ϊ��ԭ���෴�Ŀ�ͨ���ԡ�
	virtual bool          modify_colAtMousePick( os_pickTileIdx& _ps ,BOOL bUseParm=FALSE,bool bPass=false);
	
	//! �洢�޸ĺ����ײ��Ϣ�ļ�,�Ժ��������ںϵ���ͼ�ļ��С�
	virtual bool          save_colheiModInfo( FILE*& _file,int _x,int _z );

# endif
	//@} 



	//! �����ú�������һ��bounding Box.
	virtual void          draw_bbox( os_bbox* _box,DWORD _color,bool _solid = false );
# ifdef _DEBUG
	//! ���ڶ��²�ı����������.
	virtual void          debug_test( void );

# endif 

	//! �����ú�������һ����
	virtual void          draw_line( const osVec3D* _verList,int _vnum,
		                   DWORD _color = 0xffffffff,float _width =1.0f );

	//! �����豸lost�ͻظ��Ĺ���
	static void           on_deviceLost( LPDIRECT3DDEVICE9 pDev );
	static void           on_deviceRestor( LPDIRECT3DDEVICE9 pDev );


# if __ALLCHR_SIZEADJ__
	//! ʹ������ӿڣ����Ե�����������������skinMesh��Ʒ�Ĵ�С��
	//! �˴��ĵ���Ϊͳһ�������������Ͳ���ص���Ʒ��
	virtual void          set_allCharSizeAdj( osVec3D& _sizeAdj );
# endif 

	/** \brief
	 *  ��һ������õ���ǰTGMgr�е�ǰ�����osc_TileGroundPtr.
	 *
	 *  \param _x,_y ��������ΪTileGround��������
	 *  
	 *  River @ 2006-3-8: ��Ϊ���к�������Ҫ����decal�ļ��㡣
	 */
	osc_TileGroundPtr      get_tground( int _x,int _y );


	/** \brief
	 *  ����һ���������õ��Ƿ���ر��ཻ���Լ��ཻ���λ�á�
	 *  
	 *  ʹ�ø�����ѡ�ر�һ���ĺ����㷨��
	 *  \param osVec3D& _rayStart  ���ߵĿ�ʼλ�á�
	 *  \param osVec3D& _rayDir    ���ߵķ���
	 *  \param osVec3D& _resPos    ����ཻ����ֵΪ���صĽ���λ�á�
	 *  
	 */
	bool                  get_rayInterTerrPos( osVec3D& _rayStart,osVec3D& _rayDir,osVec3D& _resPos );  

	/** \brief
	 *  ����һ���߶�,��ȷ�ϴ��߶��Ƿ���ر��ཻ,�Լ������λ��.
	 *
	 *  ���������Ҫ�����Ż�������ر����ײ����,�����㷨���Ժ���get_rayInterTerrPos
	 *
	 *  \param osVec3D& _ls �߶εĿ�ʼλ��.
	 *  \param osVec3D& _le �߶εĽ���λ��.
	 *  \param osVec3D& _resPos ����ཻ,�����ཻ�Ľ���λ��.
	 *  \return bool            ����true,����ر��ཻ.
	 */
	bool                  get_lineSegInterTerrPos( osVec3D& _ls,osVec3D& _le,osVec3D& _resPos );

	/** \brief
	 *  ����һ����Χ����ر����ײ���.
	 *
	 *  �㷨�������:
	 *  ÿһ�μ���bsphere����ײʱ,��Ҫ������һ��û����ײ��bsphere���ڵ�λ��.�����ǰ��bsphere��
	 *  �ر�֮������ײ,���ҵ���bsphere����ܵ��ﵱǰ��λ��,����û�и��ر���ײ��λ��.
	 *  
	 *  \return bool �����ײ,����true,���򷵻�false.
	 *  \param  os_bsphere* _bs  Ҫ�����ε���ײ��Ϣ����ײ����bounding sphere
	 */
	bool	              get_sphereInterTerrPos( os_bsphere* _bs );

	bool				load_SoundInfo(const char* fileName);

	/** \brief
	 *  ��ǰ����ĵ�ͼ�Ƿ��Ѿ��������ڴ档
	 */
	bool                is_tgLoaded( int _x,int _y );

		
//--------------------------------------------------------------------------------
//@{ tzz added for galaxy shader effect
//--------------------------------------------------------------------------------

	//! whole scene growing manager class
	static CGrowEff		m_growEff;

	/*!	\fn virtual void gala_actGrowScene(BOOL _bActivate = TRUE);
	*	\brief : activate the grow scene after been initialized
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actGrowScene(BOOL _bActivate = TRUE) ;

	/*! \fn virtual void gala_initGrowScene(ini _iScreenWidth,int _iScreenHeight) ;
	*	\brief : initialize the grow scene 
	*
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\return void
	*/
	virtual void gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) ;

	//! whole scene real HDR manager class
	static CHDREff		m_realHDREff;

	//! initialize the hdr data
	virtual void init_realHDRScene(int _iScreenWidth,int _iScreenHeight) ;

	//! activate the HDR scene effect
	virtual void act_realHDRScene(BOOL _bActivate = TRUE);

	virtual void act_waterEff(BOOL _bAct = TRUE);

	//! enable light 
	virtual void enable_light(BOOL _bEnable);

	//! set the day time for light
	virtual void set_dayTimeForLight(size_t _dayTime);

	//! set the day time for light
	virtual void set_constLight(const osColor& _color,const osVec3D&,float _range);




# if  GALAXY_VERSION

	/*! \fn virtual void gala_genExplode(osc_mapTerrian::gala_exlode* _explode) = 0;
	*	\brief : generate a explode effect
	*
	*	\param _explode		:  struct of exploding effect
	*	\return void
	*/
	virtual void gala_genExplode(gala_explode* _explode);

	/*! \fn virtual void gala_genBullet(gala_spaceHole* _bullet) = 0;
	*	\brief : generate a bullet effect
	*
	*	\param _bullet		:  struct of bullet effect
	*	\return void
	*/
	///virtual void gala_genBullet(gala_spaceHole* _bullet) ;
	
	/*! \fn virutal void gala_delSpaceHole(int	_index);
	*	\brief : delete the shader effect of bullet
	*	
	*	\param _index		: index of bullet effect
	*	\return void		
	*/
	virtual void gala_delSpaceHole(int _index);

	/*! \fn virtual void gala_refreshSpaceHole(gala_spaceHole* _hole)
	*	\brief : refresh the space hole position and direction
	*
	*	\param _bullet : bullet struct
	*	\return void
	*/
	virtual void gala_refreshSpaceHole(gala_spaceHole* _hole);

	/*! \fn virtual void gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum);
	*	\biref : begin the space hole effect
	*	
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\param _iWidthNum		: horizontal grid Number
	*	\param _iHeightNum		: vertical grid Number
	*	\param _iStartX		: rotating center x
	*	\param _iStartY		: rotating center y
	*/
	virtual void gala_renderHole(int _iScreenWidth,int _iScreenHeight,int _iWidthNum,int _iHeightNum,int _iStartX,int _iStartY) ;

	/*! \fn virtual BOOL gala_isRenderHole(void);
	*	\biref : is render hole if yes return TRUE;
	*
	*	\param void
	*	\return BOOL	: TRUE if rendering hole;
	*/
	virtual BOOL gala_isRenderHole(void);

	/*! \fn virtual void gala_initGrowScene(ini _iScreenWidth,int _iScreenHeight) ;
	*	\brief : initialize the grow scene 
	*
	*	\param _iScreenWidth	: current screen Width
	*	\param _iScreenHeight	: curren screen Height
	*	\return void
	*/
	virtual void gala_initGrowScene(int _iScreenWidth,int _iScreenHeight) ;

	/*!	\fn virtual void gala_actGrowScene(BOOL _bActivate = TRUE);
	*	\brief : activate the grow scene after been initialized
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actGrowScene(BOOL _bActivate = TRUE) ;

	/*!	\fn virtual void gala_actHeroCorona(BOOL _bActivate = TRUE);
	*	\brief : draw the hero around corona
	*
	*	\param _bActivate : switch of active
	*	\return void
	*/
	virtual void gala_actHeroCorona(BOOL _bActivate = TRUE) ;

	/*!	\fn void gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP)
	*	\brief show the blood process of craft in game
	*
	*	\param _craftPos	: the screen position of craft
	*	\param _fCraftScale	: the mesh of craft scale
	*	\param _fMaxHP		: the max HP of npc air craft
	*	\param _fCurrHP		: current HP of NPC air craft
	*/
	virtual void gala_showBlood(const osVec2D& _craftPos,float _fCraftScale,float _fMaxHP,float _fCurrHP);

private: //some variable to render scene

	//! render manager to render the space hole
	static CGridRect				m_gridRect;


	//! the container of blood
	static std::vector<CBlood>		m_vecBlood;

	//! the index of current active blood process
	int						m_iCurrBlood;

	//! display the blood process
	void DisplayBlood(void);

	//! draw corona around hero?
	BOOL					m_bCorona;

	/*! \class CProxyGrid
	*	\brief : this class will hole the CGridRect(member m_gridRect)
	*			in osc_TGManager::render_scene function, and will call 
	*			the CGridRect::SetReadyToRender in destructor
	*/
	class CProxyGrid{

		//! reference class
		CGridRect&	m_refGrid;

		//! private defualt constrcutor to prevent
		//! default construct
		CProxyGrid(void);
	public:
		CProxyGrid(CGridRect& _grid):m_refGrid(_grid){}

		//! destructor to call some function
		~CProxyGrid(void)
		{
			if(m_refGrid.IsFirstRender()){
				m_refGrid.SetReadyToRender();
			}
		}
	};

# endif //GALAXY_VERSION
# if __EFFECT_EDITOR__
	//!���osaģ�Ͷ��������ٶ�
	virtual void 			set_OSAPlaySpeed(float _speed) {gOSAPLAYERSpeed = _speed;};

# endif 
//--------------------------------------------------------------------------------
//@}
//--------------------------------------------------------------------------------

};





//! ����inline����ʵ���ļ���
# include "fdFieldMgr.inl"

//! �����������ڵ�Ŀ¼
# define  BKSOUND_DIR    "sound\\"



# endif // # define __FDFIELDMGR_INCLUDE__