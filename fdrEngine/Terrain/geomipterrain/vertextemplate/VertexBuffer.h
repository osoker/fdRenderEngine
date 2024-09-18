//===================================================================
/** \file  
* Filename :   VertexBuffer.h
* Desc     :   A thin wrapper for Direct3D vertex buffers.  It adds some memory management
*			   and associates vertex metadata - fields - with the class.

* Note	   :   the field layout and element size don't have to belong here.  D3D 
*			   won't prevent you from putting a mix of different vertex types into one
*			   vertex buffer.
* His      :   Windy create @2005-10-20 16:01:17
*/
//===================================================================
#ifndef VERTEX_VertexBuffer_h
#define VERTEX_VertexBuffer_h

#include <assert.h>
#include <D3D9.h>

namespace Vertex
{
	class VertexBuffer
	{
	public:
		VertexBuffer(IDirect3DDevice9*, const VertexDescription&, size_t nVtx);
		~VertexBuffer();

		bool isGood() const								{ return pVB_ != NULL; }
		size_t nVertices() const						{ return nVertices_; }
		const VertexDescription& vertexDescription()	{ return description_; }

		void setStreamSource(IDirect3DDevice9* pDev, UINT streamNo);
		void Release();

		// This is somewhat simplified.  A real example should implement 
		// "template hoisting".  The Lock/Unlock functions can be put into a
		// non-template base class to avoid code bloat.
		template <class V> class Access 
		{
		public:
			Access(VertexBuffer* pV): pBuffer_(pV), pBegin_(NULL), pEnd_(NULL)
			{
				assert(V::vertexDescription() == pV->vertexDescription());
				const size_t nBytes = pV->nVertices() * V::vertexDescription().sizeofVertex();
			//	const DWORD flags = D3DLOCK_NOOVERWRITE|D3DLOCK_DISCARD;
				const DWORD flags = D3DLOCK_NOOVERWRITE;

				void* pData = NULL;
				if (!FAILED(pV->pVB_->Lock(0, (UINT) nBytes, &pData, flags)))
				{
					pBegin_ = reinterpret_cast<V*>(pData);
					pEnd_ = pBegin_ + pV->nVertices();
				}
			}

			~Access()
			{
				if (pBegin_)
					pBuffer_->pVB_->Unlock();
			}

			typedef V* iterator;
			iterator begin()		{ return pBegin_; }
			iterator end()			{ return pEnd_; }

			V& operator[](size_t i)
			{
				assert(i < nVertices());
				V* pV = pBegin_ + i;
				return *pV;
			}

		private:
			VertexBuffer* const	pBuffer_;
			V*					pBegin_;
			V*					pEnd_;
		};

		template <class T> friend class Access;

	private:
		IDirect3DVertexBuffer9*		pVB_;
		const size_t				nVertices_;
		const VertexDescription&	description_;
	};

	inline
	VertexBuffer::VertexBuffer(IDirect3DDevice9* pDev, const VertexDescription& desc, size_t nVtx): 
		pVB_(NULL),
		nVertices_(nVtx),
		description_(desc)
	{
		const UINT nBytes = static_cast<UINT>(nVtx * desc.sizeofVertex());
		pDev->CreateVertexBuffer(nBytes, D3DUSAGE_DYNAMIC |D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVB_, NULL);
		if (pVB_==NULL)
		{
			assert(pVB_);
		}
	}

	inline
	VertexBuffer::~VertexBuffer()
	{
		//pVB_->Release();
		Release();
	}

	inline
	void VertexBuffer::setStreamSource(IDirect3DDevice9* pDev, UINT streamNo)
    { 
		const UINT offset=0;
		pDev->SetStreamSource(streamNo, pVB_, offset, (UINT) description_.sizeofVertex());
	}
	inline
	void VertexBuffer::Release()
	{
		if (pVB_){
			pVB_->Release();
			pVB_ = NULL;
		}
	}
}	// namespace Vertex


#endif

