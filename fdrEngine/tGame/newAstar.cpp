//---------------------------------------------------------------------
  /** \file
   * Filename:newAstar.cpp
   *
   * Desc: ����һ��Astar��
   *
   * His : honghaier Created @  2003 12/18 9:19
   *		tyx Mod @ 2004-3-20
   * tyx mod @ 2004-3-26 ��εı䶯�Ƚϴ�,��Ҫ����ȫ����д,��������ǰ��
   *			   �������:
   *          1. ��дһ������������ ͳһ��������е�·������,��ȥ�˳����е�һЩ����Ҫ�����ƶ�.
   *          2. �����ȥPopstack();���� ���㷨�޸ĺ������ȫû�б�Ҫ
   *          3. �����ȥ//! ���������# define MAX_PROCESSTIME 1024*4 ��������޸ĺ� 
   *				�Ѿ�û�ж�������--�����µ�open����������,��������̫���ѭ������,��ȥ
   ��������еļ��������Ի����ٵ���߳�����ٶ�.
   *          4. �����ȥ//! ����A*�����Ѱ·������# define MAX_PATHASTAR  48 ��Ϊ���µ����ݹ����¿����޸�
   *				open/close����ܵĴ�С������Ѱ·�������.
   *          5. �޸�OBLIQUE_VALUE ��ֵΪ16(ԭ�ȵ�ֵΪ14),��������ʹ�õ���·�����ӽ����·��.
   *          6. �Գ����е������ɵ�·�� ��һ���̶��ı���m_sTmp ����ÿ��������·��ʱnewһ����
   *			 ����,��߳����ٶ� �����Լ����ڴ���Ƭ
   *          7. �޸�Ѱ·�����ս�� ��Ŀ�겻�ɵ���ʱ,��������������·��.
   */
//---------------------------------------------------------------------
# include "stdafx.h"

# include "tengine.h"
# include "dxutil.h"
# include "newAstar.h"

//! ���ݱ������ �����޸����ֵ���޶�A�㷨�ļ���·����Ŀ,�������,�ٶ�
# define INIT_VECSIZE   1024*2
# define PERMAP_SIZE        128

//! �����ƶ�������ֵ������Ŀ��
# define DIRECT_VALUE       10

//! б���ƶ�������ֵ������Ŀ��
# define OBLIQUE_VALUE      16

CVList  fd_AstarBase::m_vecList;

//! ��ʼ��static������
fd_AstarBase* fd_AstarBase::m_instance = NULL;

/** \brief
* A*�㷨��Ĺ��캯��
*/
fd_AstarBase::fd_AstarBase()
{
	m_vecList.ReSize(INIT_VECSIZE);//��ʼ�����ݱ�ĳߴ�
	m_pPath = NULL;
}

/** \brief
* A*�㷨�����������
*/
fd_AstarBase::~fd_AstarBase()
{

}


/** \brief
* ����һ�����ʵ��ָ��.
*/
fd_AstarBase* fd_AstarBase::Instance()
{
	if( !m_instance )
		m_instance = new fd_AstarBase;		
	return m_instance;

}
/** \brief
* ɾ��һ��ʵ������ָ��.
*/
void fd_AstarBase::DInstance()
{
	if( m_instance )
		delete m_instance;
}


/** \brief
*  ����A*�㷨Ϊ��ʼ״̬��
*/
void fd_AstarBase::reset_aStar( void )
{
	//�����ݱ������
	m_vecList.ReSet();
}

/** \brief
*  ����������
*/
int fd_AstarBase::estimate_value( int _x,int _y )
{
	return abs(m_iEX-_x)+abs(m_iEY-_y);
}

/** \brief
*  ���뵱ǰ��㵽open��
*/
Link *fd_AstarBase::add_toOpen(const Link _link)
{
	return m_vecList.AddToOpen(&_link);
}


/** \brief
*  ��Open���еõ���ѽ��,���ҰѸõ����close���С�
*
*  \param Link ������ѽ����open���еĵ�ַָ�롣
*/
Link *fd_AstarBase::get_bestElement( void )
{
	return m_vecList.GetBestLink();
}

