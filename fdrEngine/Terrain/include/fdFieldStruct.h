//////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: fdFieldStruct.h
 *
 *  His:      River created @ 2004-5-21
 *
 *  Des:      ��ͼ��������صĽṹ��
 *   
 * ����ּ���֮�ǣ����������������ǽ֮��Ҳ���� 
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////////////////
# pragma once

//! ��������Ľӿ�ͷ�ļ�
# include "../../interface/osInterface.h"

# include "../../mfpipe/include/middlePipe.h"

# include "../../effect/include/fdBillboard.h"

# include "../../effect/include/fdTrack.h"
# include "../../effect/include/dlight.h"
# include "../../effect/include/fdSky.h"
# include "../../effect/include/fdWater.h"
# include "../../effect/include/fdParticle.h"


# include "../../tools/include/osConsole.h"

# include "../../mfpipe/include/osMesh.h"

# include "../../mfpipe/include/deviceInit.h"

//! �϶����ݽṹ��ص�ͷ�ļ���
# include "../../backPipe/include/osCamera.h"

# include "fdTGQuadtree.h"


//! ��Ļ��ʾ��άͼƬ��������ݽṹ��
typedef std::vector<os_screenPicDisp>    VEC_screenPicDis;


//! ÿ��64*64�ĵ�ͼ������ʹ�õĶ�̬������Ŀ,Ӧ�����Ƶ�64��.
# define MAX_SMOBJPERMAP         64

//! �Ƿ�ʹ��WaterAlpha��ˮ���밶�ߵĽӷ촦��͸��
# define WATER_ALPHA             0


//! �����µ�ͼ����õ�ʱ�䡣
# define  FIRST_TIMETICK         400
# define  MAX_TIMETICK           100

//! ������֡�Ժ��ٴε�����Ʒ��
# define  OBJLOAD_PERFRAME       800

//! ����������������õ�Ŀ¼.
# define  BILLBOARD_DIR   "efftexture\\"
//! skinMesh����������õ�Ŀ¼.
# define  SMAINOBJ_DIR    "data\\smani\\"
//! keyAniMesh������Ʒ���ڵ�Ŀ¼,�������õ�������smAniĿ¼��,�Թ�������
# define  KEYANIOBJ_DIR   "keyani\\"
//! water�ļ����ڵ�Ŀ¼.
# define  WATERINI_DIR    "water\\"
//! �����ļ����ڵ�Ŀ¼��
# define  PARTICLE_DIR    "particle\\"
//! ������Ч�ļ����ڵ�Ŀ¼
# define  SCEEFFECT_DIR   "speff\\"


///////////////////////////////////////////////////////////////////////////////////
//
// Ҫ�����޷�ĵ�ͼ���룬���ܻ�ʹ������һ���߳��������Ҫ�����ĵ�ͼ��
// 
///////////////////////////////////////////////////////////////////////////////////

//! TileIdx��stl Vector.
typedef std::vector<os_tileIdx>  VEC_tileIdx;

# define QUADTREE_TEST  1


//! ���������õ���tgManager��
class    osc_TGManager;
class    osc_tgLoadReleaseMgr;

/** \brief
 *   ��ͼ�ļ�ͷs
 */
struct os_TGFileHeader
{
	//!�ļ���ʶ
	char	   m_cID[2];
	//!�ļ�����
	WORD	   m_wFileType;
	//!�ļ��汾
	DWORD	   m_dwVersion;
	//!��ͼ��
	CHAR	   m_strMapName[128];

	//! ���ֵ��Ϊ�߼��ϵ���������64*64�ĵ�ͼ��ƴ��Ϊ����ĵ�ͼ��
	LONG       m_vMapPos[2];

	//!�������������ڵ�ͼ��
	DWORD      m_dwObjectNum;

	DWORD      m_dwTexNum;
	//��ͼ�ڶ���һ��ʹ���˶�������
	DWORD      m_dwBackTexNum;

};




//! ��Ⱦ�������ݵ�Stl vector.
typedef std::vector<os_sceVerTile> VEC_verTile;



//! ��Ұ��������Ʒ����Ŀ. 
# define MAX_INVIEWOBJ      512   

