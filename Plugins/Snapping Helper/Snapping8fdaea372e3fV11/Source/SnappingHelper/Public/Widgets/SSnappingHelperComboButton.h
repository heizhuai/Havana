//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SNAPPINGHELPER_API SSnappingHelperComboButton : public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS(SSnappingHelperComboButton)
		{
		}

		SLATE_EVENT(FOnCheckStateChanged, On3DCheckStateChanged)
		SLATE_EVENT(FOnCheckStateChanged, On2DCheckStateChanged)

		SLATE_ATTRIBUTE(ECheckBoxState, Is3DChecked)
		SLATE_ATTRIBUTE(ECheckBoxState, Is2DChecked)

		SLATE_ATTRIBUTE(bool, Is3DEnabled)
		SLATE_ATTRIBUTE(bool, Is2DEnabled)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
