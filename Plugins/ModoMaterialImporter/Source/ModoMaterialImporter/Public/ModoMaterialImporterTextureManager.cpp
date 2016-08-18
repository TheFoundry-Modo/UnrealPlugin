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
#include "ModoMaterialImporterTextureManager.h"
#include "ModoMaterialImporterHelper.h"
#include "ModoMaterialImporterLog.h"

using namespace ModoMaterial;

TextureManager	*gManager = NULL;

TextureManager* TextureManager::Instance()
{
	TextureManager *manager = gManager;
	if (NULL != manager)
		return (manager);

	gManager = new TextureManager;

	return (gManager);
}

TextureManager::TextureManager()
{
	UE_LOG(ModoMaterialImporter, Log, TEXT("Creating TextureManager"));
}

TextureManager::~TextureManager()
{

}

UTexture* TextureManager::LoadTexture(const FString& TextureFilename, const FString& path, const FString& rootPath, bool isSRGB, TextureCompressionSettings compSetting)
{
	FString BaseName = FPaths::GetBaseFilename(TextureFilename);

	// Remove invalid characters from the texture name.
	CommonHelper::RemoveInvalidCharacters(BaseName);

	FString PackageName = TEXT("/Game/") + BaseName;

	UE_LOG(ModoMaterialImporter, Log, TEXT("Creating texture: %s"), *PackageName);

	// Deal with invalid long name issue
	if (!CommonHelper::GetValidePackageName(PackageName))
		return NULL;

	// Find if the texture exists anywhere in the content, in any package
	UTexture* tex = FindObjectFast<UTexture>(NULL, *BaseName, false, true);
	if (tex)
	{
		// If the texture doesn't match current setting, we need update it
		bool changed = false;
		if (tex->SRGB != isSRGB)
		{
			tex->SRGB = isSRGB;
			changed = true;
		}

		if (tex->CompressionSettings != compSetting)
		{
			tex->CompressionSettings = compSetting;
			changed = true;
		}

		if (changed)
			tex->PostEditChange();

		return tex;
	}
	
	// Texture not found, create a new one by binary loading it and packing
	// it into a new asset package in the root content folder 
	bool isRelativePath = FPaths::IsRelative(TextureFilename);
	FString Filename;

	if (isRelativePath)
	{
		if (path.IsEmpty())
			Filename = FPaths::GameContentDir() + TEXT("../Data/") + TextureFilename;
		else
			Filename = path + TextureFilename;

		const int32 FileSize = IFileManager::Get().FileSize(*Filename);

		if (IFileManager::Get().FileSize(*Filename) == INDEX_NONE) 
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("File '%s' not found in relative path but marked as relative"), *Filename);
			Filename = rootPath + TextureFilename;
		}
	}
	else
		Filename = TextureFilename;

	const int32 FileSize = IFileManager::Get().FileSize(*Filename);
	bool bValidFileSize = true;
	bool bLoadedFile = false;

	if (FileSize == INDEX_NONE)
	{
		UE_LOG(ModoMaterialImporter, Log, TEXT("File '%s' does not exist"), *Filename);
		bValidFileSize = false;
	}

	TArray<uint8> RawData;
	if (bValidFileSize && FFileHelper::LoadFileToArray(RawData, *Filename))
	{
		UPackage* AssetPackage = CreatePackage(NULL, *PackageName);
		UTextureFactory* texFactory = NewObject<UTextureFactory>();
		EObjectFlags Flags = RF_Public | RF_Standalone;

		bLoadedFile = true;
		RawData.Add(0);
		const uint8* Ptr = &RawData[0];
		UObject* texAsset = texFactory->FactoryCreateBinary(UTexture::StaticClass(), AssetPackage, FName(*BaseName), Flags, NULL, *FPaths::GetExtension(TextureFilename), Ptr, Ptr + RawData.Num() - 1, GWarn);

		if (texAsset)
		{
			UTexture* texture = Cast<UTexture>(texAsset);
			texture->SRGB = isSRGB;
			texture->CompressionSettings = compSetting;

			texAsset->MarkPackageDirty();
			ULevel::LevelDirtiedEvent.Broadcast();
			texAsset->PostEditChange();

			return Cast<UTexture>(texAsset);
		}
		else 
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Texture '%s' fails to create"), *TextureFilename);
		}

	}

	UE_LOG(ModoMaterialImporter, Log, TEXT("File '%s' is not a valid image"), *Filename);
	return NULL;
}
