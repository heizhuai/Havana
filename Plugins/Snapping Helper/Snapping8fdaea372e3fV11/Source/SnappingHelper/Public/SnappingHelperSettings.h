//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#pragma once

#include "Engine/DeveloperSettings.h"
#include "SnappingHelperSettings.generated.h"

UCLASS(Config = EditorPerProjectUserSettings)
class SNAPPINGHELPER_API USnappingHelperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(Config, EditDefaultsOnly, Category = "GeneralSettings")
	bool bCheckMaxVertexDistance = true;

	/* Vertices further than this distance will not be highlighted */
	UPROPERTY(Config, EditDefaultsOnly, Category = "GeneralSettings", meta = (EditCondition="bCheckMaxVertexDistance"))
	float MaxVertexDistanceFromCamera {20000.f};

	UPROPERTY(Config, EditDefaultsOnly, Category = "GeneralSettings")
	int32 HighlightedVerticesLimit {3500};

	UPROPERTY(Config, EditDefaultsOnly, Category = "GeneralSettings|Preview")
	bool bEnablePreviewActors = true;

	UPROPERTY(Config, EditDefaultsOnly, Category = "GeneralSettings|Preview", meta = (EditCondition="bEnablePreviewActors"))
	float PreviewLineThickness {3.f};

	UPROPERTY(Config, EditDefaultsOnly, Category = "GeneralSettings|Preview", meta = (EditCondition="bEnablePreviewActors"))
	int32 PreviewVerticesLimit {25000};

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Sprite Settings")
	TSoftObjectPtr<UTexture2D> SpriteOverride;

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Sprite Settings")
	FColor SnappingSpriteColor3D {FColor::Yellow};

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Sprite Settings")
	FColor SnappingSpriteColor2D {FColor::Green};

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Sprite Settings")
	float SpriteSize {10.f};

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Line Settings")
	FColor SnappingLineColor {FColor::Yellow};

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Line Settings")
	FColor SnappingWithDuplicateLineColor {FColor::Orange};

	UPROPERTY(Config, EditDefaultsOnly, Category = "ColorSettings|Line Settings")
	float SnappingLineThickness {5.f};

	static const USnappingHelperSettings* Get();

private:

	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FText GetSectionText() const override;
};
