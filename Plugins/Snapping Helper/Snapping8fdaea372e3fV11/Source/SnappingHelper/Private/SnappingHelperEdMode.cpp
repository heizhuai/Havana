//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#include "SnappingHelperEdMode.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EngineUtils.h"
#include "VertexIterators.h"
#include "SnappingHelperSettings.h"
#include "ImageUtils.h"
#include "LevelEditorViewport.h"
#include "Selection.h"
#include "UnrealEdGlobals.h"
#include "Algo/Copy.h"
#include "Async/Async.h"
#include "Editor/UnrealEdEngine.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "FSnappingHelperEdMode"

namespace FSnappingHelperEdModeLocal
{
	const FString SnappingHelperName {TEXT("SnappingHelper")};
	const FString SpriteIcon {"/SpriteCircleIconStroke_16x.png"};
}

void FSelectedActorData::ResetPivotPointOffset() const
{
	if (IsValid(SelectedActor))
	{
		SelectedActor->Modify();
		SelectedActor->SetPivotOffset(OriginalPivotOffset);
	}
}

bool USelectionData::IsSelectedActorsEqual(const TArray<AActor*>& InActorsForCheck)
{
	if (InActorsForCheck.Num() != SelectedActorsData.Num())
	{
		return false;
	}

	for (int i = 0; i < SelectedActorsData.Num(); ++i)
	{
		const FSelectedActorData& SelectedActorData = SelectedActorsData[i];
		if (SelectedActorData != InActorsForCheck[i])
		{
			return false;
		}
	}

	return true;
}

FSelectedActorData* USelectionData::FindSelectedActorData(AActor* InActorForSelect)
{
	auto IsActorMatch = [InActorForSelect](const FSelectedActorData& InSelectedActorData)
	{
		return InSelectedActorData == InActorForSelect;
	};

	return SelectedActorsData.FindByPredicate(IsActorMatch);
}

TArray<AActor*> USelectionData::GetAllSelectedActors()
{
	TArray<AActor*> ActorsResult{};

	for (const FSelectedActorData& SelectedActorData : SelectedActorsData)
	{
		if (IsValid(SelectedActorData.SelectedActor))
		{
			ActorsResult.Add(SelectedActorData.SelectedActor);
		}
	}

	return ActorsResult;
}

void USelectionData::ModifySelectedActors()
{
	for (const FSelectedActorData& SelectedActorData : SelectedActorsData)
	{
		if (IsValid(SelectedActorData.SelectedActor))
		{
			SelectedActorData.SelectedActor->Modify();
		}
	}
}

void USelectionData::SetSelectedActorsData(const TArray<AActor*>& InActorsForAdd)
{
	SelectedActorsData.Empty();

	for (AActor* Actor : InActorsForAdd)
	{
		if (IsValid(Actor))
		{
			FSelectedActorData FreshSelectedActorData {Actor, Actor->GetPivotOffset()};
			SelectedActorsData.Add(FreshSelectedActorData);
		}
	}
}

bool USelectionData::IsSomeoneSelected() const
{
	return SelectedActorsData.Num() > 0;
}

bool USelectionData::IsSelectedVertexValid() const
{
	return SelectedVertex != FVector::ZeroVector;
}

void USelectionData::UpdateSelection()
{
	TArray<AActor*> NewSelectedActors;

	USelection* CurrentEditorSelection = GEditor->GetSelectedActors();
	CurrentEditorSelection->GetSelectedObjects<AActor>(NewSelectedActors);

	//	Select none
	if (NewSelectedActors.Num() == 0)
	{
		EraseSelection();
	}

	//	Selection not changed
	if (IsSelectedActorsEqual(NewSelectedActors))
	{
		
	}

	TArray<FSelectedActorData> ActorsDataForRemove {};

	for (FSelectedActorData& SelectedActorData : SelectedActorsData)
	{
		//	Actor deselected
		if (!NewSelectedActors.Contains(SelectedActorData.SelectedActor))
		{
			SelectedActorData.ResetPivotPointOffset();
			ActorsDataForRemove.Add(SelectedActorData);
		}
	}

	for (const FSelectedActorData& ActorDatForRemove : ActorsDataForRemove)
	{
		SelectedActorsData.Remove(ActorDatForRemove);
	}

	for (AActor* NewSelectedActor : NewSelectedActors)
	{
		if (!SelectedActorsData.Contains(NewSelectedActor))
		{
			FSelectedActorData FreshSelectedActorData {NewSelectedActor, NewSelectedActor->GetPivotOffset()};
			SelectedActorsData.Add(FreshSelectedActorData);

			//	Added new actor
			if (IsSelectedVertexValid())
			{
				SetTemporaryPivotPoint(NewSelectedActor, SelectedVertex);
			}
		}
	}
}

