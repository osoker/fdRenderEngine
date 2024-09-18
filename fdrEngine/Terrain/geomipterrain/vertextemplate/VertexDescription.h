//===================================================================
/** \file  
* Filename :   VertexDescription.h
* Desc     :   A collection of ComponentDescription objects which between them describe the 
*			   format and layout of a D3D vertex.  These are wrapped in a class to hide
*			   the details from clients who don't care.  Speed and directness of access
*			   are not an issue (cf. the vertex members themselves).
* His      :   Windy create @2005-10-20 15:58:57
*/
//===================================================================
#ifndef VERTEX_VertexDescription_h
#define VERTEX_VertexDescription_h
#include <D3D9.h>
#include <vector>
#include <assert.h>
#include <iterator>

namespace Vertex
{
	class ComponentDescription;

	class VertexDescription
	{
	public:
		VertexDescription();

		IDirect3DVertexDeclaration9* createDecl(IDirect3DDevice9* pD3DDev) const;

		void pushComponent(const ComponentDescription*);
		const ComponentDescription& operator[](size_t i) const;
		size_t sizeofVertex() const;
		size_t nComponents() const;

		bool operator==(const VertexDescription&) const;

	private:
		typedef std::vector<const ComponentDescription*> ComponentVec;
		ComponentVec	components_;
		size_t			sizeofVertex_;
	};

	// It is possible to create declarations for multiple streams.  Each
	// VertexDescription in the vector is assumed to correspond to a stream (in
	// order).
	IDirect3DVertexDeclaration9* createMultiStreamDecl(IDirect3DDevice9* pD3DDev, const std::vector<const VertexDescription*>&);

	// Support the common two stream case as a convenient overload.
	IDirect3DVertexDeclaration9* createMultiStreamDecl(IDirect3DDevice9* pD3DDev, const VertexDescription&, const VertexDescription&);
}        // namespace Vertex

#endif

