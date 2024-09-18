//===================================================================
/** \file  
* Filename :   ComponentDescription.cpp
* Desc     :   
* His      :   Windy create @2005-12-22 11:11:14
*/
//===================================================================
# include "stdafx.h"
#include "ComponentDescription.h"

namespace Vertex
{
	ComponentDescription::ComponentDescription(const std::string& n, BYTE t, BYTE s,BYTE i):
		name_(n),
		type_(t),
		size_(sizeofVertexFieldType(t)),
		semantic_(s),
		usage_(i)
	{
	}

	const std::string& ComponentDescription::name() const
	{
		return name_;
	}

	size_t ComponentDescription::size() const
	{
		return size_;
	}

	BYTE ComponentDescription::type() const
	{
		return type_;
	}

	BYTE ComponentDescription::semantic() const
	{
		return semantic_;
	}

	size_t sizeofVertexFieldType(BYTE type)
	{
		switch (type)
		{
			case D3DDECLTYPE_FLOAT1:   return 4;
			case D3DDECLTYPE_FLOAT2:   return 8;
			case D3DDECLTYPE_FLOAT3:   return 12;
			case D3DDECLTYPE_FLOAT4:   return 16;
			case D3DDECLTYPE_D3DCOLOR: return 4;
			case D3DDECLTYPE_SHORT2:   return 4;
			case D3DDECLTYPE_SHORT4:   return 8;
			case D3DDECLTYPE_UBYTE4:   return 4;
			default: assert(0);        return 0;
		}
	}

	void ComponentDescription::setVertexElement(D3DVERTEXELEMENT9& dst, BYTE streamIndex, BYTE fieldOffset) const
	{
		dst.Stream = streamIndex;
		dst.Offset = fieldOffset;
		dst.Type = type_;
		dst.Method = D3DDECLMETHOD_DEFAULT;
		dst.Usage = semantic_;
		dst.UsageIndex = usage_;
	}
}        // namespace Vertex
