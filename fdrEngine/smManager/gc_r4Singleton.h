#ifndef _R4SINGLETON_H_
#define _R4SINGLETON_H_
#include <assert.h>
#pragma warning (disable : 4311)
#pragma warning (disable : 4312)

#define CREATESINGLETON(_name) if(_name::getSingletonPtr()==NULL) static _name __##_name

template <typename T>
class gc_r4Singleton
{
	public:
		typedef T singleton_type;
    protected:		
        static T* ms_Singleton;		
    public:
        gc_r4Singleton( void )
        {
            assert( !ms_Singleton );
            int offset = (int)(T*)1 - (int)(gc_r4Singleton <T>*)(T*)1;
            ms_Singleton = (T*)((int)this + offset);
        }
        virtual ~gc_r4Singleton( void )
		{  assert( ms_Singleton );  ms_Singleton = 0;  }
        static T& getSingleton( void )
		{  assert( ms_Singleton );  return ( *ms_Singleton );  }
        static T* getSingletonPtr( void )
		{  return ( ms_Singleton );  }
};
#endif//EOF _R4SINGLETON_H_