//===================================================================
/** \file  
* Filename :   ComponentDescription.h
* Desc     :   A meta-data object which describes a set of member variables within a
*			   vertex class.  Each component corresponds to an input register on the
*			   vertex shader.
* His      :   Windy create @2005-10-20 16:09:08
*/
//===================================================================
#ifndef GSTATE_ComponentDescription_h
#define GSTATE_ComponentDescription_h

#include <string>
#include <D3D9.h>
#include <assert.h>
namespace Vertex
{
	class ComponentDescription
	{
	public:
		ComponentDescription(const std::string& desc, BYTE ty, BYTE sem,BYTE index = 0);

		const std::string& name() const;
		size_t size() const;
		BYTE type() const;
		BYTE semantic() const;

		void setVertexElement(D3DVERTEXELEMENT9& dst, BYTE streamIndex, BYTE fieldOffset) const;

	private:
		// We can deduce the size from the type, but store it anyway.  
		// Memory isn't an issue with these.
		const std::string	name_;				// e.g. "position"
		const size_t		size_;				// e.g. 12
		const BYTE			type_;				// e.g. FLOAT3
		const BYTE			semantic_;			// e.g. POSITION
		const BYTE			usage_;				// e.g. 0
	};

	// For debug sanity checking this returns the size of an item in the enum.
	size_t sizeofVertexFieldType(BYTE type);
}        // namespace Vertex

#endif
