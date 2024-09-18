//===================================================================
/** \file  
* Filename :   IndexBuffer.h
* Desc     :   
* His      :   Windy create @2005-10-15 9:18:01
*/
//===================================================================
#ifndef __INDEXBUFFER_H__
#define __INDEXBUFFER_H__

namespace Vertex
{
	class IndexBuffer
	{
	public:
		enum IndexType {
			IT_16BIT=2,
			IT_32BIT =4
		};
		IndexBuffer(IDirect3DDevice9*,size_t nIdx, IndexType IT = IT_16BIT);
		~IndexBuffer();

		bool isGood() const							{ return pIB_ != NULL; }
		size_t nIndices() const						{ return nIndices_; }
		const IndexType& indexType()				{ return indexType_; }

		void setIndices(IDirect3DDevice9* pDev);
		void Release();
		//!顶点数据存储操作类
		template <class V> class Access 
		{
		public:
			Access(IndexBuffer* pI): pBuffer_(pI), pBegin_(NULL), pEnd_(NULL)
			{
				//assert(V::vertexDescription() == pV->vertexDescription());
				assert( (sizeof(V)==IT_16BIT)||(sizeof(V)==IT_32BIT) );
				const size_t nBytes = pI->nIndices() * sizeof(V);
				const DWORD flags = D3DLOCK_NOOVERWRITE;

				void* pData = NULL;
				if (!FAILED(pI->pIB_->Lock(0, (UINT) nBytes, &pData, flags)))
				{
					pBegin_ = reinterpret_cast<V*>(pData);
					pEnd_ = pBegin_ + pI->nIndices();
				}
			}

			~Access()
			{
				if (pBegin_)
					pBuffer_->pIB_->Unlock();
			}

			typedef V* iterator;
			iterator begin()		{ return pBegin_; }
			iterator end()			{ return pEnd_; }

			V& operator[](size_t i)
			{
				assert(i < nIndices());
				V* pV = pBegin_ + i;
				return *pV;
			}

		private:
			IndexBuffer* const	pBuffer_;
			V*					pBegin_;
			V*					pEnd_;
		};

		template <class T> friend class Access;

	private:
		LPDIRECT3DINDEXBUFFER9 pIB_;
		IndexType indexType_;
		size_t nIndices_;
	};
	//////////////////////////////////////////////////////////////////////////
	inline
	IndexBuffer::IndexBuffer(IDirect3DDevice9* pDev,size_t nIdx, IndexType IT /* = IT_16BIT */):
		pIB_(NULL),
		nIndices_(nIdx),
		indexType_(IT)
	{
		if (IT==IT_16BIT){
			pDev->CreateIndexBuffer( static_cast<INT>(nIdx*IT), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB_, 0L );
		}
		else if (IT == IT_32BIT){
			pDev->CreateIndexBuffer( static_cast<INT>(nIdx*IT), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pIB_, 0L );
		}
	}
	inline
	IndexBuffer::~IndexBuffer()
	{
		//pIB_->Release();
		Release();
	}
	inline
	void IndexBuffer::setIndices(IDirect3DDevice9* pDev)
	{ 
		pDev->SetIndices(pIB_);
	}
	inline
	void IndexBuffer::Release()
	{
		if (pIB_){
			pIB_->Release();
			pIB_ = NULL;
		}
	}

}


#endif //__INDEXBUFFER_H__