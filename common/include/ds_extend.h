
//================================================================================================
/** \file 
 *  Filename: ds_extend.h
 *
 *  His: snowwin create @ 2006-1-16
 *
 *  Des: 有时需要的使用静态内存的数据结构。只是满足某些需求，并没有把它们写得很完善。
 *
 */ 
//================================================================================================

#ifndef __SW_DATA_STRUCT_EXTEND_H__
#define __SW_DATA_STRUCT_EXTEND_H__

#include <vector>


template< typename T > class static_vector
{
public:
	static_vector( void ):buffer_(0),size_(0),nodeCount_(0){}
	static_vector( int size, T freeValue ){ create( size, freeValue ); }

	~static_vector( void ){	if( buffer_ )free( buffer_ ); }

	void create( int size, T freeValue )
	{
		nodeCount_ = 0;
		size_ = size; freeValue_ = freeValue;
		buffer_ = (T*)malloc( sizeof(T)*size_ );
		clear();
	}
	void resize( int size ){ buffer_ = (T*)realloc( buffer_, size ); size_ = size; }

	int get_freeNode( int startPos = 0 )
	{
		for( int i = startPos; i < size_; ++i )
			if( buffer_[i] == freeValue_ )return i;
		int tmp = size_;
		resize( size_ * 2 );
		return get_freeNode( tmp );
	}

	void clear( void ){ for( int i = 0; i < size_; ++i )buffer_[i] = freeValue_; nodeCount_ = 0;}
	void push( T data ){ buffer_[get_freeNode()] = data; ++ nodeCount_; }
	void push_cover( T data )
	{
		for( int i = 0; i < size_; ++i )
		{
			if( buffer_[i] == freeValue_ )
			{
				buffer_[i] = data;
				++ nodeCount_;
				return;
			}
			else if( buffer_[i] == data )
			{
				return;
			}
		}
		resize( size_ * 2 );
		return push_cover( data );
	}
	void erase( int pos ){ if(value(pos)==freeValue_)return; buffer_[pos]=freeValue_;--nodeCount_; }

	int size( void ){ return size; }
	T value( int pos ){ return buffer_[pos]; }
	int nodeCount( void ){ return nodeCount_;}

private:
	T*		buffer_;
	int		size_;
	T		freeValue_;
	int		nodeCount_;
};

//----------------------------------------------------------------------


template < typename T > class static_set
{
public:
	static_set( void )							:m_size(0),m_defValue(T()){}
	static_set( int _size )						{ resize( _size ); m_size = 0;}
	static_set( int _size, const T& defValue )	{ m_buffer.resize( _size, defValue ); m_size = 0; }

	void	resize( int _size, const T& defValue = T() ){ m_defValue = defValue; m_buffer.resize( _size, defValue );}
	int		size( void )			{ return m_size; }
	T*		value_ptr( int id )		{ return &( m_buffer[id] );}

	void insert_up( const T& val )
	{
		for( int i = 0; i < m_size; ++i )
		{
			if( m_buffer[i] == val )
				return;
		}

		while( m_size >= (int)m_buffer.size() )
			m_buffer.push_back( m_defValue );

		m_buffer[m_size] = val;
		++m_size;
	}
	void insert( const T& val )
	{
		for( int i = 0; i < m_size; ++i )
		{
			if( m_buffer[i] == val )
				return;
		}
		m_buffer[m_size] = val;
		++m_size;
	}
	void push_up( const T& val )
	{
		while( m_size >= (int)m_buffer.size() )
			m_buffer.push_back( m_defValue );

		m_buffer[m_size] = val;
		++m_size;
	}
	void push( const T& val )
	{
		m_buffer[m_size] = val;
		++m_size;
	}

	void clear( void )				{ m_size = 0; }

private:
	T					m_defValue;
	std::vector< T >	m_buffer;
	int					m_size;
};

//----------------------------------------------------------------------

template < typename K, typename D >struct pair_t
{
	pair_t( void ):key( K() ),data( D() ){}
	pair_t( K k, D d ): key(k),data(d){}

	K key;
	D data;
};

template < typename T > class static_intMap
{
public:
	static_intMap( void ):m_defValue(pair_t<int,T>(0,T())){}
	static_intMap( int _size ){ resize( _size ); }
	static_intMap( int _size, const T& defValue ){ resize( _size, defValue );}
	~static_intMap( void ){}

	void resize( int _size, T defVal = T() )
	{
		m_defValue = pair_t<int,T>(0,defVal);
		m_buffer.resize( _size, m_defValue );
	}

	T& operator [] ( int key )
	{
		for( int i = 0; i < m_currPos; ++i )
		{
			if( m_buffer[i].key == key )
				return m_buffer[i].data;
		}

		while( m_currPos >= (int)m_buffer.size() )
			m_buffer.push_back( m_defValue );

		++ m_currPos;
		m_buffer[ m_currPos-1 ].key = key;
		return ( m_buffer[ m_currPos-1 ].data );
	}

	int size( void ){ return m_currPos;}

	T* value_ptr( int id, int& key )
	{
		key = m_buffer[id].key;
		return &( m_buffer[ id ].data );
	}

	void clear( void )	{ m_currPos = 0; }

private:
	pair_t<int,T>					m_defValue;
	std::vector< pair_t<int,T> >	m_buffer;
	int								m_currPos;
};


#endif //__SW_DATA_STRUCT_EXTEND_H__