//! һ֡�п�����Ӻ�ɾ��������object��Ŀ,
//! ATTENTION TO FIX:ʹ�ø��ٵĶ��У������Ż��Ĵ���			
# define MAX_ADDDELOBJ      64



//! ��ǰ������Ұ��ÿһ����Ʒ��״̬.
enum  ose_inviewObjState
{
	//! ��ǰԪ�ز�����Ұ����ʾ��
	OSE_IVO_NOTINVIEW = 0,

	//! ��ǰԪ������һ֡�ж�Ӧ����Ʒ����,��ǰ֡����Ʒ����������Ұ��.
	OSE_IVO_NORMALDIS,

	//! ��ǰԪ�������ڱ�Ե״̬,���ڵ�����Ұ.
	OSE_IVO_FADEIN,

	//! ��ǰԪ�������ڱ�Ե״̬,���ڵ�����Ұ.
	OSE_IVO_FADEOUT,

	//! ��ǰԪ���ɵ�����Ұ��Ϊ������Ұ.������û�����,��ʼ����.
	OSE_IVO_FOUT_TO_FIN,

	//! ��ǰԪ���ڵ�����Ұ�Ĺ����У�û����ɵ���״̬ʱ����ʼ������Ұ��
	OSE_IVO_FIN_TO_FOUT,

	//! ��ǰԪ��Ϊ��͸����ʾ��Ϊ�������������֮�����Ʒ��ʾ��
	OSE_IVO_HALFALPHA,

};

typedef std::vector<ose_inviewObjState>  VEC_inviewObjState;

//! ��Ļ�������ʾ64��������������֮��İ�͸����Ʒ��
# define MAX_HALFALPHAOBJ   64


/** \brief
 *  ���ι������еĿɼ���Ʒ������.
 *
 *  �������еĿɼ���Ʒ,ʹ���������,��Ⱦ��Ʒ,��ʹ�˳���Ұ����Ʒ
 *  ��Alpha��������,ʹ������Ұ����Ʒ�е������.
 *  
 *  River @ 2004-8-10:
 *  ������ÿһ��TGʹ��һ��ObjMgr,ÿһ֡����ʱ��ֻ��Ҫ�Ա����õ�ǰ������
 *  �ڲ�ÿһ����Ʒ��״̬�Ϳ����ˡ�
 *  ÿһ��viewObjMgr�ڴ洢����һ֡��Ʒ�Ŀ���״̬����ÿһ��֡�Ĵ����У�����
 *  ��ǰ֡����Ʒ��״̬����һ֡��Ʒ�Ŀ���״̬���õ���������ÿһ����Ʒ�Ŀ���״̬��
 *  
 */
class osc_inViewObjMgr
{
	friend class             osc_newTileGround;

	// TEST CODE:
	friend class             osc_TGManager;
private:

	//! Ŀǰ�ڴ�����У������õĽ����Ŀ��
	int               m_iMaxNodeNum;

	/** \brief
	 *  ��ǰ��״̬���������ÿһ��objIndex��״̬.
	 * 
	 */
	VEC_inviewObjState  m_vecIvOs;
	
	//! ������ʾ����Ʒ���б�
	VEC_int           m_vecNormalDis;
	//! ��ǰ��������TG��ÿһ����Ʒ��һ֡�Ŀ���״̬��
	VEC_bool           m_vecLfVs;

	//! ������ʾ����Ʒ����Ŀ��
	int                m_iNDObjNum;



	//! �ɼ���Ʒ��������Ӧ�ĵ��������.
	VEC_fadeMgr        m_vecFadeInMgr;
	//! Ҫ���뵽��Ұ�ڵ���Ʒ�Ķ���.-1��ʾû��ʹ��.
	VEC_int            m_vecFadeInObj;


	//! ����Ұ����ʧʱ,��Ӧ����Ʒ����������.
	VEC_fadeMgr        m_vecFadeOutMgr;
	//! Ҫ����Ұ��ɾ������Ʒ�Ķ���.-1��ʾû��ʹ��.
	VEC_int            m_vecFadeOutObj;


