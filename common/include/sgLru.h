//////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: sgLru.h
 *
 *  His:      River created @ 12/27 2003.
 *
 *  Desc:     least-recently Used �ڴ���̭�㷨�ͻ���.���ڶ�̬�ķ�������ͷ�ϵͳ
 *            ��Դ.
 *            �ļ���Ҳ����������С�Ĺ����ࡣ
 *  
 */
//////////////////////////////////////////////////////////////////////////////////////
# pragma once

# include "windows.h"
# include "stdio.h"
# include "../common.h"
# include "sgdebug.h"

# include <list>
# include <vector>

/** \brief
 *  ���ü�����صĻ��ࡣ
 *
 *  ʹ�����ü���������Դ������������
 */
class COMMON_API os_refObj
{
private:
	int    m_iRefNum;

protected:

	/** \brief
	 *  �ͷŵ�ǰ��Դ�Ĵ��麯������release_ref�������á�
	 *  
	 */
	virtual void release_obj( DWORD _ptr = NULL ) = 0;

public:
	os_refObj();

	//! �� ����һ���µ�ref.
	int    add_ref( void );
	//!    ��ȥһ��ref.
	int    release_ref( DWORD _ptr = NULL );
	/** \brief
	 *  �õ���ǰref����ֵ��
	 *
	 *  �����������һ����Ҫ����;�ǣ�
	 *  �����ǰ��refֵΪ�㣬������ǰ��obj�ǿյ�obj,
	 *  ������Ϊδ��ʼ����objʹ�á�
	 */
	int    get_ref( void );

};



/** \brief
 *  lru�Ļ���.������Ҫʹ��lru�㷨�͹���������Ҫ���������м̳�.
 *
 *  ʹ�ô��ĵ�ͼ���ǵ�ͼ�кܶ���Ʒʱ,��Ҫlru��֧��,��ʹ���޷��
 *  �����ɾ���ڴ��е���Ʒ.
 *  lurElement
 */
class COMMON_API  os_lruEle
{
private: 
	//!   �ж���֡��ʱ��û���õ���ǰ��Ԫ�ء�
	int         m_iNotUsed;

public:
	os_lruEle();
	~os_lruEle();

	//! �����ж���֡����ʱ��û���õ���ǰ��Ԫ�ء�
	void        add_nu( int _nu = 1 );

	//! �õ���ǰû��ʹ�õ�ʱ�����֡������
	int         get_nu( void );

	//! ����ʹ��ʹ���˵�ǰ��Ԫ�ء�
	void        set_used( void );

};

/** \brief
 *  ����ĳЩ������ɾ����Ԫ�أ�ʹ���������ȷ������Ҫ�����id�Ƿ������Ǵ���ʱ��ID
 *
 *  ����Decal��DecalMgr������һ��Decal�󣬷��ظ��ϲ�һ��Id.���Decal������ɾ����
 *  Decal,�����Decal�������������ʱ���ڲ������Decal��ɾ���ˣ������Decalʵ����
 *  λ���ִ�����һ���µ�Decal.��ʱ����ϲ���Ҫɾ��ԭ����Dedal,�ͻ�������⣬���´�
 *  ��Decal��ɾ����
 */
class COMMON_API  os_createVal
{
private:
	//! ��Ԫ�صĴ���ֵ
	WORD          m_wCreateVal;
	//! ��ʵ���Ƿ��ʼ�����
	WORD          m_wInitialized;
public:
	os_createVal() { m_wCreateVal = 0; }
	
	void          increase_val( void ) 
	{ 
		m_wCreateVal ++ ; 

		// River @ 2011-2-26:Ϊ���ϲ㲻����С�����ID��
		if( m_wCreateVal == 32767 )
			m_wCreateVal = 0;
	}
	
	WORD          get_val( void  ) { return m_wCreateVal; }

	BOOL          validate_cval( DWORD _id )
	{
		if( m_wCreateVal == WORD((_id&0xffff0000)>>16) )
			return TRUE;
		else
			return FALSE;
	}

	//@{
	/** \brief
	 *  ��Ϊ�����ڵĲ�ʵ�����Ƕ��̵߳��룬����ÿһ��ʵ����frameMove֮ǰ������ȷ��
	 *  �Ѿ���ʼ��
	 */
	void          reset_ins( void ) { m_wInitialized = 0 ; }
	void          set_insInit( void ) {  m_wInitialized = 1; }
	BOOL          get_insInit( void ) { return m_wInitialized; }
	//@}

};


