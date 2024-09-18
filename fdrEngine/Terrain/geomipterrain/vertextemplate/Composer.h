//===================================================================
/** \file  
* Filename :   Composer.h
* Desc     :   Template classes which are used to construct vertices in a template meta 
*			   programming style.  This file creates classes which would be called Vertex
*			   were it not for the template stuff.
* His      :   Windy create @2005-10-20 16:05:00
*/
//===================================================================
#ifndef VERTEX_COMPOSER_H
#define VERTEX_COMPOSER_H

#include "ComponentDescription.h"
#include "VertexDescription.h"
#include <assert.h>
//#include <boost/type_traits.hpp>
//#include <boost/static_assert.hpp>

namespace Vertex
{
	class EndList	
	{
	protected:
		// This class is a stop marker for the recursive inheritance.  The
		// composer class requires that these methods exist.  The recursion
		// requires that they do nothing.
		static void packingCheck()									{}
		static void pushComponentDescription(VertexDescription*)	{}
	};

	// A vertex which multiply inherits from another vertex and an additional
	// vertex component.  It thus aggregates using inheritance.
	template <class C, class V = EndList>
	class Composer : public C, public V
	{
	public:
		// Note: you will get a chain of inherited fields methods.  The one 
		// in the most derived class hides the others in the base classes.
		// (Hopefully, the base class ones won't get instantiated.)
		static const VertexDescription& vertexDescription();
		static void packingCheck();

	protected:
		static void pushComponentDescription(VertexDescription*);
	};

	template <class C, class V>
	void Composer<C, V>::packingCheck()
	{
		// Verify that the meta-data for component C is self-consistent.
		assert(sizeof(C) == sizeofVertexFieldType(
			C::description().type()));
		assert(sizeof(C) == C::description().size());

		// D3D vertex shaders require that all vertex buffer components are
		// multiples of 4 bytes.
		assert(sizeof(C) % 4 == 0);

		// This will fail if your compiler does not produce a non-zero size
		// when EndList is used as a sub-object.
		//assert(boost::is_empty<EndList>::value);

		// Verify that the aggregation by multiple inheritance (or object 
		// packing in general) doesn't pad out the size of this class object.
		//assert((boost::is_same<V, EndList>::value || 
		//	(sizeof(Composer<C,V>) == sizeof(V) + sizeof(C))));
		//assert((boost::is_same<V, EndList>::value == 
		//	(sizeof(Composer<C,V>) == sizeof(C))));

		// Recursively apply the same above checks to the base class.
		// EndList needs to implement nothing to end the recursion.
		V::packingCheck();
	}

	template <class C, class V>
	void Composer<C, V>::pushComponentDescription(VertexDescription* pVec)
	{
		// Push the type of the component that we add.
		pVec->pushComponent(&C::description());

		// Recursively descend through the base class functions.
		// EndList must implement nothing to end the recursion.
		V::pushComponentDescription(pVec);
	}

	template <class C, class V>
	const VertexDescription& Composer<C, V>::vertexDescription()
	{
		// Call the check method to ensure that it gets instantiated.
		packingCheck();

		// First time around, recursively descend through the template classes,
		// pushing a type for each one.
		static VertexDescription result;
		if (result.nComponents() == 0)
			Composer<C, V>::pushComponentDescription(&result);

		return result;
	}
}        // End of namespace

#endif

