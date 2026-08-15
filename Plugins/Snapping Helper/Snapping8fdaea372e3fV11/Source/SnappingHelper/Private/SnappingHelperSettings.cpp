//$ Copyright 2021, UsefulCode - All Rights Reserved $//

#include "SnappingHelperSettings.h"

namespace FSnappingHelperSettings
{
	const FName ProjectName(TEXT("Project"));
	const FName Category {TEXT("Plugins")};
	const FText SectionText {FText::FromString(TEXT("Snapping Helper"))};
}

const USnappingHelperSettings* USnappingHelperSettings::Get()
{
	return GetDefault<USnappingHelperSettings>();
}

FName USnappingHelperSettings::GetContainerName() const
{
	return FSnappingHelperSettings::ProjectName;
}

FName USnappingHelperSettings::GetCategoryName() const
{
	return FSnappingHelperSettings::Category;
}

FText USnappingHelperSettings::GetSectionText() const
{
	return FSnappingHelperSettings::SectionText;
}
