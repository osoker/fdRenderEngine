//////////////////////////////////////////////////////////////////////////////////////
/** \brief
 *  Filename: sgLru.h
 *
 *  His:      River created @ 12/27 2003.
 *
 *  Desc:     least-recently Used 内存淘汰算法和基类.用于动态的分配和秋释放系统
 *            资源.
 *            文件中也包括了其它小的工具类。
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
 *  引用计数相关的基类。
 *
 *  使用引用计数的类可以从这个类派生。
 */
class COMMON_API os_refObj
{
private:
	int    m_iRefNum;

protected:

	/** \brief
	 *  释放当前资源的纯虚函数，由release_ref函数调用。
	 *  
	 */
	virtual void release_obj( DWORD _ptr = NULL ) = 0;

public:
	os_refObj();

	//! 　 加入一个新的ref.
	int    add_ref( void );
	//!    减去一个ref.
	int    release_ref( DWORD _ptr = NULL );
	/** \brief
	 *  得到当前ref的数值。
	 *
	 *  这个函数还有一个重要的用途是：
	 *  如果当前的ref值为零，表明当前的obj是空的obj,
	 *  可以做为未初始化的obj使用。
	 */
	int    get_ref( void );

};



/** \brief
 *  lru的基类.所有需要使用lru算法和管理器都需要从这个类进行继承.
 *
 *  使用大块的地图或是地图中很多物品时,需要lru的支持,以使用无缝的
 *  调入和删除内存中的物品.
 *  lurElement
 */
class COMMON_API  os_lruEle
{
private: 
	//!   有多少帧和时间没有用到当前的元素。
	int         m_iNotUsed;

public:
	os_lruEle();
	~os_lruEle();

	//! 设置有多少帧或是时间没有用到当前的元素。
	void        add_nu( int _nu = 1 );

	//! 得到当前没有使用的时间或是帧数。　
	int         get_nu( void );

	//! 设置使用使用了当前的元素。
	void        set_used( void );

};

/** \brief
 *  对于某些可能自删除的元素，使用这个类来确认我们要处理的id是否是我们创建时的ID
 *
 *  比如Decal，DecalMgr创建了一个Decal后，返回给上层一个Id.这个Decal又是自删除的
 *  Decal,当这个Decal的生命周期完成时，内部把这个Decal给删除了，而这个Decal实例的
 *  位置又创建了一个新的Decal.此时如果上层想要删除原来的Dedal,就会出现问题，即新创
 *  的Decal被删除。
 */
class COMMON_API  os_createVal
{
private:
	//! 此元素的创建值
	WORD          m_wCreateVal;
	//! 此实例是否初始化完毕
	WORD          m_wInitialized;
public:
	os_createVal() { m_wCreateVal = 0; }
	
	void          increase_val( void ) 
	{ 
		m_wCreateVal ++ ; 

		// River @ 2011-2-26:为了上层不出现小于零的ID。
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
	 *  因为程序内的部实例都是多线程调入，所以每一个实例在frameMove之前，必须确认
	 *  已经初始化
	 */
	void          reset_ins( void ) { m_wInitialized = 0 ; }
	void          set_insInit( void ) {  m_wInitialized = 1; }
	BOOL          get_insInit( void ) { return m_wInitialized; }
	//@}

};


/** \brief
*  空间管理相关的模板
*
*  目前引擎中对空间的管理比较低效：在队列中查找相应的空闲远元素，
*  然后使用。使用这个模板，可以提高效率。
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

	//! 设置初始化回调,空间不足时，new之后调用用户初始化函数
	void set_initCallback( SPACEMGR_INITCALLBACK _pfnInitCallback )
	{
		m_pfnInitCallback = _pfnInitCallback;
	}

	//! 初始化时，为结点管理器初始化出一部分内存
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

	//! 可用的最大id.
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
			// River Mod @ 2005-10-12:永远都有freeNode.
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

	//! 得到固写的某一个node索引
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
	*  找出并使用一个freeNode.
	*
	*  \param _useStaticSize 如果此值为true,则空间不够的话，返回空。
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
				// River Mod @ 2005-10-12:永远都有freeNode.
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

	//! 把一个结点放到非使用队列，但结点对应的内存仍然在使用中.
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

	//! 释放当前管理器对应的所有内存
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

	//! 得到结点的内容指针
	T* get_nodePtr( int _idx ){
		if(_idx<0 || _idx>=(int)m_use.size())
			return NULL;

		return m_use[_idx];
	}

	//! 分配内存后初始化函数调用
	UNUSE_LIST* get_unuseList(){
		return &m_unuse;
	}

	//！使用[]的下标符
	T& operator[]( int _idx )
	{
		osassertex( (_idx>=0)&&(_idx<int(m_use.size())),"下标越界...\n" );
		osassertex( m_use[_idx],"索引内的项没有使用..\n" );
		return *m_use[_idx];
	}

	//@{
	//! 用于对当前空间管理器内可用的数据结构进行遍历
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