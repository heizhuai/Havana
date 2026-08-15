//$ Copyright 2021, UsefulCode - All Rights Reserved $//

using UnrealBuildTool;

public class SnappingHelper : ModuleRules
{
	public SnappingHelper(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateIncludePaths.AddRange(
			new string[]
			{
				"SnappingHelper/Public/Helpers",
				"SnappingHelper/Public/Widgets",
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"ApplicationCore",
				"DeveloperSettings",
				"Projects",
				"EditorStyle",
				"EditorFramework",
				"ToolMenus",
			}
		);
	}
}