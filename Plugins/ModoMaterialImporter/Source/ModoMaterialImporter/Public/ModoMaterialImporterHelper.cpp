/*
 *   Copyright 2016 The Foundry Visionmongers Ltd.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
 
#include "ModoMaterialImporterHelper.h"
#include "PackageName.h"

#include "ModoMaterialImporterLog.h"

using namespace ModoMaterial;

bool CommonHelper::GetValidePackageName(FString& PackageName)
{
	FText outReason;
	if (!FPackageName::IsValidLongPackageName(PackageName, false, &outReason))
	{

		UE_LOG(ModoMaterialImporter, Log, TEXT("PackageName is invalid because: %s"), *outReason.BuildSourceString());

		// try to reconver by replacing blank to underscodea
		for (int i = 0; i < PackageName.Len(); i++)
		{
			if (PackageName[i] == ' ')
			{
				PackageName[i] = '_';
			}
		}

		if (!FPackageName::IsValidLongPackageName(PackageName, false, &outReason))
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Auto Resolve failed because: %s"), *outReason.BuildSourceString());
			return false;
		}
		else
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Auto Resolve to: %s"), *PackageName);
			return true;
		}
	}

	return true;
}

// Strip invalid characters from the string, replacing spaces with underscores.

bool CommonHelper::RemoveInvalidCharacters(FString& name)
{
	FString validName = "";
	bool bChanged = false;

	for (int i = 0; i < name.Len(); i++)
	{
		if (isalnum(name[i]) || name[i] == '_' || name[i] == '-')
		{
			validName.AppendChar(name[i]);
		}
		else
		{
			if (name[i] == ' ')
			{
				validName.AppendChar('_');
			}
			bChanged = true;
		}
	}

	if (bChanged)
	{
		name = validName;
	}

	return bChanged;
}

bool CommonHelper::CorrectInvalidPath(FString& name)
{
	bool bChanged = false;
	FString validName = "";

	for (int i = 0; i < name.Len(); i++)
	{
		if (name[i] == '\\')
		{
			validName.AppendChar('/');
			bChanged = true;
		}
		else
		{
			validName.AppendChar(name[i]);
		}
	}

	if (bChanged)
	{
		name = validName;
	}

	return bChanged;
}

bool CommonHelper::RemoveMaterialSlotSuffix(FString& name)
{
	bool bChanged = false;

	if (name.Len() > 6)
	{
		int32 Offset = name.Find(TEXT("_SKIN"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (Offset != INDEX_NONE)
		{
			// Chop off the material name so we are left with the number in _SKINXX
			FString SkinXXNumber = name.Right(name.Len() - (Offset + 1)).RightChop(4);

			if (SkinXXNumber.IsNumeric())
			{
				// remove the '_skinXX' suffix from the material name
				name = name.Left(Offset);

				bChanged = true;
			}
		}
	}

	return bChanged;
}