//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#pragma once

#include "CoreMinimal.h"
#include "SnappingHelperStyle.h"

class SNAPPINGHELPER_API FSnappingHelperCommands : public TCommands<FSnappingHelperCommands>
{
public:
	FSnappingHelperCommands()
	: TCommands<FSnappingHelperCommands>(TEXT("SnappingHelper"), NSLOCTEXT("Contexts", "SnappingHelper", "SnappingHelper Plugin"), NAME_None, FSnappingHelperStyle::GetStyleSetName()){}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> SnappingHelper3D;
	TSharedPtr<FUICommandInfo> SnappingHelper2D;
};