/** \brief
*  �������Խ���Ƿ������OPEN��, �������Բ���open��Ľڵ����open��(������)
*
*  \param x�������x����
*  \param y�������y����(3d����ϵ��ʵ��Ϊz)
*  \param f������Ĺ���ֵ
*  \param _parent��������������һ����(��ָ��)
*  \return Bool �����Ƿ���Բ���open��(�ڳ�����û��ʵ����;)
*/
bool fd_AstarBase::try_tile(int _x,int _y,int _f, Link *_parent)
{
	if( (_x<m_iMinXNum )||(_x>=m_iMaxXNum) )//������ǰ��ͼ�ķ�Χ
		return false;
	if( (_y<m_iMinYNum )||(_y>=m_iMaxYNum) )
		return false;

	// ���tile����ͨ�������أ�
	if( !is_tilePass( _x,_y ) )
		return false;

	int t_iTile = this->tile_xy( _x,_y );//(_x,_y)����������(��������)

	Link *t_pOld;
	//�����open����
	if( (t_pOld=m_vecList.IsInOpen( t_iTile )) != NULL)
	{
		if(t_pOld->m_iValue > _f)//���¹���ֵ
		{
			t_pOld->m_iValue = _f;
			t_pOld->parent = _parent;
		}
	}
	//�����close����
	else if( (t_pOld=m_vecList.IsInClose(t_iTile)) != NULL )
	{
		if(t_pOld->m_iValue > _f)//���¹���ֵ ���ڵ�
		{
			t_pOld->m_iValue = _f;
			t_pOld->parent = _parent;
		}

	}
	else//����open���� Ҳ����close���� �����Ԫ�ص�open����
	{
		m_sTmp.m_iValue = _f;
		m_sTmp.m_iCurIdx = t_iTile;
		m_sTmp.parent = _parent;

		if( !add_toOpen( m_sTmp ) )//���뵽open����
			return false;
	}
	return true;
}

/** \brief
*  ���ڵõ���ǰ��ͼ�����Ƿ����ͨ���ĺ�����
*
*  ��������Ƕ�̬��ͼ�ĺ��ġ�����ά����һ������ĵ�ͼ��
*��
*  ��ͼ��0Ϊͨ����1Ϊ��ͨ��������ʹ�÷ǲ������ء�
*/
bool fd_AstarBase::is_tilePass( int _x,int _y )
{
	int    t_x,t_y;

	//��ǰ����ֵ���ڵĵ�ͼ��
	int  t_mapIdx,t_mapIdy;
	t_mapIdx = _x/PERMAP_SIZE;
	t_mapIdy = _y/PERMAP_SIZE;

	const os_TileCollisionInfo*  t_ptrTI = NULL;//��ͼ��ײ��Ϣ

	for( int t_i=0;t_i<m_iMapNum;t_i++ )
	{
		if( (m_arrTGIdx[t_i].nX == t_mapIdx)&&
			(m_arrTGIdx[t_i].nY == t_mapIdy) )
		{
			t_ptrTI = m_colMapInfo[t_i];//ȡ�õ�ǰ��ͼ��ײ��Ϣ
			break;
		}
	}

	// �����ǰ��ͼû�����ڴ���,��ǰ��·�㲻��ͨ��.
	if( t_ptrTI == NULL )
		return false;

	//·���������ڵĵ�ͼ�ϵ�����
	t_x = _x%PERMAP_SIZE;
	t_y = _y%PERMAP_SIZE;

	return !t_ptrTI[t_y*PERMAP_SIZE+t_x].m_bHinder;//�����ܷ�ͨ����ֵ
}



/** \brief
*  ����ֵ��ĵ�ͼ���ݡ�
*/
void fd_AstarBase::process_mapData( as_colInfo* _colInfo )
{
	int   t_x,t_y;
	//
	// ����ǰ�ĵ�ͼ��Ŀ,���ڵõ�һ������Ĵ��ͼ��
	m_iMapNum = _colInfo->m_iMapNum;//tyx:��ǰ��ͼ����Ŀ
	memcpy( m_arrTGIdx,_colInfo->m_arrTGIdx,sizeof( BaseMesh )*m_iMapNum );
	for( int t_i=0;t_i<m_iMapNum;t_i++ )
		this->m_colMapInfo[t_i] = _colInfo->m_arrColInfo[t_i];
	//
	// �õ���ǰ��ͼ�����ֵ����Сֵ��
	this->m_iMaxXNum = -1,this->m_iMaxYNum = -1;
	this->m_iMinXNum = 10000,this->m_iMinYNum = 10000;
	for( int t_i=0;t_i<_colInfo->m_iMapNum;t_i++ )
	{
		t_x = _colInfo->m_arrTGIdx[t_i].nX*PERMAP_SIZE;
		if( t_x < this->m_iMinXNum )
			m_iMinXNum = t_x;
		t_x = (_colInfo->m_arrTGIdx[t_i].nX+1)*PERMAP_SIZE;
		if( t_x > this->m_iMaxXNum )
			m_iMaxXNum = t_x;

		t_y = _colInfo->m_arrTGIdx[t_i].nY*PERMAP_SIZE;
		if( t_y < this->m_iMinYNum )
			m_iMinYNum = t_y;
		t_y = (_colInfo->m_arrTGIdx[t_i].nY+1)*PERMAP_SIZE;
		if( t_y > this->m_iMaxYNum )
			m_iMaxYNum = t_y;
	}
	//
	// �õ���ǰ��ͼ�Ŀ�ȡ�
	this->m_iMapWidth = this->m_iMaxXNum-this->m_iMinXNum;
	this->m_iMapHeight = this->m_iMaxYNum-this->m_iMinYNum;

	return;
}


