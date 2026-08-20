//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#include "SnappingHelperStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FSnappingHelperStyle::StyleInstance {nullptr};

namespace FSnappingHelperStyleLocal
{
	const FName StyleSetName(TEXT("SnappingHelperStyle"));
}

void FSnappingHelperStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSnappingHelperStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

void FSnappingHelperStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSnappingHelperStyle::Get()
{
	return *StyleInstance;
}

FName FSnappingHelperStyle::GetStyleSetName()
{
	return FSnappingHelperStyleLocal::StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);

TSharedRef<FSlateStyleSet> FSnappingHelperStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("SnappingHelperStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SnappingHelper")->GetBaseDir() / TEXT("Resources"));

	Style->Set("SnappingHelper.SnappingHelper3D", new IMAGE_BRUSH(TEXT("CheckBox3d_UE5_16x"), Icon16x16));
	Style->Set("SnappingHelper.SnappingHelper2D", new IMAGE_BRUSH(TEXT("CheckBox2d_UE5_16x"), Icon16x16));

	return Style;
}

#undef IMAGE_BRUSH