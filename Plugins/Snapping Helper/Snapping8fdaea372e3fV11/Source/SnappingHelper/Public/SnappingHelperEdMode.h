//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "SnappingHelperEdMode.generated.h"

DECLARE_DELEGATE(FOnCollectingDataFinished);

struct FCapturedVertexData
{
	bool operator <(const FCapturedVertexData& InOptions) const
	{
		return DistanceToCursor < InOptions.DistanceToCursor;
	}

	bool operator>(const FCapturedVertexData& InOptions) const
	{
		return !operator<(InOptions);
	}

	FVector VertexInWorldPosition {};
	FVector2D VertexOnScreenPosition {};
	AActor* VertexOwnerActor {nullptr};
	float DistanceToCursor {0.f};
};

USTRUCT()
struct FSelectedActorData
{
	GENERATED_BODY()

	bool operator == (const AActor* InOtherActor) const
	{
		return SelectedActor == InOtherActor;
	}

	bool operator != (const AActor* InOtherActor) const
	{
		return SelectedActor != InOtherActor;
	}

	bool operator == (const FSelectedActorData& InOtherSelectedActorData) const
	{
		return SelectedActor == InOtherSelectedActorData.SelectedActor;
	}

	UPROPERTY()
	AActor* SelectedActor {};

	UPROPERTY()
	FVector OriginalPivotOffset {FVector::ZeroVector};

	void ResetPivotPointOffset() const;
};

UCLASS()
class USelectionData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FSelectedActorData> SelectedActorsData {};

	UPROPERTY()
	FVector SelectedVertex {FVector::ZeroVector};

	bool IsSelectedActorsEqual(const TArray<AActor*>& InActorsForCheck);
	FSelectedActorData* FindSelectedActorData(AActor* InActorForSelect);
	TArray<AActor*> GetAllSelectedActors();
	void ModifySelectedActors();
	void SetSelectedActorsData(const TArray<AActor*>& InActorsForAdd);
	bool IsSomeoneSelected() const;
	bool IsSelectedVertexValid() const;
	void UpdateSelection();
	void EraseSelection();
	void UpdateInitialSelection();
	void ResetPivotPointForAllSelectedActors();
	void SetTemporaryPivotPoint(AActor* InActor, const FVector& InPivotLocation);
	void SetTemporaryPivotPoint(TArray<AActor*> InActors, const FVector& InPivotLocation);
	void SetPivotPointAsDefault(AActor* InActor, const FVector& InPivotLocation);
	void SetPivotPointAsDefault(TArray<AActor*> InActors, const FVector& InPivotLocation);
	void SetSelectedVertex(const FVector& InLocation);
	void ResetSelectedVertex();
	void ResetSelectedActorsData();
};

struct HSnappingHelperHitProxy : public HHitProxy
{
	DECLARE_HIT_PROXY()

	HSnappingHelperHitProxy(FVector InPosition, AActor* InActor)
		: HHitProxy(HPP_UI), RefVector(InPosition), RefActor(InActor){}

	FVector RefVector;
	AActor* RefActor;
};

class FSnappingHelperEdMode : public FEdMode
{

public:

	const static FEditorModeID EM_SnappingHelperEdModeId;

	bool bIsModeActive{false};
	TArray<FCapturedVertexData> LastCapturedVerticesData {};
	TArray<FCapturedVertexData> CapturedVerticesData {};
	bool bIsMouseMove{false};
	bool bIsCtrlKeyDown{false};
	bool bIsAltKeyDown{false};
	bool bIsShiftKeyDown{false};
	bool bPreviousDroppingPreview{false};
	USelectionData* CurrentSelection {nullptr};
	FVector2D MouseOnScreenPosition{};
	FVector MouseInWorld{};
	float DPIScale {1.f};
	UTexture2D* SpriteIcon {nullptr};
	FTimerHandle CollectVerticesTimerHandle{};
	FTimerHandle InvalidateHitProxiesTimerHandle{};
	const FSceneView* EdModeView;
	bool bDataCollectionInProgress{false};
	FOnCollectingDataFinished OnCollectingDataFinished{};
	TArray<AActor*> AllActors {};
	bool bIs3DMode {true};
	UE::Widget::EWidgetMode WidgetMode {};
	FRotator ViewDirection {};
	FVector ViewLocation {};
	FMatrix ProjectionMatrix {};

	FSnappingHelperEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual void ActorSelectionChangeNotify() override;
	virtual bool ShouldDrawWidget() const override;
	virtual FVector GetWidgetLocation() const override;
	virtual bool UsesToolkits() const override;
	// End of FEdMode interface

	bool IsKeysForSnappingPressed();
	bool IsKeysForSnappingWithDuplicatePressed();
	FVector CalculateLocationIgnoringOneAxis(const FEditorViewportClient* InViewportClient, const FVector& InCurrentLocation, const FVector& InDesiredLocation);
	FVector2D GetMouseVector2D();
	void CollectCursorData(const FSceneView* InSceneView);
	void CollectPressedKeysData(const FViewport* InViewport);
	void CollectViewportData(const FViewport* InViewport);
	void CollectingDataFinished();
	void AsyncCollectingDataFromWorld();
	void LoadSpriteIconTexture();
	void InvalidateHitProxies();
	void EraseDroppingPreview();
	void DrawStaticMeshWire(FPrimitiveDrawInterface* PDI, const UStaticMeshComponent* InComponent, const FTransform& InTargetTransform, const FColor Color, const float InScale, int32& InOutVerticesCounter) const;
	TSharedPtr<SWidget> CreateHitProxyContextMenu(const FVector& InHitProxyLocation) const;
	void DrawPreview(const FVector& InVertexUnderCursor, const float InScale, FPrimitiveDrawInterface* InPDI);
	bool IsTryingSelectHitProxyWithManyVertices(const FVector& InHitProxyLocation);
	static bool IsViewportActive(const FViewport* InViewport);
};