/** \brief
*  �ռ������ص�ģ��
*
*  Ŀǰ�����жԿռ�Ĺ���Ƚϵ�Ч���ڶ����в�����Ӧ�Ŀ���ԶԪ�أ�
*  Ȼ��ʹ�á�ʹ�����ģ�壬�������Ч�ʡ�
*  
*/
template <class T>
class CSpaceMgr
{
public:
	typedef int ( * SPACEMGR_INITCALLBACK )(T*);
public:
	struct NODE
	{
		int idx;
		T* p;
	};
	typedef std::list<NODE> UNUSE_LIST;
	typedef std::vector<T*>   USE_LIST;
public:
	CSpaceMgr() :m_pfnInitCallback(0) {}
	CSpaceMgr( int _size ):m_pfnInitCallback(0){
		resize(_size); }
	~CSpaceMgr( void ){
		destroy_mgr();
	}

	//! ���ó�ʼ���ص�,�ռ䲻��ʱ��new֮������û���ʼ������
	void set_initCallback( SPACEMGR_INITCALLBACK _pfnInitCallback )
	{
		m_pfnInitCallback = _pfnInitCallback;
	}

	//! ��ʼ��ʱ��Ϊ����������ʼ����һ�����ڴ�
	void resize( int _size ){
		destroy_mgr();
		NODE n;
		m_use.resize(_size );
		for(int i=0;i<_size;++i)
		{
			n.idx=i;
			n.p=new T;
			if( m_pfnInitCallback )
				(*m_pfnInitCallback)(n.p);
			m_unuse.push_back(n);

			//m_use[i] = NULL;
		}
	}
	void resize( int _size, T _defaultValue ){
		release_all();
		NODE n;
		m_use.resize(_size,NULL);
		for(int i=0;i<_size;++i)
		{
			n.idx = i;
			n.p = new T;
			*n.p = _defaultValue;
			m_unuse.push_back(n);
		}
	}

	//! ���õ����id.
	int size() { return (int)m_use.size(); }
	BOOL validate_id( int _id )
	{
		osassertex( _id < int(m_use.size()),va( "The id is<%d>...\n",_id) );
		osassertex( _id >= 0,va( "The id is<%d>...\n",_id) );
		if( m_use[_id] == NULL )
			return FALSE;
		else
		{
			//osassertex( !::IsBadReadPtr( m_use[_id],sizeof( DWORD ) ),va( "The idx is<%d>",_id ) );
			return TRUE;
		}
	}

	int get_freeNode( void )
	{
		if(m_unuse.size()==0)
		{
			// River Mod @ 2005-10-12:��Զ����freeNode.
			NODE    n;
			n.p = new T;
			n.idx = (int)m_use.size();

			m_use.push_back( n.p );
			return int(m_use.size())-1;
		}

		int idx = m_unuse.front().idx;
		m_use[idx] = m_unuse.front().p;
		m_unuse.pop_front();
		return idx;
	}

	//! �õ���д��ĳһ��node����
	BOOL get_nodeIndexAndUse( int _idx,T** _pp )
	{
		osassert( _idx >= 0 );

		std::list<NODE>::iterator  t_iter;

		for( t_iter = m_unuse.begin();
			t_iter != m_unuse.end();t_iter ++ )
		{
			if( t_iter->idx == _idx )
			{
				m_use[_idx] = t_iter->p;
				*_pp = t_iter->p;
				m_unuse.erase( t_iter );
				return TRUE;
			}
		}

		*_pp = NULL;
		return FALSE;
	}

	/** \brief
	*  �ҳ���ʹ��һ��freeNode.
	*
	*  \param _useStaticSize �����ֵΪtrue,��ռ䲻���Ļ������ؿա�
	*/
	int get_freeNodeAUse( T** _pp,BOOL _useStaticSize = FALSE )
	{
		if( 0 == m_unuse.size() )
		{
			if( _useStaticSize )
			{
				*_pp = NULL;
				return -1;
			}
			else
			{
				// River Mod @ 2005-10-12:��Զ����freeNode.
				NODE    n;
				n.p = new T;
				n.idx = (int)m_use.size();
				// syq 5-18 add
				if( m_pfnInitCallback )
					(*m_pfnInitCallback)(n.p);


				m_use.push_back( n.p );
				*_pp = n.p;
				return int(m_use.size())-1;
			}
		}

		int idx = m_unuse.front().idx;
		m_use[idx] = m_unuse.front().p;
		*_pp=m_use[idx];
		m_unuse.pop_front();
		return idx;
	}

	//! ��һ�����ŵ���ʹ�ö��У�������Ӧ���ڴ���Ȼ��ʹ����.
	BOOL release_node( int _idx ){
		if(_idx<0 || _idx>=(int)m_use.size())
			return FALSE;
		if( m_use[_idx] )
		{
			NODE n;

			n.idx=_idx;
			n.p=m_use[_idx];
			m_unuse.push_back( n );

			m_use[_idx]=NULL;
		}
		return TRUE;
	}

