//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#pragma once

#include "CoreMinimal.h"

class ILevelEditor;
DECLARE_DELEGATE_OneParam(FOnPerspectiveTypeActive, bool)

class FSnappingHelperEdMode;

class FSnappingHelperModule : public IModuleInterface
{

public:

	bool bIsPerspectiveType {true};
	TSharedPtr<FUICommandList> SnappingHelperActions;
	FOnPerspectiveTypeActive OnPerspectiveTypeActive;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void BindCommands();
	FToolMenuEntry CreateToolMenuEntry();

	bool Is2DModeCheckBoxEnabled() const;
	ECheckBoxState Is2dModeChecked() const;
	void On2DModeStateChanged(ECheckBoxState InState);
	void On2DModeToggle();
	void ActivateEdMode2D() const;

	ECheckBoxState Is3dModeChecked() const;
	void On3DModeStateChanged(ECheckBoxState InState);
	void On3DModeToggle();
	void ActivateEdMode3D() const;

	void DeactivateEdMode();
	void PerspectiveTypeChanged(const bool bIsPerspectiveActive);
	FSnappingHelperEdMode* GetSnappingHelperEdMode() const;
	void OnLevelEditorCreated(TSharedPtr<ILevelEditor> InLevelEditor);
};