void USelectionData::EraseSelection()
{
	ResetPivotPointForAllSelectedActors();
	ResetSelectedVertex();
	ResetSelectedActorsData();
}

void USelectionData::UpdateInitialSelection()
{
	TArray<AActor*> NewSelectedActors;

	USelection* CurrentEditorSelection = GEditor->GetSelectedActors();
	CurrentEditorSelection->GetSelectedObjects<AActor>(NewSelectedActors);

	if (NewSelectedActors.Num() > 0)
	{
		SetSelectedActorsData(NewSelectedActors);
		SetSelectedVertex(GEditor->GetPivotLocation());
	}
}

void USelectionData::ResetPivotPointForAllSelectedActors()
{
	for (const FSelectedActorData& SelectedActorData : SelectedActorsData)
	{
		SelectedActorData.ResetPivotPointOffset();
	}

	GUnrealEd->UpdatePivotLocationForSelection(true);
}

void USelectionData::ResetSelectedVertex()
{
	SetSelectedVertex(FVector::ZeroVector);
}

void USelectionData::ResetSelectedActorsData()
{
	SelectedActorsData.Empty();
}

void USelectionData::SetTemporaryPivotPoint(AActor* InActor, const FVector& InPivotLocation)
{
	const FTransform NewPivotTransform = FTransform(InPivotLocation);

	InActor->SetPivotOffset(NewPivotTransform.GetRelativeTransform(InActor->GetActorTransform()).GetLocation());
	GUnrealEd->UpdatePivotLocationForSelection(true);
}

void USelectionData::SetTemporaryPivotPoint(TArray<AActor*> InActors, const FVector& InPivotLocation)
{
	for (AActor* Actor : InActors)
	{
		SetTemporaryPivotPoint(Actor, InPivotLocation);
	}
}

void USelectionData::SetPivotPointAsDefault(AActor* InActor, const FVector& InPivotLocation)
{
	FSelectedActorData* SelectedActorDataPtr = FindSelectedActorData(InActor);
	if ( SelectedActorDataPtr )
	{
		const FTransform NewPivotTransform = FTransform(InPivotLocation);
		const FVector NewPivotOffset = NewPivotTransform.GetRelativeTransform(InActor->GetActorTransform()).GetLocation();
		SelectedActorDataPtr->OriginalPivotOffset = NewPivotOffset;

		InActor->Modify();

		InActor->SetPivotOffset(NewPivotOffset);
		GUnrealEd->UpdatePivotLocationForSelection(true);
	}
}

void USelectionData::SetPivotPointAsDefault(TArray<AActor*> InActors, const FVector& InPivotLocation)
{
	for ( AActor* Actor : InActors )
	{
		SetPivotPointAsDefault(Actor, InPivotLocation);
	}
}

void USelectionData::SetSelectedVertex(const FVector& InLocation)
{
	if (InLocation == FVector::ZeroVector)
	{
		SelectedVertex = InLocation;
	}
	else
	{
		GEditor->BeginTransaction(LOCTEXT("ChangeSelectedVertex", "Change selected vertex"));

		Modify();
		SelectedVertex = InLocation;

		GEditor->EndTransaction();
	}
}

IMPLEMENT_HIT_PROXY(HSnappingHelperHitProxy, HHitProxy);

const FEditorModeID FSnappingHelperEdMode::EM_SnappingHelperEdModeId = TEXT("EM_SnappingHelperEdMode");

FSnappingHelperEdMode::FSnappingHelperEdMode()
{
	LoadSpriteIconTexture();
}

void FSnappingHelperEdMode::Enter()
{
	FEdMode::Enter();
	bIsModeActive = true;

	if (!IsValid(CurrentSelection))
	{
		CurrentSelection = NewObject<USelectionData>();
		CurrentSelection->AddToRoot();
		CurrentSelection->SetFlags(RF_Transactional);
	}

	if (!OnCollectingDataFinished.IsBoundToObject(this))
	{
		OnCollectingDataFinished.BindRaw(this, &FSnappingHelperEdMode::CollectingDataFinished);
	}

	GetWorld()->GetTimerManager().SetTimer(CollectVerticesTimerHandle, FTimerDelegate::CreateRaw(this, &FSnappingHelperEdMode::AsyncCollectingDataFromWorld), 0.2f, false);
	GetWorld()->GetTimerManager().SetTimer(InvalidateHitProxiesTimerHandle, FTimerDelegate::CreateRaw(this, &FSnappingHelperEdMode::InvalidateHitProxies), 0.3f, true);

	CurrentSelection->UpdateInitialSelection();
}

