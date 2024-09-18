//===================================================================
/** \file  
* Filename :   VertexDescription.cpp
* Desc     :   
* His      :   Windy create @2005-10-20 16:01:00
*/
//===================================================================
# include "stdafx.h"
#include "VertexDescription.h"
#include "ComponentDescription.h"

namespace Vertex
{
	VertexDescription::VertexDescription(): 
		sizeofVertex_(0)
	{
	}

	IDirect3DVertexDeclaration9* VertexDescription::createDecl(IDirect3DDevice9* pD3DDev) const
	{
		std::vector<const VertexDescription*> streams(1, this);
		return createMultiStreamDecl(pD3DDev, streams);
	}

	IDirect3DVertexDeclaration9* createMultiStreamDecl(IDirect3DDevice9* pD3DDev, const std::vector<const VertexDescription*>& streams)
	{
		// Under DX9, a shader can only have 16 inputs.  Reserve for this many
		// plus the end marker.  (It'll work anyway if the number is exceeded.)
		std::vector<D3DVERTEXELEMENT9> elements;
		elements.reserve(17);

		size_t nComponents = 0;
		for (BYTE streamIndex = 0; streamIndex != streams.size(); ++streamIndex)
		{
			const VertexDescription& vtxDesc = *(streams[streamIndex]);
			BYTE fieldOffset = 0;
			for (size_t i=0; i!=vtxDesc.nComponents(); ++i)
			{
				elements.push_back(D3DVERTEXELEMENT9());
				vtxDesc[i].setVertexElement(elements.back(), streamIndex, fieldOffset);

				// This should be a safe cast because the number of vertex components
				// is highly constrained by the vertex shader inputs.
				assert(vtxDesc[i].size() < 256);
				fieldOffset += static_cast<BYTE>(vtxDesc[i].size());
				++nComponents;
			}
		}

		// You have to use D3DDECL_END to initialise a static.  It's designed
		// only to be used for array initializer lists.
		static const D3DVERTEXELEMENT9 theD3dEnd = D3DDECL_END();
		elements[nComponents] = theD3dEnd;

		// This isn't guaranteed to work according to the C++ standard.  The
		// vector does not have to store its members in contiguous memory.  If
		// this worries you, use a C-style array instead.
		const D3DVERTEXELEMENT9* pElements = &(elements[0]);

		IDirect3DVertexDeclaration9* pResult = NULL;
		pD3DDev->CreateVertexDeclaration(pElements, &pResult);
		return pResult;
	}

	IDirect3DVertexDeclaration9* createMultiStreamDecl(IDirect3DDevice9* pD3DDev, const VertexDescription& d1, const VertexDescription& d2)
	{
		std::vector<const VertexDescription*> streams;
		streams.reserve(2);
		streams.push_back(&d1);
		streams.push_back(&d2);
		return createMultiStreamDecl(pD3DDev, streams);
	}

	const ComponentDescription& VertexDescription::operator[](size_t i) const
	{
		assert(i < nComponents());
		return *(components_[i]);
	}

	size_t VertexDescription::sizeofVertex() const
	{
		return sizeofVertex_;
	}

	size_t VertexDescription::nComponents() const
	{
		return components_.size();
	}

	bool VertexDescription::operator==(const VertexDescription& that) const
	{
		// We expect that objects are built statically by the template meta
		// programming.  Thus we can compare pointers.
		return (this == &that);
	}

	void VertexDescription::pushComponent(const ComponentDescription* pC)
	{
		components_.push_back(pC);
		sizeofVertex_ += pC->size();
	}
}        // End of namespace
