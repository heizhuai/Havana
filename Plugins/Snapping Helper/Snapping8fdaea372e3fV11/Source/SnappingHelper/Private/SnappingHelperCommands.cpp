//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#include "FSnappingHelperCommands.h"

#define LOCTEXT_NAMESPACE "FSnappingHelperModule"

void FSnappingHelperCommands::RegisterCommands()
{
#if PLATFORM_WINDOWS || PLATFORM_LINUX
	UI_COMMAND(SnappingHelper3D, "Snapping Helper 3D", "On/Off Snapping Helper 3D (Alt + Shift + 3)", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::Three, EModifierKey::Alt | EModifierKey::Shift));
	UI_COMMAND(SnappingHelper2D, "Snapping Helper 2D", "On/Off Snapping Helper 2D (Alt + Shift + 2)", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::Two, EModifierKey::Alt | EModifierKey::Shift));
#endif

#if PLATFORM_MAC
	UI_COMMAND(SnappingHelper3D, "Snapping Helper 3D", "On/Off Snapping Helper 3D (Option + Shift + 3)", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::Three, EModifierKey::Alt | EModifierKey::Shift));
	UI_COMMAND(SnappingHelper2D, "Snapping Helper 2D", "On/Off Snapping Helper 2D (Option + Shift + 2)", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::Two, EModifierKey::Alt | EModifierKey::Shift));
#endif
}

#undef LOCTEXT_NAMESPACE