void FSnappingHelperEdMode::Exit()
{
	bIsModeActive = false;

	if (CollectVerticesTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CollectVerticesTimerHandle);
	}

	if (InvalidateHitProxiesTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InvalidateHitProxiesTimerHandle);
	}

	CurrentSelection->EraseSelection();

	FEdMode::Exit();
}

void FSnappingHelperEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);

	const bool bCurrentDroppingPreview{FLevelEditorViewportClient::GetDropPreviewActors().Num() > 0};

	if (bPreviousDroppingPreview && !bCurrentDroppingPreview)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateRaw(this, &FSnappingHelperEdMode::EraseDroppingPreview));
	}

	bPreviousDroppingPreview = bCurrentDroppingPreview;
}

void FSnappingHelperEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	if (!IsViewportActive(Viewport) || bPreviousDroppingPreview)
	{
		return;
	}

	CollectCursorData(View);
	CollectPressedKeysData(Viewport);
	CollectViewportData(Viewport);

	EdModeView = View;
	ProjectionMatrix = View->ViewMatrices.GetViewProjectionMatrix();

	const auto EditorViewportClient = static_cast<FEditorViewportClient*>(Viewport->GetClient());
	if (EditorViewportClient)
	{
		const bool bIsPerspectiveView{EditorViewportClient->IsPerspective()};

		const USnappingHelperSettings* Settings{USnappingHelperSettings::Get()};
		const int32 HighlightedVerticesLimit = Settings->HighlightedVerticesLimit;
		const FVector EditorCameraLocation = EditorViewportClient->GetViewLocation();
		const FColor SpriteColor{bIs3DMode ? Settings->SnappingSpriteColor3D : Settings->SnappingSpriteColor2D};

		const UTexture2D* SpriteTexture{SpriteIcon};
		if (const UTexture2D* SpriteTextureOverride{Settings->SpriteOverride.Get()})
		{
			SpriteTexture = SpriteTextureOverride;
		}

		if (Settings->bEnablePreviewActors
			&& LastCapturedVerticesData.Num() > 0
			&& (IsKeysForSnappingWithDuplicatePressed() || IsKeysForSnappingPressed())
			&& CurrentSelection->IsSomeoneSelected())
		{
			const FCapturedVertexData& ClosetsToCursorVertex = LastCapturedVerticesData[0];

			const float& ClosetsToCursorVertexDistance = ClosetsToCursorVertex.DistanceToCursor;
			if (ClosetsToCursorVertexDistance < 50.f)
			{
				const FVector& VertexInWorldPosition = ClosetsToCursorVertex.VertexInWorldPosition;
				const float LinePreviewScale{static_cast<float>(View->WorldToScreen(VertexInWorldPosition).W * (1.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]))};
				DrawPreview(VertexInWorldPosition, LinePreviewScale, PDI);
			}
		}

		for (int i = 0; i < LastCapturedVerticesData.Num(); ++i)
		{
			if (i < HighlightedVerticesLimit)
			{
				const FCapturedVertexData& VertexData{LastCapturedVerticesData[i]};
				const float Scale{static_cast<float>(View->WorldToScreen(VertexData.VertexInWorldPosition).W * (1.0f / View->UnscaledViewRect.Width() / View->ViewMatrices.GetProjectionMatrix().M[0][0]))};

				FVector SpriteLocation{VertexData.VertexInWorldPosition};

				if (bIsPerspectiveView)
				{
					//	Draw the sprite with a slight offset towards the camera to avoid gaps in the geometry
					SpriteLocation += (EditorCameraLocation - VertexData.VertexInWorldPosition).GetSafeNormal() * 3;
				}

				PDI->SetHitProxy(new HSnappingHelperHitProxy(VertexData.VertexInWorldPosition, VertexData.VertexOwnerActor));

				PDI->DrawSprite(
					SpriteLocation,
					Settings->SpriteSize * Scale,
					Settings->SpriteSize * Scale,
					SpriteTexture->GetResource(),
					SpriteColor,
					SDPG_World,
					0.0, 0.0, 0.0, 0.0);
			}
		}
	}

	//	TODO: If a message is displayed on the screen, for example "LIGHTING NEEDS TO BE REBUILT", then during the execution of Viewport->InvalidateHitProxy() the sprites begin to flicker, I could not solve this problem
	//	TODO: So this crutch with an message on the screen completely removes the problem (I don't know why)
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, "This message must be of a certain length", true, FVector2D::ZeroVector);
}

void FSnappingHelperEdMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);

	if (!IsViewportActive(Viewport) || bPreviousDroppingPreview)
	{
		return;
	}

	DPIScale = Canvas->GetDPIScale();

	if ((IsKeysForSnappingPressed() || IsKeysForSnappingWithDuplicatePressed()) && CurrentSelection->IsSomeoneSelected() && CurrentSelection->IsSelectedVertexValid())
	{
		const USnappingHelperSettings* Settings{USnappingHelperSettings::Get()};
		const FColor LineColor{IsKeysForSnappingWithDuplicatePressed() ? Settings->SnappingWithDuplicateLineColor : Settings->SnappingLineColor};

		FVector2D SelectedVertexOnScreen;
		View->WorldToPixel(CurrentSelection->SelectedVertex, SelectedVertexOnScreen);

		FCanvasLineItem LineItem;
		LineItem.LineThickness = Settings->SnappingLineThickness / DPIScale;
		LineItem.SetColor(LineColor);

		LineItem.Draw(Canvas, MouseOnScreenPosition, SelectedVertexOnScreen / DPIScale);
	}
}

bool FSnappingHelperEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	bool bHandled {false};

	if (HitProxy)
	{
		const bool bIsLeftMouseButtonClick = Click.GetKey() == EKeys::LeftMouseButton;
		const bool bIsRightMouseButtonClick = Click.GetKey() == EKeys::RightMouseButton;

		if (HitProxy->IsA(HActor::StaticGetType()))
		{
			const auto ActorHitProxy = static_cast<HActor*>(HitProxy);

			if (bIsCtrlKeyDown || bIsShiftKeyDown)
			{
				return bHandled;
			}

			//	Just click one actor
			CurrentSelection->ResetPivotPointForAllSelectedActors();
			CurrentSelection->SetSelectedVertex(ActorHitProxy->Actor->GetTransform().TransformPosition(ActorHitProxy->Actor->GetPivotOffset()));
		}

		if (bIsLeftMouseButtonClick && (IsKeysForSnappingWithDuplicatePressed() || IsKeysForSnappingPressed()) && CurrentSelection->IsSomeoneSelected() && HitProxy->IsA(HSnappingHelperHitProxy::StaticGetType()))
		{
			bHandled = true;
			const auto VertexHitProxy = static_cast<HSnappingHelperHitProxy*>(HitProxy);
			const FVector HitProxyLocation {VertexHitProxy->RefVector};

			GEditor->BeginTransaction(LOCTEXT("SnapToVertex", "Snap to vertex"));

			if (IsKeysForSnappingWithDuplicatePressed())
			{
				GUnrealEd->edactDuplicateSelected(GWorld->GetCurrentLevel(), false);
			}

			for (const FSelectedActorData& SelectedActorData : CurrentSelection->SelectedActorsData)
			{
				AActor* SelectedActor = SelectedActorData.SelectedActor;
				if (IsValid(SelectedActor))
				{
					SelectedActor->Modify();
					FVector Offset {SelectedActor->GetActorLocation() - CurrentSelection->SelectedVertex};
					FVector DesiredLocation {HitProxyLocation + Offset};

					if (!bIs3DMode)
					{
						DesiredLocation = CalculateLocationIgnoringOneAxis(InViewportClient, SelectedActor->GetActorLocation(), DesiredLocation);
					}

					SelectedActor->SetActorLocation(DesiredLocation);
				}
			}

			FVector PivotLocationAfterSnapping {HitProxyLocation};

			if (!bIs3DMode)
			{
				PivotLocationAfterSnapping = CalculateLocationIgnoringOneAxis(InViewportClient, CurrentSelection->SelectedVertex, PivotLocationAfterSnapping);
			}

			CurrentSelection->SetSelectedVertex(PivotLocationAfterSnapping);
			GUnrealEd->UpdatePivotLocationForSelection(true);

			GEditor->EndTransaction();
		}
		else if (bIsLeftMouseButtonClick && HitProxy->IsA(HSnappingHelperHitProxy::StaticGetType()))
		{
			bHandled = true;

			const auto VertexHitProxy = static_cast<HSnappingHelperHitProxy*>(HitProxy);
			const FVector& HitProxyLocation {VertexHitProxy->RefVector};
			AActor* HitProxyActor {VertexHitProxy->RefActor};

			const bool bHooked = IsTryingSelectHitProxyWithManyVertices(HitProxyLocation);
			if ( bHooked )
			{
				CurrentSelection->SetSelectedVertex(HitProxyLocation);
				CurrentSelection->SetTemporaryPivotPoint(CurrentSelection->GetAllSelectedActors(), HitProxyLocation);
			}
			else
			{
				if (CurrentSelection->IsSomeoneSelected())
				{
					if (CurrentSelection->SelectedActorsData.Contains(HitProxyActor))
					{
						CurrentSelection->SetSelectedVertex(HitProxyLocation);
						CurrentSelection->SetTemporaryPivotPoint(CurrentSelection->GetAllSelectedActors(), HitProxyLocation);
					}
					else if (IsValid(HitProxyActor))
					{
						GEditor->BeginTransaction(LOCTEXT("SelectByVertex", "Select actor by vertex"));

						GEditor->SelectNone(false, true);
						CurrentSelection->SetSelectedVertex(HitProxyLocation);
						HitProxyActor->Modify();
						GEditor->SelectActor(HitProxyActor, true, true);

						GEditor->EndTransaction();
					}
				}
				else if (IsValid(HitProxyActor))
				{
					GEditor->BeginTransaction(LOCTEXT("SelectByVertex", "Select actor by vertex"));

					CurrentSelection->SetSelectedVertex(HitProxyLocation);
					HitProxyActor->Modify();
					GEditor->SelectActor(HitProxyActor, true, true);

					GEditor->EndTransaction();
				}
			}
		}
		else if ( bIsRightMouseButtonClick && HitProxy->IsA(HSnappingHelperHitProxy::StaticGetType()) )
		{
			const auto VertexHitProxy = static_cast<HSnappingHelperHitProxy*>(HitProxy);
			const FVector& HitProxyLocation {VertexHitProxy->RefVector};

			const TSharedPtr<SWidget> MenuWidget = CreateHitProxyContextMenu(HitProxyLocation);
			if ( MenuWidget.IsValid() )
			{
				bHandled = true;

				FSlateApplication::Get().PushMenu
				(
					FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef(),
					FWidgetPath(),
					MenuWidget.ToSharedRef(),
					FSlateApplication::Get().GetCursorPos(),
					FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
				);
			}
		}
	}

	return bHandled;
}

