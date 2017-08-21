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

#include "ModoMaterialImporterImportFactory.h"
#include "AssetRegistryModule.h"

#include "ModoMaterialImporterMaterialCreator.h"

//////////////////////////////////////////////////////////////////////////
// UPaperSpriteSheetImportFactory

const FString gTempPackageName = TEXT("/Game/ModoMaterialImporter_TempPackage");

UModoMaterialImporterImportFactory::UModoMaterialImporterImportFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), _meshPackage(NULL)
{
	bEditorImport = true;
	bEditAfterNew = false;
	bText = false;

	SupportedClass = UObject::StaticClass();

	Formats.Add(TEXT("xml;MODO Material file"));
}

FText UModoMaterialImporterImportFactory::GetToolTip() const
{
	return NSLOCTEXT("MODO Material Importer", "MODOMaterialImporterFactoryDescription", "Unreal material descriptions exported from MODO");
}

bool UModoMaterialImporterImportFactory::FactoryCanImport(const FString& Filename)
{
	FString path = ModoMaterial::MaterialCreator::GetContentBrowserPathRaw();

	// Avoid overwriting mesh objects that have same package name as XML file
	// We rename it, then rename it back
	FString baseName = FPaths::GetBaseFilename(Filename);
	FString PackageName = path + TEXT("/") + baseName;
	UPackage* existingPackage = FindPackage(NULL, *PackageName);

	if (existingPackage)
	{
		UPackage* tempPackage = FindPackage(NULL, *gTempPackageName);

		if (tempPackage && tempPackage->IsValidLowLevel())
		{
			tempPackage->MarkPendingKill();
			tempPackage->ConditionalBeginDestroy();
		}

		existingPackage->Rename(*gTempPackageName);
		_meshPackage = existingPackage;
	}
	else
	{
		_meshPackage = NULL;
	}

	return true;
}

UObject* UModoMaterialImporterImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FString	filename = Filename;
	FXmlFile* matXml = new FXmlFile(filename);

	// Find Path of associated resources
	int ilastslash = 0;
	for (ilastslash = filename.Len() - 1; ilastslash >= 0; --ilastslash)
	{
		if (filename[ilastslash] == TCHAR('/'))
		{
			break;
		}
	}

	FString path = filename.Left(ilastslash + 1);
	FString oriName = InParent->GetName();

	if (_meshPackage && _meshPackage->IsValidLowLevel())
	{
		// Name swap: Restore the mesh package name
		InParent->Rename(*(oriName + TEXT("M38O03D48O_B1234A9842K")));
		_meshPackage->Rename(*oriName);
		InParent->Rename(*gTempPackageName);
	}

	if (matXml->IsValid())
	{
		ModoMaterial::Assignment matAssignment;
		ModoMaterial::MaterialCreator::SetContentBrowserPathRaw(FPaths::GetPath(oriName));
		ModoMaterial::MaterialCreator::LoadMaterial(matXml, path, &matAssignment);
		matAssignment.ApplyToMeshes(ModoMaterial::MaterialCreator::GetContentBrowserPathRaw());

		return matAssignment.GetMaterial();
	}

	return NULL;
}