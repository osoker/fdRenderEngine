////////////////////////////////////////////////////////////////////////////////////////////////
/** \file
 *  Filename: gc_r4ResourcePool.h
 *
 *  Desc:     ��Դ�صĻ��ࡣ
 *
 *  His:      raffaello created @ 2005-6-19
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _R4RESOURCEPOOL_H_
#define _R4RESOURCEPOOL_H_
#include <map>
#include <string>
template<typename _ResourceType>
class lru_Polics{
	typedef typename _ResourceType	resource_type;
	typedef typename resource_type*	resource_pointer;
	typedef typename resource_type&	resource_reference;
	typedef typename const _ResourceType&	const_resource_reference;

	typedef std::map<std::string,resource_pointer>	resources_;
};


template<typename _ResourceType,typename _RePolics	=	lru_Polics<_ResourceType> >
class I_r4ResourcePool
{
public:
	typedef typename _ResourceType	resource_type;
	typedef typename resource_type*	resource_pointer;
	typedef typename resource_type&	resource_reference;
	typedef typename const _ResourceType&	const_resource_reference;

	typedef std::map<std::string,resource_pointer>	resources_;
public:
	virtual ~I_r4ResourcePool(){destory_allresources();}
	size_t size()	const{return m_mResources.size();}
	bool	empty()	const{return m_mResources.empty();}
	//! ����Ƿ��Ѿ����������Դ
	virtual bool has_loaded(const std::string& szResourceName);
	//! ����������Դ
	virtual void destory_allresources();
	//! ����һ����Դ
	virtual bool destory_resource(const std::string& szResourceName);
	//! �ͷ�һ����Դ
	virtual bool unload_resource(const std::string& szResourceName);
	//! ����һ����Դ
	virtual resource_pointer get_resource(const std::string& szResourceName);
	//! ���ļ�����һ����Դ
	virtual resource_pointer load_resource(const std::string& szResourceName);
	//! ������Դ��·��
	virtual void set_rootpath(const std::string& szRootPath){m_szResourceRootPath	=	szRootPath;}
	//! �õ���Դ��·��
	const std::string get_rootpath()	const{return m_szResourceRootPath;}
protected:
	//! ��Դӳ���б�
	resources_	m_mResources;
	//! ��Դ��·��
	std::string								m_szResourceRootPath;
};




template<typename _ResourceType,typename _RePolics>
typename I_r4ResourcePool<_ResourceType,_RePolics>::resource_pointer 
I_r4ResourcePool<_ResourceType,_RePolics>::load_resource(const std::string& szResourceName)
{
	resource_pointer pResource	=	new resource_type;
	if(!pResource->load(std::string(m_szResourceRootPath+szResourceName).c_str()))
	{
		delete pResource;
		return NULL;
	}
	pResource->touch();
	if(m_mResources.insert(std::pair<std::string,resource_pointer>(szResourceName,pResource)).second)
		return pResource;
	return NULL;
}


template<typename _ResourceType,typename _RePolics>
typename I_r4ResourcePool<_ResourceType,_RePolics>::resource_pointer 
I_r4ResourcePool<_ResourceType,_RePolics>::get_resource(const std::string& szResourceName)
{
	resources_::iterator	itor	=	m_mResources.find(szResourceName);
	if(itor!=m_mResources.end())
	{//�Ѿ��������Դ,ֱ�ӷ�����
		resource_pointer pResource	=	itor->second;
		pResource->touch();
		return pResource;
	}
	//��û�������Դ,��Ӳ���ж�ȡ
	return load_resource(szResourceName);
}


template<typename _ResourceType,typename _RePolics>
bool I_r4ResourcePool<_ResourceType,_RePolics>::unload_resource(const std::string& szResourceName)
{
	resources_::iterator	itor	=	m_mResources.find(szResourceName);
	if(itor!=m_mResources.end())
	{//�Ѿ��������Դ,ֱ�ӷ�����
		resource_pointer pResource	=	itor->second;
		pResource->unload();


		return true;
	}
	return false;
}


template<typename _ResourceType,typename _RePolics>
bool I_r4ResourcePool<_ResourceType,_RePolics>::destory_resource(const std::string& szResourceName)
{
	resources_::iterator	itor	=	m_mResources.find(szResourceName);
	if(itor!=m_mResources.end())
	{
		resource_pointer pResource	=	itor->second;
		pResource->destory();
		delete pResource;
		m_mResources.erase(itor);
		return true;
	}
	return false;
}


template<typename _ResourceType,typename _RePolics>
bool I_r4ResourcePool<_ResourceType,_RePolics>::has_loaded(const std::string& szResourceName)
{
	resources_::iterator	itor	=	m_mResources.find(szResourceName);
	if(itor!=m_mResources.end())
	{
		return true;
	}
	return false;
}


template<typename _ResourceType,typename _RePolics>
void I_r4ResourcePool<_ResourceType,_RePolics>::destory_allresources()
{
	resources_::iterator	itor	=	m_mResources.begin();
	for(;itor!=m_mResources.end();itor++)
	{
		resource_pointer pResource	=	itor->second;
		pResource->destory();
		delete pResource;
	}
	m_mResources.clear();
}


#endif//_R4RESOURCEPOOL_H_