TSharedPtr<SWidget> FSnappingHelperEdMode::CreateHitProxyContextMenu(const FVector& InHitProxyLocation) const
{
	FMenuBuilder MenuBuilder(true, nullptr);

	const TAttribute<FText> SetDefaultPivotButtonLabel = FText::FromString("Set Pivot Point As Default");

	auto SetPivotPointAsDefaultForSelectedActors = [Selection = this->CurrentSelection, InHitProxyLocation]()
	{
		GEditor->BeginTransaction(LOCTEXT("SelectByVertex", "Set pivot point as default"));

		Selection->SetSelectedVertex(InHitProxyLocation);
		Selection->SetPivotPointAsDefault(Selection->GetAllSelectedActors(), Selection->SelectedVertex);

		GEditor->EndTransaction();
	};

	MenuBuilder.AddMenuEntry(SetDefaultPivotButtonLabel, {}, FSlateIcon(), FUIAction(FExecuteAction::CreateLambda(SetPivotPointAsDefaultForSelectedActors)));
	return MenuBuilder.MakeWidget();
}

void FSnappingHelperEdMode::DrawPreview(const FVector& InVertexUnderCursor, const float InScale, FPrimitiveDrawInterface* InPDI)
{
	const FVector& SelectedVertexValue = CurrentSelection->SelectedVertex;

	if (!InVertexUnderCursor.Equals(SelectedVertexValue, 10.f))
	{
		const USnappingHelperSettings* Settings{USnappingHelperSettings::Get()};
		const FColor LineColor {IsKeysForSnappingWithDuplicatePressed() ? Settings->SnappingWithDuplicateLineColor : Settings->SnappingLineColor};

		int32 DrawnVerticesCounter = 0;

		for (const FSelectedActorData& SelectedActorsData : CurrentSelection->SelectedActorsData)
		{
			const AActor* SelectedActor = SelectedActorsData.SelectedActor;
			if ( !IsValid(SelectedActor) )
			{
				continue;
			}

			TInlineComponentArray<UStaticMeshComponent*> PrimitiveComponents {};
			SelectedActor->GetComponents(PrimitiveComponents);

			for (const UStaticMeshComponent* PrimitiveComponent : PrimitiveComponents)
			{
				FVector Offset = PrimitiveComponent->GetComponentLocation() - CurrentSelection->SelectedVertex;
				FVector DesiredLocation = InVertexUnderCursor + Offset;

				FTransform TargetTransform {PrimitiveComponent->GetComponentRotation(), DesiredLocation, PrimitiveComponent->GetComponentScale()};
				DrawStaticMeshWire(InPDI, PrimitiveComponent, TargetTransform, LineColor, InScale, DrawnVerticesCounter);
			}
		}
	}
}