/** \brief
*  ͳ�ƣ�������������·���������·��ֵ��
*
*  \return int �������-1�����Ҳ���·����
*/
int fd_AstarBase::get_path( as_colInfo* _colInfo )
{
	PATH* t_pathPtr;//����Ҫ���ص�·����ָ��
	t_pathPtr = _colInfo->m_resultPtr;

	int t_pathIdx = 0;//·��ĸ���

	BaseMesh t_mesh;
	while (m_pPath) 
	{
		if(!m_pPath->parent) //���㵽��ʼ��(��ʼ��û�и���)
			break;
		t_mesh.nX = tile_x(m_pPath->m_iCurIdx);
		t_mesh.nY = tile_y(m_pPath->m_iCurIdx);
		t_pathPtr->push_front(t_mesh);

		t_pathIdx++;//·�������һ
		m_pPath = m_pPath->parent;
	}

	return t_pathIdx;//tyx:����·����
}



/** \brief
*  ���������������·�� ���õ������
*
*  \return int ���ش�Դ�㵽Ŀ�����Ҫ�೤��·���������-1,�����ҵ�·��
*/
int fd_AstarBase::find_path( int _sx, int _sy, int _dx, int _dy, as_colInfo* _colInfo )
{
	int        t_idx,t_x,t_y;

	this->m_iSX = _sx;
	this->m_iSY = _sy;
	this->m_iEX = _dx;
	this->m_iEY = _dy;

	m_pPath = NULL;

	// ��ʼ�������·����
	reset_aStar();//tyx:��ʼ��

	// ������ĵ�ͼ���ݡ�
	process_mapData( _colInfo );

	Link t_sLink;
	//ת��Ϊ���������--Ȼ���ٵõ�����
	t_sLink.m_iCurIdx = tile_xy(m_iSX, m_iSY);
	t_sLink.m_iValue = estimate_value(m_iSX, m_iSY);
	t_sLink.parent = NULL;
	add_toOpen(t_sLink);

	// ѭ��ʹ��open��
	int   t_iProcessNum = 0;
	Link	*t_pBextLink = NULL;
	while( 1 ) 
	{
		t_pBextLink = get_bestElement();//ȡ������ֵ��õĵ㲢����close����

		//���open�����
		if(!t_pBextLink)
		{
			//��close�����Ҹ�����ĵ�
			Link *t_pTemp = m_pPath = m_vecList.GetCloseHead()->next;
			int t_iDistance = estimate_value(tile_x(m_pPath->m_iCurIdx), tile_y(m_pPath->m_iCurIdx));
			while(t_pTemp->next)
			{
				t_x = tile_x(t_pTemp->next->m_iCurIdx);
				t_y = tile_y(t_pTemp->next->m_iCurIdx);
				if( estimate_value(t_x, t_y) < t_iDistance )
				{
					m_pPath = t_pTemp;
					t_iDistance = estimate_value(tile_x(m_pPath->m_iCurIdx), tile_y(m_pPath->m_iCurIdx));
				}
				t_pTemp = t_pTemp->next;
			}
			break;
		}
		//

		t_x = tile_x( t_pBextLink->m_iCurIdx );
		t_y = tile_y( t_pBextLink->m_iCurIdx );

		//!����Ѿ�����Ŀ�����˳�
 		if( (t_x == this->m_iEX)&&(t_y == this->m_iEY) )////tyx:�ﵽĿ��
		{
			m_pPath = t_pBextLink;
			break;
		}
		////���������ƶ�
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x, t_y+1);
		try_tile( t_x, t_y+1, t_idx, t_pBextLink );
		//// ���������ƶ�
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x, t_y-1);
		try_tile( t_x, t_y-1, t_idx, t_pBextLink );
		//// ���������ƶ�
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x-1, t_y);
		try_tile( t_x-1, t_y, t_idx, t_pBextLink );
		//// ���������ƶ�
		t_idx = t_pBextLink->m_iValue + DIRECT_VALUE +estimate_value(t_x+1, t_y);
		try_tile( t_x+1, t_y, t_idx, t_pBextLink );
		//// �����������ƶ�
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x+1,t_y+1); 
		try_tile( t_x+1,t_y+1,t_idx, t_pBextLink );
		//// �����������ƶ�
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x+1,t_y-1); 
		try_tile( t_x+1,t_y-1,t_idx, t_pBextLink );
		//// �����������ƶ�
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x-1,t_y-1); 
		try_tile( t_x-1,t_y-1,t_idx, t_pBextLink );
		//// �����������ƶ�
		t_idx = t_pBextLink->m_iValue + OBLIQUE_VALUE + estimate_value( t_x-1,t_y+1); 
		try_tile( t_x-1,t_y+1,t_idx, t_pBextLink );

	};//end while

	// ͳ�Ƶ�ǰ��·���ж��ٲ��������ء�
	return get_path( _colInfo );
}
