/** \file QuadTree.h
\n 功能描述 :   四叉树模板类,管理以AABB方式组织的节点，能快速找到与传入的AABB相交的子节点.
参考 http://www.irrlichtnx.mmdevel.de/docs/OctTree_8h-source.html
\author         windy   
\date           2006-7-18 15:02:47*/
#ifndef __QUADTREE_H__
#define __QUADTREE_H__

# include "../../interface/osInterface.h"
using namespace std;

// 节点最多包含数据个数
#define NodeDataNr 3
//! 四叉树模板类
template <class T>
class QuadTree
{
public:
	int mNodeCount;
	vector<T> mNodeList;
	vector<T*> mCurrentNodePtrList;
public:
	QuadTree()
	{
		mRoot = NULL;
	};
	~QuadTree()
	{
		if (mRoot)
		{
			delete mRoot;
		}
	}
	void Compiler(const vector<T>& _alldataList,const vector<os_aabbox>& _allboxList)
	{
		osassert(_allboxList.size()==_alldataList.size());
		mNodeCount = 0;
		mNodeList = _alldataList;
		vector<int> indexList;
		// 加入所有节点
		for (size_t i = 0; i < _alldataList.size();i++)
		{
			indexList.push_back((int)i);
		}
		mRoot = new QuadTreeNode(mNodeCount, 0,mNodeList, _allboxList, indexList);
	}
	void Calculate(const os_aabbox& box)
	{
		if (mRoot)
		{
			os_aabbox __tempBox = box;
			__tempBox.recalc_aabbox();
			mCurrentNodePtrList.clear();
			mRoot->getInsertNode(__tempBox,mCurrentNodePtrList);
		}
		
	}
	void Calculate(const os_bsphere& sphere)
	{
		if (mRoot)
		{
			mCurrentNodePtrList.clear();
			mRoot->getInsertNode(sphere,mCurrentNodePtrList);
		}
	}
	vector<T*> GetNodePtrList() const
	{
		return mCurrentNodePtrList;
	}


private:
	// 内部私有类
	//! 四叉树节点 
	class QuadTreeNode
	{
	public:
		QuadTreeNode(int& _nodeCount, int _currentdepth,
			vector<T>& _alldataList,
			const vector<os_aabbox>& _allboxList,
			const vector<int>&	_indices)
		{


			mDepth = _currentdepth+1;
			++_nodeCount;

			// 清空子节点
			for (int i=0; i<4; ++i)
				mChildren[i] = NULL;
			// 计算当前节点的AABBOX
			mBox.m_vecMax = osVec3D( float(SG_MIN_VALUE),float(SG_MIN_VALUE),float(SG_MIN_VALUE) );
			mBox.m_vecMin = osVec3D( float(SG_MAX_VALUE),float(SG_MAX_VALUE),float(SG_MAX_VALUE) );
			for (size_t i = 0; i < _indices.size();i++)
			{
				mBox.extend(_allboxList[_indices[i]].m_vecMin);
				mBox.extend(_allboxList[_indices[i]].m_vecMax);
			}
			mBox.recalc_aabbox();

			if (_indices.size() < NodeDataNr)
			{
				mIsLeft = TRUE;
				for (size_t ch = 0 ; ch < _indices.size();ch++)
				{
					mDataPtrList.push_back(&_alldataList[_indices[ch]]);
				}
			}
			else
			{
				mIsLeft = FALSE;
				// 计算四个子空间的数据索引
				os_aabbox ChildrenBox[4];
				osVec3D Center = mBox.get_center();

				ChildrenBox[0].m_vecMin = Center - osVec3D(0.0f,(mBox.m_vecMax.y - mBox.m_vecMin.y)/2.0f,0.0f);
				ChildrenBox[0].m_vecMax = mBox.m_vecMax;

				ChildrenBox[1].m_vecMin = ChildrenBox[0].m_vecMin - osVec3D(0.0f,0.0f,(mBox.m_vecMax.z - mBox.m_vecMin.z)/2.0f);
				ChildrenBox[1].m_vecMax = ChildrenBox[0].m_vecMax - osVec3D(0.0f,0.0f,(mBox.m_vecMax.z - mBox.m_vecMin.z)/2.0f);


				ChildrenBox[2].m_vecMin = ChildrenBox[1].m_vecMin - osVec3D((mBox.m_vecMax.x - mBox.m_vecMin.x)/2.0f,0.0f,0.0f);
				ChildrenBox[2].m_vecMax = ChildrenBox[1].m_vecMax - osVec3D((mBox.m_vecMax.x - mBox.m_vecMin.x)/2.0f,0.0f,0.0f);

				ChildrenBox[3].m_vecMin = ChildrenBox[2].m_vecMin + osVec3D(0.0f,0.0f,(mBox.m_vecMax.z - mBox.m_vecMin.z)/2.0f);
				ChildrenBox[3].m_vecMax = ChildrenBox[2].m_vecMax + osVec3D(0.0f,0.0f,(mBox.m_vecMax.z - mBox.m_vecMin.z)/2.0f);

				
				for (int ch=0; ch<4; ++ch)
				{
					vector<int> childrenIndices;
					
					for (size_t i = 0; i < _indices.size();i++)
					{
						if ( ChildrenBox[ch].pt_inBox(_allboxList[_indices[i]].get_center()) )
						{
							childrenIndices.push_back(_indices[i]);
						}
					}
					if ( childrenIndices.size() > 0 )
					{
						mChildren[ch] = new QuadTreeNode(_nodeCount,_currentdepth,_alldataList,_allboxList,childrenIndices);
					}
				}
			}
		}

		~QuadTreeNode()
		{
			for (int i=0; i<4; ++i)
			{
				if (mChildren[i])
				{
					delete mChildren[i];
				}
			}
		}

		void getInsertNode(const os_aabbox& box,vector<T*>& nodePtrList)
		{
			if (mBox.collision(box))
			{
				if (mIsLeft)
				{
					for (size_t i = 0 ; i < mDataPtrList.size();i++)
					{
						nodePtrList.push_back(mDataPtrList[i]);
					}
				}
				else
				{
					for (int i=0; i<4; ++i)
					{
						if (mChildren[i])
						{
							mChildren[i]->getInsertNode(box,nodePtrList);
						}
					}
				}
			}

		}
		void getInsertNode(const os_bsphere& sphere,vector<T*>& nodePtrList)
		{
			if (mBox.collision(sphere))
			{
				if (mIsLeft)
				{
					for (size_t i = 0 ; i < mDataPtrList.size();i++)
					{
						nodePtrList.push_back(mDataPtrList[i]);
					}
				}
				else
				{
					for (int i=0; i<4; ++i)
					{
						if (mChildren[i])
						{
							mChildren[i]->getInsertNode(sphere,nodePtrList);
						}
					}
				}
			}
		}


		BOOL	  mIsLeft;
		os_aabbox mBox;
		vector<T*> mDataPtrList;
		/*
		 3 | 0
		 -----
		 2 | 1
		*/
		QuadTreeNode* mChildren[4];
		int mDepth;

	};

	QuadTreeNode* mRoot;
};

#endif //__QUADTREE_H__