//===================================================================
/** \file  
* Filename :   Components.cpp
* Desc     :   
* His      :   Windy create @2005-10-20 16:09:03
*/
//===================================================================
# include "stdafx.h"
#include "Components.h"
#include "ComponentDescription.h"
#include "VertexDescription.h"
//#include "../../../common/com_include.h"
//#include "../math/mathAll.h"


namespace Vertex
{
	// static 
	const ComponentDescription& Position::description()
	{
		static ComponentDescription f("3D position", D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION);
		return f;
	}

	// static 
	const ComponentDescription& Normal::description()
	{
		static ComponentDescription f("normal", D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL);
		return f;
	}

	// static 
	const ComponentDescription& Tangent::description()
	{
		static ComponentDescription f("tangent", D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_TANGENT);
		return f;
	}

	// static 
	const ComponentDescription& TexCoords0::description()
	{
		static ComponentDescription f("2D texture coords, set 0", D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD);
		return f;
	}

	// static 
	const ComponentDescription& TexCoords1::description()
	{
		static ComponentDescription f("2D texture coords, set 1", D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD,1);
		return f;
	}

	// static 
	const ComponentDescription& PackedColour::description()
	{
		static ComponentDescription f("colour", D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR);
		return f;
	}
	// static 
	const ComponentDescription& BlendWeight::description()
	{
		static ComponentDescription f("blendweight", D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_BLENDWEIGHT);
		return f;
	}
	// static 
	const ComponentDescription& BlendIndices::description()
	{
		//!因为Geforce3不支持UBYTE4,所以使用COLOR代替.
		static ComponentDescription f("blendindices", D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_BLENDINDICES);
		return f;
	}

	void Position::SubFields::set(const D3DXVECTOR3& v)
	{
		x_ = v.x;
		y_ = v.y;
		z_ = v.z;
	}
	/*void Position::SubFields::set(const D3DXVECTOR3& v)
	{
		x_ = v.x;
		y_ = v.y;
		z_ = v.z;
	}*/
	void Position::SubFields::set(const float& x, const float& y, const float& z)
	{
		x_ = x;
		y_ = y;
		z_ = z;
	}

	void Normal::SubFields::set(const D3DXVECTOR3& v)
	{
		x_ = v.x;
		y_ = v.y;
		z_ = v.z;
	}
	/*void Normal::SubFields::set(const osVec3D& v)
	{
		x_ = v.x;
		y_ = v.y;
		z_ = v.z;
	}*/
	void TexCoords0::SubFields::set(const D3DXVECTOR2& v)
	{
		u_ = v.x;
		v_ = v.y;
	}
	void TexCoords0::SubFields::set(const float &u,const float &v)
	{
		u_ = u;
		v_ = v;
	}
	/*void TexCoords0::SubFields::set(const osVec2D & v)
	{
		u_ = v.x;
		v_ = v.y;
	}*/
	void TexCoords1::SubFields::set(const D3DXVECTOR2& v)
	{
		u_ = v.x;
		v_ = v.y;
	}
	void TexCoords1::SubFields::set(const float &u,const float &v)
	{
		u_ = u;
		v_ = v;
	}
	/*void TexCoords1::SubFields::set(const osVec2D & v)
	{
		u_ = v.x;
		v_ = v.y;
	}*/
	/*void TexCoords0::SubFields::set(const Vec2D& v)
	{
		u_ = v.x;
		v_ = v.y;
	}*/
	void PackedColour::SubFields::set(const D3DCOLOR& c)
	{
		memcpy((void *)&packed_,(void *)&c,4);
	}
	void BlendWeight::SubFields::set(const D3DXVECTOR4& v)
	{
		w0_ = v.x;
		w1_ = v.y;
		w2_ = v.z;
		w3_ = v.w;
	}
	void BlendIndices::SubFields::set(const D3DCOLOR& c)
	{
		memcpy((void *)&packed_,(void *)&c,4);
	}
}        // namespace Vertex
