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
 
#include "ModoMaterialImporterPrivatePCH.h"
#include "ModoMaterialImporterHelper.h"
#include "ModoMaterialImporterLog.h"

#include "PackageName.h"

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
				PackageName[i] = '_';
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