	//! �ͷŵ�ǰ��������Ӧ�������ڴ�
	void destroy_mgr(){
		int i;
		for(i=0;i<int(m_use.size());++i){
			if(m_use[i])
				delete m_use[i];
		}
		m_use.clear();
		UNUSE_LIST::iterator it = m_unuse.begin();
		for(;it!=m_unuse.end();++it)
		{ 
			if(it->p)
				delete it->p;
		}
		m_unuse.clear();
	}

	//! �õ���������ָ��
	T* get_nodePtr( int _idx ){
		if(_idx<0 || _idx>=(int)m_use.size())
			return NULL;

		return m_use[_idx];
	}

	//! �����ڴ���ʼ����������
	UNUSE_LIST* get_unuseList(){
		return &m_unuse;
	}

	//��ʹ��[]���±��
	T& operator[]( int _idx )
	{
		osassertex( (_idx>=0)&&(_idx<int(m_use.size())),"�±�Խ��...\n" );
		osassertex( m_use[_idx],"�����ڵ���û��ʹ��..\n" );
		return *m_use[_idx];
	}

	//@{
	//! ���ڶԵ�ǰ�ռ�������ڿ��õ����ݽṹ���б���
	NODE begin_usedNode( void )
	{
		int   t_i;
		NODE  t_n;

		for( t_i=0;t_i<(int)m_use.size();t_i ++ )
		{
			if( NULL != m_use[t_i] )
			{
				t_n.idx = t_i;
				t_n.p = m_use[t_i];
				return t_n;
			}
		}

		t_n.idx = -1;
		t_n.p = NULL;

		return t_n;
	}

	NODE next_validNode( NODE* _node )
	{
		int t_i;
		NODE  t_n;

		for( t_i=_node->idx+1;t_i<(int)m_use.size();t_i ++ )
		{
			if( NULL != m_use[t_i] )
			{
				t_n.idx = t_i;
				t_n.p = m_use[t_i];
				return t_n;
			}
		}
		t_n.idx = -1;
		t_n.p = NULL;

		return t_n;
	}
	//@} 

private:

	UNUSE_LIST m_unuse;
	USE_LIST   m_use;
	SPACEMGR_INITCALLBACK m_pfnInitCallback;

};


/*! \brief template of ObjectStaticMgr to manage memory allocating
*
*		This template manager is faster than calling malloc purely	
*		
*		\param maxStackCon		: max memory space of object's address  on the stack
*/
template<typename T,int maxStackCon>
class ObjectStaticAllocMgr
{
	static const int scm_iMaxContainer	= maxStackCon;

	//@{ stack buffer to initialize very fast
	// serious bug:
	// 
	// T cannot holder the std::list variable.....
	//
	//
	T*						m_pStackContainer;
	bool					m_bStackUsed[scm_iMaxContainer];		//!< the flag to indicate this space whether be used
	//@}

	//!	debug information to record the object's number
	//! to adjust the stack and heap space  
	int						m_iUsedObjectCounter;
	int						m_iAllocateObjectMem;

public:

	ObjectStaticAllocMgr(void):m_iUsedObjectCounter(0),m_iAllocateObjectMem(0)


	{
		memset(m_bStackUsed,0,sizeof(m_bStackUsed));
		m_pStackContainer = (T*)::malloc(sizeof(T) * scm_iMaxContainer);
	}

	~ObjectStaticAllocMgr(void)
	{
		::free(m_pStackContainer);
	}

public:

	/*! \brief find the free space on the stack first and
	*			heap secondly
	*			this function is linear complexity O(n) 
	*/
	T* GetFreeObject(void)
	{
		// record the object using counter
		//
		m_iUsedObjectCounter++;

		// search unused object on the stack
		for(int i = 0 ; i< scm_iMaxContainer ;i++){
			if(m_bStackUsed[i] == false){
				m_bStackUsed[i] = true;

				return &(m_pStackContainer[i]);
			}
		}

		m_iAllocateObjectMem++;

		return (T*)malloc(sizeof(T));

	}

	/*! \brief This fucntion using binary searching the address to release
	*			because the address on the stack is ordered...
	*/
	void FreeObject(T* p)
	{
		m_iUsedObjectCounter--;

		if(p >= &(m_pStackContainer[0]) && p<= &(m_pStackContainer[scm_iMaxContainer - 1])){

			const unsigned long t_index = ((unsigned long)p - (unsigned long)&(m_pStackContainer[0])) / sizeof(T);

#ifdef _DEBUG
			if(m_bStackUsed[t_index] != true){
				__asm{int 3};
			}
#endif //_DEBUG
			m_bStackUsed[t_index] = false;

		}else{

			m_iAllocateObjectMem--;

			free((void*)p);
		}
	}

	int GetUsedObjectNum(void)const {return m_iUsedObjectCounter;}
	int GetAllocateObjectNum(void)const {return m_iAllocateObjectMem;}
	int GetStackObjectNum(void)const {return scm_iMaxContainer;}

};