	//@{
	//! ��͸����ʾ����Ʒ�б�,������ʾ��ס�������Ʒ��
	int                m_vecHalfAlphaObj[MAX_HALFALPHAOBJ];
	//! �������Ѿ����ڵĵ�ס����İ�͸����Ʒ���б�.
	int                m_vecAccuHAObj[MAX_HALFALPHAOBJ];
	/** \brief
	 *  �Ӳ�͸������Ʒ����͸����Ʒ.
	 *
	 *  ���ڴӰ�͸������͸������Ʒ,���ǰ���Щ��Ʒ�뵽
	 *  fadeIn��Ʒ�Ĺ�������д���.
	 */
	VEC_fadeMgr        m_vecHalfFadeMgr;
	//@} 



private:

	//! ���һ��fadeInObject.
	bool               add_fadeInObj( int _idx );

	//! ���һ��FadeOutObject
	bool               add_fadeOutObj( int _idx );

	//! ÿһ֡ȥ��ס������ߵ�͸����Ʒ�Ĵ���.
	void               process_halfFadeObj( void );

	//! ��������ʾ����Ʒ������ɾ��һ����Ʒ.
	void               delete_normalDisObj( int _idx );

public:
	osc_inViewObjMgr();
	~osc_inViewObjMgr();

	//! ��ʼ����ǰ��InviewObjMgr.
	void               init_ivoMgr( int _objNum );

	/** \brief
	 *  ��ʼ��һ֡�Ĵ���.
	 *
	 *  �����ڵõ��ر���Ʒ�Ŀɼ���֮ǰ�����������.
	 *  
	 */
	void              frame_setObjMgr( void );


	/** \brief
	 *  ÿһ֡�еõ���ǰTG����Ʒ�б�󣬶�inviewObjMgr����Ʒ�Ĵ���
	 */
	void              frame_processIvo( VEC_BYTE& _ivoList );

	//! ��һ��������ʾ����Ʒ��Ϊ��͸������Ʒ��
	void              make_halfFade( int _objIdx );


};


//@{

//! �ļ��汾��Ϣ��OSS��OS SOUND��
#define OSS_VERSION 100
static const unsigned long oss_version = (('O') | ('S'<<8) | ('S'<<16) | (OSS_VERSION<<24));

/** \brief
* OSSFile_hdr �ļ�ͷ��Ϣ
*
* ����������Ŀ�ĸ�����*/
struct OSFile_hdr
{
	int		SoundCnt;	//!�����ļ��ĸ���.
};
// snd �ļ�ͷ
struct SndHeader
{
	char	mFileFlag[4];	
	DWORD	mVersion;
	


	DWORD	mdwOfsSoundItem;	//!< OSSoundItem ����
	DWORD	mdwNrSoundItem;		//!< 
};

/** \brief
* ÿ��������Ŀ����Ϣ
*
* ������Դid,�����ļ���,����������Ϣ*/
struct OSSoundItem
{
	//!��Դid
    int		sound_id;	
	//!��Դ��
	//char    m_idString[40];
	//! λ��
	osVec3D m_pos;
    //!�����ļ���
    char	m_SoundName[128];
	//!�������,Ĭ�ϳ�ʼ2
	float   m_flMinDistance;	
	//!������,Ĭ�ϳ�ʼ100
	float	m_flMaxDistance;	
	//!��Դ������0��-10000;Ĭ��10000
	//long	m_lVolume;		
	//! ���Ÿ�������,������,1/10~9/10,ѭ������,  ��Ӧ0-10
	DWORD     m_playType;
	// ��Ч�Ĳ���ʱ��
	float   m_fSoundPlayTime;

	OSSoundItem( void )
	{
		sound_id = -1;
		m_pos = osVec3D( 0.0f, 0.0f, 0.0f );
		m_SoundName[0] = 0;
		m_flMinDistance= 2.0f;
		m_flMaxDistance= 100.0f;
		m_playType  = 0;
	}
	OSSoundItem& operator=( const OSSoundItem& _sounItem )
	{
		sound_id = _sounItem.sound_id;
		m_pos = _sounItem.m_pos;
		strcpy( m_SoundName, _sounItem.m_SoundName );
		m_flMinDistance = _sounItem.m_flMinDistance;
		m_flMaxDistance = _sounItem.m_flMaxDistance;
		m_playType   = _sounItem.m_playType;

		return *this;
	}
};