bool FSnappingHelperEdMode::IsTryingSelectHitProxyWithManyVertices(const FVector& InHitProxyLocation)
{
	bool bHooked = false;
	if (CurrentSelection->IsSomeoneSelected())
	{
		TSet<AActor*> ActorsWithVertexInHitProxyLocation {};
		for (FCapturedVertexData& CapturedVertexData : LastCapturedVerticesData)
		{
			if (FVector::PointsAreNear(InHitProxyLocation, CapturedVertexData.VertexInWorldPosition, 1.f))
			{
				ActorsWithVertexInHitProxyLocation.Add(CapturedVertexData.VertexOwnerActor);
			}
		}

		for (AActor* ActorWithVertex : ActorsWithVertexInHitProxyLocation)
		{
			const FSelectedActorData* SelectedActorData = CurrentSelection->FindSelectedActorData(ActorWithVertex);
			if (SelectedActorData)
			{
				bHooked = true;
				break;
			}
		}
	}

	return bHooked;
}

bool FSnappingHelperEdMode::IsViewportActive(const FViewport* InViewport)
{
	return GEditor->GetActiveViewport() == InViewport;
}

bool FSnappingHelperEdMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	if (InViewportClient->GetCurrentWidgetAxis() == EAxisList::None)
	{
		return false;
	}

	if (!InDrag.IsZero())
	{
		CurrentSelection->SetSelectedVertex(GetWidgetLocation() + InDrag);

		if (bIsAltKeyDown)
		{
			return false;
		}

		for (const FSelectedActorData& SelectedActorData : CurrentSelection->SelectedActorsData)
		{
			SelectedActorData.SelectedActor->AddActorWorldOffset(InDrag);
		}

		if (!bIsMouseMove)
		{
			for (const FSelectedActorData& SelectedActorData : CurrentSelection->SelectedActorsData)
			{
				CurrentSelection->SetTemporaryPivotPoint(SelectedActorData.SelectedActor, CurrentSelection->SelectedVertex);
			}
		}
		return true;
	}

	return false;
}

void FSnappingHelperEdMode::ActorSelectionChangeNotify()
{
	CurrentSelection->UpdateSelection();
}

bool FSnappingHelperEdMode::ShouldDrawWidget() const
{
	if (CurrentSelection->IsSomeoneSelected())
	{
		return true;
	}

	return false;
}

FVector FSnappingHelperEdMode::GetWidgetLocation() const
{
	if (CurrentSelection->IsSomeoneSelected() && CurrentSelection->IsSelectedVertexValid())
	{
		return CurrentSelection->SelectedVertex;
	}

	return FEdMode::GetWidgetLocation();
}

bool FSnappingHelperEdMode::UsesToolkits() const
{
	return false;
}

FVector FSnappingHelperEdMode::CalculateLocationIgnoringOneAxis(const FEditorViewportClient* InViewportClient, const FVector& InCurrentLocation, const FVector& InDesiredLocation)
{
	// Top and bottom. Ignore Z
	if (InViewportClient->GetViewportType() == LVT_OrthoXY || InViewportClient->GetViewportType() == LVT_OrthoNegativeXY)
	{
		return FVector(InDesiredLocation.X, InDesiredLocation.Y, InCurrentLocation.Z);
	}

	// Front and back. Ignore Y
	if (InViewportClient->GetViewportType() == LVT_OrthoXZ || InViewportClient->GetViewportType() == LVT_OrthoNegativeXZ)
	{
		return FVector(InDesiredLocation.X, InCurrentLocation.Y, InDesiredLocation.Z);
	}

	// Left and right. Ignore X
	if (InViewportClient->GetViewportType() == LVT_OrthoYZ || InViewportClient->GetViewportType() == LVT_OrthoNegativeYZ)
	{
		return FVector(InCurrentLocation.X, InDesiredLocation.Y, InDesiredLocation.Z);
	}

	return InDesiredLocation;
}

bool FSnappingHelperEdMode::IsKeysForSnappingPressed()
{
#if PLATFORM_MAC
	return bIsAltKeyDown;
#endif

	return bIsCtrlKeyDown && bIsAltKeyDown;
}

bool FSnappingHelperEdMode::IsKeysForSnappingWithDuplicatePressed()
{
#if PLATFORM_MAC
	return bIsAltKeyDown && bIsShiftKeyDown;
#endif

	return bIsCtrlKeyDown && bIsShiftKeyDown;
}

FVector2D FSnappingHelperEdMode::GetMouseVector2D()
{
	return FVector2D{GEditor->GetActiveViewport()->GetMouseX() / DPIScale, GEditor->GetActiveViewport()->GetMouseY() / DPIScale};
}

