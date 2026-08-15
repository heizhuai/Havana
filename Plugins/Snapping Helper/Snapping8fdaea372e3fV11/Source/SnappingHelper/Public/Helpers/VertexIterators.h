//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#pragma once

#include "CoreMinimal.h"
#include "Rendering/SkeletalMeshRenderData.h"


//	Everything copied from here \UnrealEd\Private\VertexSnapping.cpp
namespace FSnappingHelperIterators
{
	/**
 * Base class for an interator that iterates through the vertices on a component
 */
	class FVertexIterator
		{
		public:
		virtual ~FVertexIterator(){};

		/** Advances to the next vertex */
		void operator++()
		{
			Advance();
		}

		/** @return True if there are more vertices on the component */
		explicit operator bool() const
		{
			return HasMoreVertices();
		}

		/**
		* @return The position in world space of the current vertex
		*/
		virtual FVector Position() const = 0;

		/**
		* @return The position in world space of the current vertex normal
		*/
		virtual FVector Normal() const = 0;

		protected:
		/**
		* @return True if there are more vertices on the component
		*/
		virtual bool HasMoreVertices() const = 0;

		/**
		* Advances to the next vertex
		*/
		virtual void Advance() = 0;
		};

	/**
	* Iterates through the vertices of a static mesh
	*/
	class FStaticMeshVertexIterator : public FVertexIterator
		{
		public:
		FStaticMeshVertexIterator(UStaticMeshComponent* SMC);

		/** FVertexIterator interface */
		virtual FVector Position() const override;
		virtual FVector Normal() const override;

		protected:
		virtual void Advance() override;
		virtual bool HasMoreVertices() const override;

		private:
		/** Component To World Inverse Transpose matrix */
		FMatrix ComponentToWorldIT;
		/** Component containing the mesh that we are getting vertices from */
		UStaticMeshComponent* StaticMeshComponent;
		/** The static meshes position vertex buffer */
		FPositionVertexBuffer& PositionBuffer;
		/** The static meshes vertex buffer for normals */
		FStaticMeshVertexBuffer& VertexBuffer;
		/** Current vertex index */
		uint32 CurrentVertexIndex;
		};

	/**
	* Vertex iterator for brush components
	*/
	class FBrushVertexIterator : public FVertexIterator
		{
		public:
		FBrushVertexIterator(UBrushComponent* InBrushComponent);

		/** FVertexIterator interface */
		virtual FVector Position() const override;
		/** FVertexIterator interface */
		virtual FVector Normal() const override;

		protected:
		virtual void Advance() override;
		virtual bool HasMoreVertices() const override;

		private:
		/** The brush component getting vertices from */
		UBrushComponent* BrushComponent;
		/** All brush component vertices */
		TArray<FVector> Vertices;
		/** Current vertex index the iterator is on */
		uint32 CurrentVertexIndex;
		/** The number of vertices to iterate through */
		uint32 NumVertices;
		};

	/**
	* Iterates through the vertices on a component
	*/
	class FSkeletalMeshVertexIterator : public FVertexIterator
		{
		public:
		FSkeletalMeshVertexIterator(USkinnedMeshComponent* InSkinnedMeshComp);

		/** FVertexIterator interface */
		virtual FVector Position() const override;
		virtual FVector Normal() const override;

		protected:
		virtual void Advance() override;
		virtual bool HasMoreVertices() const override;

		private:
		/** Component To World inverse transpose matrix */
		FMatrix ComponentToWorldIT;
		/** The component getting vertices from */
		USkinnedMeshComponent* SkinnedMeshComponent;
		/** Skeletal mesh render data */
		FSkeletalMeshLODRenderData& LODData;
		/** Current Soft vertex index the iterator is on */
		uint32 VertexIndex;
		};

	/**
	* Makes a vertex iterator from the specified component
	*/
	TSharedPtr<FVertexIterator> MakeVertexIterator(UPrimitiveComponent* Component);
}

