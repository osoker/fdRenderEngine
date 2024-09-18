//===================================================================
/**\file  
 *  Filename:   FindPathMap.h
 *  Desc:       �ڵ�ͼ��Ѱ·
 *		        ������ֹ��,������һ��.
 *		        ��������CFindPathMap::FindPathMap()
 *      
 *  His:      xlq create @  2007-5-16   16:17
 */
//===================================================================

#ifndef _def_FindPathMap
#define _def_FindPathMap

#define MAX_MAP_POINT_CNT	1280*1280		//��ͼ����
#define MAPBAR		0						//��ͼ�ϵ��ϰ�����
#define MAPBOO		1						//��ͼ�ϵĿյ���
#define MAXCLINECOUNT		1280*5			//�������е�����ܳ�
#define BOXPOINT			1500			//ÿһ���ľ�ϸ�ֱ��(1500mm)
#define BOXBORDER			15       //��Ե���գ���������̤�ڱ߽��ϣ����ÿ���

#define MAX_STEPLEN			BOXPOINT		//������mm
#define FINDPATH_BOXCOUNT	8				//ÿ���������ĸ�����


#define LIMIT_AREA			24	//! ������(����)


#define MAKEFOURCC32(ch0, ch1, ch2, ch3)                \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define PERMAP_SIZE 128


//! ����ĳһ��λ�õ��Ƿ�����ƶ�
#define MAP_CAN_MOVE  0x1
#define MAP_NOT_MOVE  0x0

//! ��ͼÿһ����ײ����ϸ�ֵ�size.
#define	_MAP_FINESIZE		1500


struct TPoint {
	int	x;
	int	y;
};

struct tagMAPFILEHEADER;

#define PATH_TOLERANCE		4

//! һ��·������Ķ���
# define MAX_WAYLENGTH  35.0f



//! River added @ 2008-11-11:�ѳ����·���������ָ�ɶ�һЩ��·���㡣
void divide_wayPoint( osVec2D& _src,osVec2D* _arrayWayPoint,int& _num,int _maxWayPoint,float _currSpeed );

//! �������õ�����ײ��Ϣ���ļ�ͷ
typedef struct tagMAPFILEHEADER
{
	char	szDesc[20];
	short	shVer;
	short	shWidth;
	short	shHeight;
	char	cEventFileIdx;
	char	cFogColor;
}MAPFILEHEADER, *LPMAPFILEHEADER;


class CFindPathMap
{
public:
	CFindPathMap();
	~CFindPathMap();

	/**brief
	*Ѱ·�㷨:  (ֻѰ�ҵ�Ŀ�ĵ��·���ϵ�һ������ֱ�ߵ��������)
	*\param	_pMapbuf	��ͼ��ײ��Ϣ,�ֽ����� 0��ʾ��λ 1��ʾ�ϰ�
	*\param	_nMapW		��ͼ���
	*\param	_nMapH		��ͼ�߶�
	*\param	_nX1		Դ����(��ȷ��mm)
	*\param	_nY1		
	*\param	_nX2		Ŀ������(��ȷ��mm)
	*\param	_nY2		
	*\param	_nX			���ص�����(��ȷ��mm)
	*\param	_nY			
	*\return	bool		�Ƿ���Ե���(���Ե���,���ص��������Ч)
	*/
	//bool FindPathMap(char * _pMapBuf, int _nMapW, int _nMapH,
	//	int _nX1, int _nY1, int _nX2, int _nY2, 
	//	int &_nX, int &_nY);
	bool FindPathMap( int _nX1, int _nY1, int _nX2, int _nY2, 
		int &_nX, int &_nY);

	//�����ڼ���8�������߿���֮�µ���һ����ȷ���꣨�ɲ໬��    
	//���롢�����FindPathMap��ͬ�� Ҫ��_nX2,_nY2������_nX1,_nY1ֻ��1��	
	//bool GetNextPoint(char * _pMapBuf, int _nMapW, int _nMapH,
	//	int _nX1, int _nY1, int _nX2, int _nY2, 
	//	int &_nX, int &_nY);
	bool GetNextPoint( int _nX1, int _nY1, int _nX2, int _nY2, 
		int &_nX, int &_nY);
	osVec2D get_mapSize();
	
public:

	//��ͼbuf
	char* m_pMapBuf;

	//��ͼ��С
	int m_MapW,m_MapH;
	//ÿ���Ŀ�� ��mmΪ��λ
	int m_dlen;
	//��ײ�洢��ʱ��
	DWORD m_timeData;

	//! �����ļ�ͷ
	tagMAPFILEHEADER*  m_sHeader;

