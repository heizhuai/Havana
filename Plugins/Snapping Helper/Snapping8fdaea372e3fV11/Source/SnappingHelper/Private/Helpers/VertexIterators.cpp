//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#include "VertexIterators.h"

#include "Components/BrushComponent.h"
#include "Engine/Polys.h"

FSnappingHelperIterators::FStaticMeshVertexIterator::FStaticMeshVertexIterator(UStaticMeshComponent* SMC)
	: ComponentToWorldIT(SMC->GetComponentTransform().ToInverseMatrixWithScale().GetTransposed())
	, StaticMeshComponent(SMC)
	, PositionBuffer(SMC->GetStaticMesh()->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer)
	, VertexBuffer(SMC->GetStaticMesh()->GetRenderData()->LODResources[0].VertexBuffers.StaticMeshVertexBuffer),
	CurrentVertexIndex(0)
{}

FVector FSnappingHelperIterators::FStaticMeshVertexIterator::Position() const
{
	return StaticMeshComponent->GetComponentTransform().TransformPosition(UE::Math::TVector<double> {PositionBuffer.VertexPosition(CurrentVertexIndex)});
}

FVector FSnappingHelperIterators::FStaticMeshVertexIterator::Normal() const
{
	return ComponentToWorldIT.TransformVector(static_cast<FVector4> (VertexBuffer.VertexTangentZ(CurrentVertexIndex)));
}

void FSnappingHelperIterators::FStaticMeshVertexIterator::Advance()
{
	++CurrentVertexIndex;
}

bool FSnappingHelperIterators::FStaticMeshVertexIterator::HasMoreVertices() const
{
	return CurrentVertexIndex < PositionBuffer.GetNumVertices();
}

FSnappingHelperIterators::FBrushVertexIterator::FBrushVertexIterator(UBrushComponent* InBrushComponent)
	: BrushComponent(InBrushComponent),
	CurrentVertexIndex(0)
{
	// Build up a list of vertices
	const UModel* Model = BrushComponent->Brush;
	for (int32 PolyIndex = 0; PolyIndex < Model->Polys->Element.Num(); ++PolyIndex)
	{
		FPoly& Poly = Model->Polys->Element[PolyIndex];
		for (int32 VertexIndex = 0; VertexIndex < Poly.Vertices.Num(); ++VertexIndex)
		{
			Vertices.Add(UE::Math::TVector<double> {Poly.Vertices[VertexIndex]});
		}
	}
}

FVector FSnappingHelperIterators::FBrushVertexIterator::Position() const
{
	return BrushComponent->GetComponentTransform().TransformPosition(Vertices[CurrentVertexIndex]);
}

FVector FSnappingHelperIterators::FBrushVertexIterator::Normal() const
{
	return FVector::ZeroVector;
}

void FSnappingHelperIterators::FBrushVertexIterator::Advance()
{
	++CurrentVertexIndex;
}

bool FSnappingHelperIterators::FBrushVertexIterator::HasMoreVertices() const
{
	return Vertices.IsValidIndex(CurrentVertexIndex);
}

FSnappingHelperIterators::FSkeletalMeshVertexIterator::FSkeletalMeshVertexIterator(USkinnedMeshComponent* InSkinnedMeshComp)
	: ComponentToWorldIT(InSkinnedMeshComp->GetComponentTransform().ToInverseMatrixWithScale().GetTransposed()),
	SkinnedMeshComponent(InSkinnedMeshComp),
	LODData(InSkinnedMeshComp->GetSkeletalMeshRenderData()->LODRenderData[0]),
	VertexIndex(0)
{}

FVector FSnappingHelperIterators::FSkeletalMeshVertexIterator::Position() const
{
	const FVector VertPos = UE::Math::TVector<double> {LODData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex)};
	return SkinnedMeshComponent->GetComponentTransform().TransformPosition(VertPos);
}

FVector FSnappingHelperIterators::FSkeletalMeshVertexIterator::Normal() const
{
	const FPackedNormal TangentZ = LODData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
	const FVector VertNormal = TangentZ.ToFVector();
	return ComponentToWorldIT.TransformVector(VertNormal);
}

void FSnappingHelperIterators::FSkeletalMeshVertexIterator::Advance()
{
	VertexIndex++;
}

bool FSnappingHelperIterators::FSkeletalMeshVertexIterator::HasMoreVertices() const
{
	return VertexIndex < LODData.StaticVertexBuffers.PositionVertexBuffer.GetNumVertices();
}

TSharedPtr<FSnappingHelperIterators::FVertexIterator> FSnappingHelperIterators::MakeVertexIterator(UPrimitiveComponent* Component)
{
	UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Component);
	if (SMC && SMC->GetStaticMesh())
	{
		return MakeShareable(new FStaticMeshVertexIterator(SMC));
	}

	UBrushComponent* BrushComponent = Cast<UBrushComponent>(Component);
	if (BrushComponent && BrushComponent->Brush)
	{
		return MakeShareable(new FBrushVertexIterator(BrushComponent));
	}

	USkinnedMeshComponent* SkinnedComponent = Cast<USkinnedMeshComponent>(Component);
	if (SkinnedComponent && SkinnedComponent->GetSkinnedAsset() && SkinnedComponent->MeshObject)
	{
		return MakeShareable(new FSkeletalMeshVertexIterator(SkinnedComponent));
	}

	return nullptr;
}
