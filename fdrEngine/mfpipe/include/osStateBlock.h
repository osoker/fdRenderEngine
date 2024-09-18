//--------------------------------------------------------------------------------------------------------
/**  \file
 *   Filename: osStateBlock.h
 *
 *   Desc:     O.S.O.K �������Ⱦ�������
 *
 *   His:      River Created @ 2004-12-14
 *
 *  ���ڴ��˽�����ѹ���У����Լ������ѹ��Զ������˶����ѹ�������˶����
 *    ѹ��Զ���繫�ڶ����ѹ������
 */
//--------------------------------------------------------------------------------------------------------
# pragma    once

# include "../../interface/osInterface.h"
# include "../../interface/miskFunc.h"


//! �����������������е�block��Ŀ��
# define MAX_STATEBLOCK     36


/** \brief
 *  ���������ڲ�ʹ�õ���Ⱦ״̬��Ĺ�������
 *
 *  ��ΪStateBlockȫ����ȫ���Եģ������濪ʼ����������Ҫʹ�ã�
 *  ���Բ���Ҫ���ͷ�StateBlock�Ĺ����ӿڡ�
 *  
 *  ȫ��ֻ��Ҫһ��ʵ���Ϳ����ˡ�
 *
 */
class os_stateBlockMgr
{
private:
	LPDIRECT3DDEVICE9       m_pd3dDevice;


	static os_stateBlockMgr* m_ptrInstance;

	//! DX�ڲ���Ⱦ״̬��ָ����б�
	LPDIRECT3DSTATEBLOCK9*   m_vecStateBlock;
	VEC_bool                m_vecSBUsed;
	VEC_word                m_vecSBCreateTimes;

	os_stateBlockMgr();
	~os_stateBlockMgr();

	// TEST CODE:
	BOOL                    m_bRecordState;

public:

	/** �õ�һ��sg_timer��Instanceָ��.
	 */
	static os_stateBlockMgr*    Instance();
	
	/** ɾ��һ��sg_timer��Instanceָ��.
	 */
	static void                DInstance();


	//! ��ʼ����Ⱦ״̬���������
	void       init_sbMgr( LPDIRECT3DDEVICE9 _dev );

	//! �õ�һ����Ⱦ״̬�顣
	int        create_stateBlock( void );

	//! ��ʼ¼��״̬�顣
	void       start_stateBlock( void );

	//! ����¼��״̬�顣
	void       end_stateBlock( int _idx );

	/** \brief
	 *  ��ǰ��stateBlock ID �Ƿ���Ч.
	 *
	 *  ���device lost ,��ÿһ��id����Ч,��Ҫ���µĴ���ÿһ���ط���state block.
	 *  Ϊ��ʹÿһ��state Block�����ǵ�ǰ��block,ʹ���˴���������sb��id��ͬ������
	 *  ��,��Ϊsb��id,��ȻReset���������µ�����:
	 *  ��ĳһ��stateId validateʱ,��������ȷ��ֵ.���ײ��stateblock�Ǳ�ĵط�����
	 *  ��stateBlock,��������»����.
	 */
	bool       validate_stateBlockId( int _id );


	//! ����Ⱦ״̬��ʹ�õ��豸��
	void       apply_stateBlock( int _idx );

	//! �ͷ�һ����Ⱦ״̬��,Ҫ�ͷŵĵ�ͼ��id��Ϊ-1.
	void       release_stateBlock( int& _idx );

	//! ���豸lost��,ȫ�����ͷŵ�ǰ��stateBlock��Դ.
	void       sbm_onLostDevice( void );

};

/** �õ�һ��sg_timer��Instanceָ��.
*/
inline os_stateBlockMgr* os_stateBlockMgr::Instance()
{
	if( m_ptrInstance )
		return m_ptrInstance;
	else
	{
		m_ptrInstance = new os_stateBlockMgr;
		return m_ptrInstance;
	}
}


//! ��ʼ����Ⱦ״̬���������
inline void os_stateBlockMgr::init_sbMgr( LPDIRECT3DDEVICE9 _dev )
{
	osassert( _dev );
	m_pd3dDevice = _dev;
}




//! ��ʼ¼��״̬�顣
inline void os_stateBlockMgr::start_stateBlock( void )
{
	m_bRecordState = TRUE;
	if( FAILED( m_pd3dDevice->BeginStateBlock() ) )
		osassert( false );
}


/** \brief
*  ��ǰ��stateBlock ID �Ƿ���Ч.
*
*  ���device lost ,��ÿһ��id����Ч,��Ҫ���µĴ���ÿһ���ط���state block.
*/
inline bool os_stateBlockMgr::validate_stateBlockId( int _id )
{
	osassert( ( ((WORD)_id) >= 0)&&(((WORD)_id)<MAX_STATEBLOCK) );

	if( osn_mathFunc::get_hiword( _id ) == m_vecSBCreateTimes[(WORD)_id] )
		return m_vecSBUsed[(WORD)_id];
	else
		return false;
}