//! ���ڲ��ŵ�������Ϣ
struct OSSoundPlayingItem
{
	// River mod 2008-3-21:�����ڴ�ķ�����ͷ�
	BOOL   m_bPlayingSound;

	//! һ��������Ϣ
	OSSoundItem * pItem;
	//! ��̬��õ�ѭ������������id,Ϊ�˹ر��������ڲ���һ�ε���������ֵΪ-1
	int idCloseLoopSound;
	//! 
	float mPlayTimeMs;
	//! 
	float mTimeLength;

	OSSoundPlayingItem()
	{
		m_bPlayingSound = FALSE;
	}

	// ����˽ṹ
	void reset( void )
	{
		m_bPlayingSound = FALSE;
		pItem = NULL;
		idCloseLoopSound = -1;
		mPlayTimeMs = 0.0f;
		mTimeLength = 0.0f;
	}

	bool operator == (const OSSoundItem* rhs)
	{
		return (pItem==rhs);
	}
	bool operator == (const OSSoundPlayingItem& rhs)
	{
		return (pItem == rhs.pItem);
	}
};

//@}

//@{
// Windy mod @ 2005-8-12 9:22:25
//Desc: ���ڱ���ÿ������Ķ���߶ȵ�������Ϣ
typedef struct _AddHeight{
	WORD x;
	WORD y;
	float Modify;
public:
	_AddHeight(WORD _x = 0,WORD _y = 0,float _Mod = 0):x(_x),y(_y),Modify(_Mod){
	}

	bool operator == (const _AddHeight & _add){
		return (x == _add.x) && (y == _add.y);
	}
}AddHeight,*PAddHeight;
typedef std::vector<AddHeight> VEC_AddHeight;

//@}


//! ����ÿһ�����ӵ�����ֵ��
extern const int MASK_TILEDISPLAY;
extern const int MASK_WATERTILE;


//! ʹ��osa�ļ���Ϊ�����е�ˮ���ļ�.
# define  USE_OSAWATER      1


//! �����һ��TG�ֳ��Ŀ�Ļ���ÿһ������֡�
enum osn_inTgPos 
{
	OSN_TGPOS_ERROR = 0,
	OSN_TGPOS_TOPLEFT,
	OSN_TGPOS_TOPRIGHT,
	OSN_TGPOS_BOTLEFT,
	OSN_TGPOS_BOTRIGHT
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  �������õ�����ɫ�ַ����͡�
// 
//! ÿһ����ɫ�����ִ������Խ��������ִ���Ŀ��
// syq mod @ 05.2.26       24��Ϊ48   (�ɵ�24���ִ���ʱ�ڴ�ռ䲻�㣬�ᵼ�´���)
// River mod @ 06-7-22:    48��Ϊ72, �ִ�Խ��Խ��
# define  MAX_STRDIS_PERCI    512
// WORK START: �Ż�������

//! ����ɫ�����ִ�����Ϊ������ʾ�ִ��Ĺ�����
class osc_szCI
{
	static char          m_cSelChar;
	static char          m_cSelCharS;
	static char          m_cSelCharE;

private:
	static DWORD     get_strColor( const char* _sz,int _idx,int& _cnum );

	//!
	static int       get_disStr( os_stringDisp* _resSD,int& _strIdx,
		                int& _xOffset,int& _yOffset,os_colorInterpretStr* _srcCI,int _cidx ,int _TextSize );

public:

	//! ����ڸ�������Ļ�������������ڣ�������ʾ�ִ�����������򷵻�false.
	static int       interpret_strDisp( os_stringDisp* _resSD,
		                     os_colorInterpretStr* _srcCI,int& _iresNum ,int _fontType = 0);
};

/** \brief
 *  ��͸������������㷨���õ���camera Ptr
 *  
 *  ������ʹ�õ���ȫ��Ψһ�����.
 */
extern osc_camera*           g_ptrCamera;








