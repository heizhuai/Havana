//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#include "SSnappingHelperComboButton.h"
#include "SnappingHelperStyle.h"
#include "FSnappingHelperCommands.h"

#include "EditorModeManager.h"
#include "SlateOptMacros.h"
#include "Styling/StarshipCoreStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSnappingHelperComboButton::Construct(const FArguments& InArgs)
{
	const FSlateIcon& Icon3D = FSnappingHelperCommands::Get().SnappingHelper3D->GetIcon();
	const FSlateIcon& Icon2D = FSnappingHelperCommands::Get().SnappingHelper2D->GetIcon();

	const FCheckBoxStyle& CheckBoxStyleLeft = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Property.ToggleButton.Start");
	const FCheckBoxStyle& CheckBoxStyleRight = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Property.ToggleButton.End");

	TSharedPtr<SCheckBox> Snapping3DCheckBox;
	{
		Snapping3DCheckBox = SNew(SCheckBox)
			.Cursor(EMouseCursor::Default)
			.Padding(FMargin(4.0f))
			.Style(&CheckBoxStyleLeft)
			.IsChecked(InArgs._Is3DChecked)
			.OnCheckStateChanged(InArgs._On3DCheckStateChanged)
			.ToolTipText(FSnappingHelperCommands::Get().SnappingHelper3D->GetDescription())
			.Content()
		[
			SNew(SBox)
			.WidthOverride(16)
			.HeightOverride(16)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(Icon3D.GetIcon())
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	TSharedPtr<SCheckBox> Snapping2DCheckBox;
	{
		Snapping2DCheckBox = SNew(SCheckBox)
			.Cursor(EMouseCursor::Default)
			.Padding(FMargin(4.0f))
			.Style(&CheckBoxStyleRight)
			.IsEnabled(InArgs._Is2DEnabled)
			.IsChecked(InArgs._Is2DChecked)
			.OnCheckStateChanged(InArgs._On2DCheckStateChanged)
			.ToolTipText(FSnappingHelperCommands::Get().SnappingHelper2D->GetDescription())
			.Content()
		[
			SNew(SBox)
			.WidthOverride(16)
			.HeightOverride(16)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(Icon2D.GetIcon())
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			Snapping3DCheckBox->AsShared()
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			Snapping2DCheckBox->AsShared()
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