	//---------------����ֵ,���·��ع����������ⲿʹ��-------------------
	int			m_pathcount;//pathlist�ĵ�ĸ���
	TPoint		m_pathlist[MAXCLINECOUNT];
	TPoint		m_NextPoint;//������һ�׶ξ�ȷ���꣬��mmΪ��λ
	TPoint		m_FirstBox;//��һ�׶�Ŀ��㣬��boxΪ��λ
protected:
	//----------������ÿ��Ѱ·���õ���ʱ����
	//�˴�ԭʼ����(��mmΪ��λ)
	int			m_x1,m_y1,m_x2,m_y2;
	//�˴�Ѱ·�Ը�������Ϊ��λ�Ĵ�����
	int			m_bx1,m_by1,m_bx2,m_by2;
	//���㷨Ҫ�õ��ĵ�ͼ����
	unsigned char m_maptmp[MAX_MAP_POINT_CNT];
	int			m_Cline1Count;
	TPoint		m_Cline1[MAXCLINECOUNT];
	int			m_Cline2Count;
	TPoint		m_Cline2[MAXCLINECOUNT];
	int			m_LineCount;//ֱ���ϵĵ������
	TPoint		m_LinePoint[MAXCLINECOUNT];
protected:
	//----------����Ϊ�ڲ�ʹ�õ�һЩ���ߺ���--------------
	//��ȡ������·��
	bool GetPathList();
	//��pathlist�л�ȡ��һ����ֱ���ϵ�����
	void GetFirstPoint();
	//����x1,y1��x2,y2�ľ�ȷ�����׵�����tx,ty������Ϊ_dlen (mm) ,����x1,y1,x2,y2��ָmm����
	bool GetStepPoint(int _x1,int _y1,int _x2,int _y2,int &_tx,int &_ty);
	//��ô�x1,y1��x2,y2ֱ���ϵĸ�����
	int GetLine(TPoint _arrLine[], int _nX1, int _nY1, int _nX2, int _nY2);
	//���ĳֱ�����Ƿ����ϰ���
	bool ChkBarLine(char* _pMapBuf, int _nMapH, int _nMapW, int _nX1, int _nY1, int _nX2, int _nY2);
	//��ֱ�ߵķ�����Ƶ�8������
	int GetFwordNear(int x1,int y1,int x2,int y2);
	//���Զ���·�����У������ݴ�m_FirstBox,һ�μ��㣬��ε���GetStepPointEx�������������m_NextPoint��
	//����x1,y1��x2,y2�ľ�ȷ�����׵�����tx,ty������Ϊ_dlen (mm) 
	/**\brief
	* ���������ݵ�ǰ�Ľ׶�Ŀ��m_FirstBox�������һ���ľ�������m_NextPoint (����mm)	
	* \param   x1,y1,ָ��ǰ��mm����
	* \param   m_FirstBox������ں�����ֱ��ʹ�á�
	* \param   
	* \return  m_NextPoint ����ֵ�����ڸ�������
	*/
	bool GetStepPointEx(int _x1,int _y1);

public:
	//! �����������ͼ
	bool LoadMapInfo( const char* _pMapFile );	
	
	//! �洢��������ͼ
	bool saveMapInfo( const char* _filename );


	//! �ж�һ���Ƿ���ͨ��
	bool CanMove( const osVec3D& _pPos );
	bool CanMove( const osVec2D& _pPos );
	bool CanMove( int _nX,int _nY );
	bool CanMoveGrid( int _nX, int _nY ); //��������
	bool CHKBAR( int x, int y );

	//! ���һ��ֱ�����Ƿ����ϰ���
	bool CheckLine(  int _nBeginX, int _nBeginY , int _nTargetX , int _nTargetY,bool _accu = false );
	bool CheckLine( osVec3D *_pStartPos, osVec3D *_pEndPos );

	//! 
	void CalcNextPoint( int _wSx, int _wSy, int _wDx, int _wDy, int _wSpace, int &_wX, int &_wY );

	/**\brief
	* ����ָ������Ѱ��ֱ����Զ������ by sisun @ 2007-06-25
	* \param    _sStartPos: ��ʼ����
	* \param	_sDir:		�ƶ�����
	*/
	void FindLinePath( osVec2D& _sStartPos, osVec2D _sDir );

	/** \brief
	 * ������ʼ����յ�ķ��򣬼������յ�����Ŀ�ͨ��������㡣 by river@ 2008-11-10:
	 *
	 * ����ʹ����CalcNextPoint������ͬ������ֵ����ϸ��(mm��λ)���ӵ����ꡣ
	 */
	bool FindNearestMovePt( int _wSx, int _wSy, int _wDx, int _wDy,int &_wX, int &_wY );


	/** \brief
	 *
	 * \param osVec2D& _src Ѱ·��ʼ���λ�á�
	 * \param osVec2D& _dest Ѱ·�������λ�á�
	 * \param osVec2D* _arrayWayPoint Ѱ·���صĹؼ�·��
	 * \param int& _num          Ѱ·���صĹؼ�·��λ��.
	 * \param int _maxWayPoint   ���������,�����Է��ص�·��λ��
	 * \param _currSpeed		��ǰ������ٶ�
	 */
	bool ComputeWayPoint(osVec2D& _src,osVec2D& _dest,osVec2D* _arrayWayPoint,int& _num,int _maxWayPoint,float _currSpeed);

	/*! \brief 

		�Ż�NoObstacle���㷨������
		1: �����ʼ�����ڸ��ӣ�������������ڵĸ��ӡ�
		2: �ӿ�ʼ��ĸ�������ÿ���ཻһ�����ӣ��õ��˸����Ƿ�����ײ��Ϣ��
		3: �Ż��������߶���X��ĽǶȣ�ÿ���ཻ��ֻ������ӵ��������ཻ������ֻ��Ҫ
		����ǰ���߶��Ƿ��������ı��߶��ཻ��
		4: ������ʼ������ӵ��ı��߶��ཻ���õ�������һ������ĸ���....ѭ����ǰ����
		����ֱ�ӵ������յĸ��ӡ�

		����м�û�и�������ײ��Ϣ���򷵻�true,���򷵻�false.

	*/
	BOOL NoObstacle(float x0,float y0,float x1, float y1);

	//! �õ��˴ε�ͼ�洢��ײ��ʱ��ֵ
	DWORD GetMapCollTime(void)const {return m_timeData;}
	

	//! get the collision info
	char* GetCollisionInfo(void){return m_pMapBuf;}

};

/**\brief
* ����������������ָ�������һ���Ϸ���
* \param    
* \return   
*/
void CalcPoint(int _nBeginX,int _nBeginY,int _nTargetX,int _nTargetY,int _nSpace,int &_nX,int &_nY);


#endif

