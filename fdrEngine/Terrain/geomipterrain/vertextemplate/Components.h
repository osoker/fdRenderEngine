//===================================================================
/** \file  
* Filename :   Components.h
* Desc     :   	A collection of vertex components each of which define one vertex member
*		variable (or a set of related variables).  These are assembled into 
*		vertices using the composer template class.
*		Clients of the vertex framework are free to define their own exotic components.
*		Real examples (in the Warhammer Online engine) include physics properties,
*		morphing deltas and all manner of indices and weights, plus traditional
*		vertex members in weird compressed forms.

*		In practice, an engine framework should provide a much richer selection of
*		vertex components.  Indeed, it is desirable for the framework to anticipate
*		as many reusable vertex components as possible.  Good examples are: further 
*		texture co-ordinate variants; bone weights and indices for skinning; DOT3
*		basis vectors; further packed/quantised versions; etc.

*		By contrast, the aim of this file is to concisely illustrate the technique
*		with a small representative sample of components.

* His      :   Windy create @2005-10-20 16:07:29
*/
//===================================================================

#ifndef VERTEX_COMPONENTS_H
#define VERTEX_COMPONENTS_H

#include <D3DX9.h>

namespace Vertex
{
	class ComponentDescription;
	

	struct Position
	{
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
			void set(const D3DXVECTOR3&);
		//	void set(const osVec3D&);
			void set(const float& x, const float& y , const float& z);
			float x_, y_, z_;
		} position_;

	protected:
		static const ComponentDescription& description();
	};

	struct Normal
	{
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
		//	void set(const osVec3D&);
			void set(const D3DXVECTOR3&);
			float x_, y_, z_;
		} normal_;

	protected:
		static const ComponentDescription& description();
	};

	struct Tangent
	{
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
			void set(const D3DXVECTOR3&);
			float x_, y_, z_;
		} tangent_;

	protected:
		static const ComponentDescription& description();
	};

	struct TexCoords0
	{
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
			void set(const D3DXVECTOR2&);
			void set(const float &,const float &);
		//	void set(const osVec2D &);
	//		void set(const Vec2D&);
			float u_, v_;
		} texCoords0_;

	protected:
		static const ComponentDescription& description();
	};
	struct TexCoords1
	{
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
			void set(const D3DXVECTOR2&);
			void set(const float &,const float &);
	//		void set(const osVec2D &);
			//		void set(const Vec2D&);
			float u_, v_;
		} texCoords1_;

	protected:
		static const ComponentDescription& description();
	};

	struct PackedColour
	{
		struct SubFields
		{
			// Add a helper method if you have a Color class.
			// void set(const Color&);
			void set(const D3DCOLOR&);
			union
			{
				struct
				{		
					unsigned char b_, g_, r_, a_;
				};
				unsigned int  packed_;
			};
		} colour_;

	protected:
		static const ComponentDescription& description();
	};
	struct BlendWeight
	{
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
			void set(const D3DXVECTOR4&);
			float w0_, w1_, w2_, w3_;
		} blendweight_;

	protected:
		static const ComponentDescription& description();
	};
	struct BlendIndices
	{
		
		struct SubFields
		{
			// Add your own helper methods if you have other vector classes.
			void set(const D3DCOLOR&);
			union
			{
				struct 
				{
					unsigned char b0_, b1_, b2_, b3_;
				};
				unsigned int  packed_;
			};
		} blendindices_;

	protected:
		static const ComponentDescription& description();
	};
}

#endif