void FSnappingHelperEdMode::CollectCursorData(const FSceneView* InSceneView)
{
	bIsMouseMove = GetMouseVector2D() != MouseOnScreenPosition;
	MouseOnScreenPosition = GetMouseVector2D();

	FVector MouseWorldPosition;
	FVector CameraDirection;
	InSceneView->DeprojectFVector2D(MouseOnScreenPosition, MouseWorldPosition, CameraDirection);

	FCollisionQueryParams TraceQueryParams;
	TraceQueryParams.bTraceComplex = true;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, MouseWorldPosition, MouseWorldPosition + (CameraDirection * 10000000), ECC_Visibility, TraceQueryParams);

	MouseInWorld = HitResult.ImpactPoint;
}

void FSnappingHelperEdMode::CollectPressedKeysData(const FViewport* InViewport)
{
	bIsCtrlKeyDown = InViewport->KeyState(EKeys::LeftControl) || InViewport->KeyState(EKeys::RightControl);
	bIsShiftKeyDown = InViewport->KeyState(EKeys::LeftShift) || InViewport->KeyState(EKeys::RightShift);
	bIsAltKeyDown = InViewport->KeyState(EKeys::LeftAlt) || InViewport->KeyState(EKeys::RightAlt);
}

void FSnappingHelperEdMode::CollectViewportData(const FViewport* InViewport)
{
	const FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(InViewport->GetClient());
	if (EditorViewportClient)
	{
		WidgetMode = EditorViewportClient->GetWidgetMode();
		ViewDirection = EditorViewportClient->GetViewRotation();
		ViewLocation = EditorViewportClient->GetViewLocation();
	}
}

void FSnappingHelperEdMode::CollectingDataFinished()
{
	LastCapturedVerticesData = CapturedVerticesData;
	bDataCollectionInProgress = false;

	if (bIsModeActive)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateRaw(this, &FSnappingHelperEdMode::AsyncCollectingDataFromWorld));
	}
}

void FSnappingHelperEdMode::AsyncCollectingDataFromWorld()
{
	if (bDataCollectionInProgress)
	{
		return;
	}

	if (!EdModeView)
	{
		return;
	}

	AllActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);

	TWeakPtr<FSnappingHelperEdMode> WeakThisPtr {SharedThis(this)};
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakThisPtr]()
	{
		FSnappingHelperEdMode* ThisBackgroundThread {WeakThisPtr.Pin().Get()};
		if (!ThisBackgroundThread)
		{
			return;
		}

		ThisBackgroundThread->bDataCollectionInProgress = true;
		ThisBackgroundThread->CapturedVerticesData.Empty();

		//	Filter visible actors 
		auto ActorWasRendered = [](const AActor* InActor)
		{
			return InActor && InActor->WasRecentlyRendered();
		};

		TArray<AActor*> ActorsOnScreen{};
		Algo::CopyIf(ThisBackgroundThread->AllActors, ActorsOnScreen, ActorWasRendered);

		const FVector& MouseInWorldPosition {ThisBackgroundThread->MouseInWorld};

		//	Sort actors by distance to mouse location in world
		Algo::Sort(ActorsOnScreen, [MouseInWorldPosition](const AActor* InActorA, const AActor* InActorB)
		{
			if (InActorA && InActorB)
			{
				return FVector::DistSquared(MouseInWorldPosition, InActorA->GetActorLocation()) < FVector::DistSquared(MouseInWorldPosition, InActorB->GetActorLocation());
			}
			return false;
		});

		const bool bNeedDistanceCheck = USnappingHelperSettings::Get()->bCheckMaxVertexDistance;
		const float DistanceTestSqr = FMath::Square(USnappingHelperSettings::Get()->MaxVertexDistanceFromCamera);

		for (int i = 0; i < ActorsOnScreen.Num(); ++i)
		{
			//	Take one hundred closest actors and collect data on their vertices
			if (i < 100)
			{
				const AActor* CurrentActor = ActorsOnScreen[i];

				TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
				CurrentActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

				for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
				{
					if (IsValid(PrimitiveComponent))
					{
						TSharedPtr<FSnappingHelperIterators::FVertexIterator> VertexGetter = FSnappingHelperIterators::MakeVertexIterator(PrimitiveComponent);
						if (VertexGetter.IsValid())
						{
							FSnappingHelperIterators::FVertexIterator& VertexGetterRef = *VertexGetter;
							for (; VertexGetterRef; ++VertexGetterRef)
							{
								if (!WeakThisPtr.IsValid())
								{
									break;
								}

								//	Depth test
								if ( bNeedDistanceCheck && (ThisBackgroundThread->ViewLocation - VertexGetterRef.Position()).SizeSquared() > DistanceTestSqr )
								{
									continue;
								}

								FVector2D VertexOnScreen {};
								ThisBackgroundThread->EdModeView->ScreenToPixel(
									ThisBackgroundThread->ProjectionMatrix.TransformFVector4(FVector4(VertexGetterRef.Position(), 1)), VertexOnScreen);
								VertexOnScreen /= ThisBackgroundThread->DPIScale;

								FCapturedVertexData CapturedVertexData;
								CapturedVertexData.VertexOwnerActor = PrimitiveComponent->GetOwner();
								CapturedVertexData.VertexInWorldPosition = VertexGetterRef.Position();
								CapturedVertexData.VertexOnScreenPosition = VertexOnScreen;
								CapturedVertexData.DistanceToCursor = FVector2D::DistSquared(ThisBackgroundThread->MouseOnScreenPosition, VertexOnScreen);

								ThisBackgroundThread->CapturedVerticesData.Add(CapturedVertexData);
							}
						}
					}
				}
			}
		}

		Algo::Sort(ThisBackgroundThread->CapturedVerticesData);

		AsyncTask(ENamedThreads::GameThread, [WeakThisPtr]()
		{
			const FSnappingHelperEdMode* ThisGameThread {WeakThisPtr.Pin().Get()};
			if (ThisGameThread)
			{
				ThisGameThread->OnCollectingDataFinished.ExecuteIfBound();
			}
		});
	});
}

void FSnappingHelperEdMode::LoadSpriteIconTexture()
{
	const auto RelativePath = IPluginManager::Get().FindPlugin(FSnappingHelperEdModeLocal::SnappingHelperName)->GetBaseDir() / TEXT("Resources");
	FString FullPath = FPaths::ConvertRelativePathToFull(RelativePath);
	FullPath += FSnappingHelperEdModeLocal::SpriteIcon;

	UTexture2D* LoadedTexture = FImageUtils::ImportFileAsTexture2D(FullPath);

	check(LoadedTexture);
	if (LoadedTexture)
	{
		LoadedTexture->AddToRoot();
		SpriteIcon = LoadedTexture;
	}
}

void FSnappingHelperEdMode::InvalidateHitProxies()
{
	if (bIsMouseMove)
	{
		GEditor->GetActiveViewport()->InvalidateHitProxy();
	}
}

void FSnappingHelperEdMode::EraseDroppingPreview()
{
	bPreviousDroppingPreview = false;
}

void FSnappingHelperEdMode::DrawStaticMeshWire(FPrimitiveDrawInterface* PDI, const UStaticMeshComponent* InComponent, const FTransform& InTargetTransform, const FColor Color, const float InScale, int32& InOutVerticesCounter) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SnappingHelper_DrawStaticMeshWire);

	const int32 PreviewVerticesLimit = USnappingHelperSettings::Get()->PreviewVerticesLimit;
	if ( InOutVerticesCounter >= PreviewVerticesLimit )
	{
		return;
	}

	UStaticMesh* ElseStaticMesh = InComponent->GetStaticMesh();
	if (!IsValid(ElseStaticMesh))
	{
		return;
	}

	const float PreviewLineThickness = InScale * USnappingHelperSettings::Get()->PreviewLineThickness;
	FStaticMeshLODResources& LOD = ElseStaticMesh->GetRenderData()->LODResources[0];
	FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
	uint32 VerticesTotal = LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
	FVector Positions[3];

	for (int i = 0; i < Indices.Num(); ++i)
	{
		if ( ++InOutVerticesCounter >= PreviewVerticesLimit )
		{
			return;
		}

		uint32 Index1 = Indices[i];
		uint32 Index2 = Indices[i + 1];
		uint32 Index3 = Indices[i + 2];

		if (Index1 > VerticesTotal || Index2 > VerticesTotal || Index3 > VerticesTotal)
		{
			continue;
		}

		{
			TRACE_CPUPROFILER_EVENT_SCOPE(SnappingHelper_TransformVertices);

			Positions[0] = UE::Math::TVector<double> {LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(Index1)};
			Positions[1] = UE::Math::TVector<double> {LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(Index2)};
			Positions[2] = UE::Math::TVector<double> {LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(Index3)};

			Positions[0] = InTargetTransform.TransformPosition(Positions[0]);
			Positions[1] = InTargetTransform.TransformPosition(Positions[1]);
			Positions[2] = InTargetTransform.TransformPosition(Positions[2]);
		}

		{
			TRACE_CPUPROFILER_EVENT_SCOPE(SnappingHelper_DrawLines);

			PDI->DrawLine(Positions[0], Positions[1], Color, SDPG_World, PreviewLineThickness);
			PDI->DrawLine(Positions[1], Positions[2], Color, SDPG_World, PreviewLineThickness);
			PDI->DrawLine(Positions[2], Positions[0], Color, SDPG_World, PreviewLineThickness);
		}
	}
}

#undef LOCTEXT_NAMESPACE
