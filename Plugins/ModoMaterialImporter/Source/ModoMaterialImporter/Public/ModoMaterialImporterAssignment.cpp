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
#include "ModoMaterialImporterAssignment.h"
#include "ModoMaterialImporterLog.h"

#include "EngineUtils.h"
#include "AssetRegistryModule.h"
#include "Editor.h"

#include "Animation/SkeletalMeshActor.h"

using namespace ModoMaterial;

void Assignment::AddMaterial(UMaterial* mat, const FString& name)
{
	std::string strName(TCHAR_TO_UTF8(*name));

	if (Materials.find(strName) == Materials.end())
	{
		Materials[strName] = mat;
	}
	else
	{
		UE_LOG(ModoMaterialImporter, Log, TEXT("pTag name %s is replicated! uses the first one"), *name);
	}
}

UMaterial* Assignment::GetMaterial()
{
	if (Materials.begin() != Materials.end())
		return Materials.begin()->second;
	else
		return NULL;
}

void Assignment::ApplyToMeshes(const FString &path)
{
	UClass *refMeshClass = UStaticMesh::StaticClass();
	UClass *refSkeletaMeshClass = USkeletalMesh::StaticClass();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FString> PathsToScan;
	PathsToScan.Add(path);
	AssetRegistry.ScanPathsSynchronous(PathsToScan);

	TArray<FAssetData> MeshAssetList;
	AssetRegistry.GetAssetsByPath(FName(*path), MeshAssetList);

	for (int j = 0; j < MeshAssetList.Num(); j++)
	{
		UObject* asset = MeshAssetList[j].GetAsset();

		if (asset == NULL)
			continue;

		if (asset->GetClass() == refMeshClass)
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Scan materials in: %s %s"), *asset->GetName(), *asset->GetClass()->GetDesc());

			UStaticMesh* staticMesh = dynamic_cast<UStaticMesh*> (asset);

			if (staticMesh != NULL)
			{
				for (int i = 0; i < staticMesh->Materials.Num(); i++)
				{
					UMaterialInterface* material = staticMesh->Materials[i];

					// It seems a UE4 bug, GetNumMaterials contains NULL materials!
					if (material == NULL)
						continue;

					std::string strName = (TCHAR_TO_UTF8(*material->GetName()));

					std::map<std::string, UMaterial*>::iterator mat_itr = Materials.find(strName);
					if (mat_itr != Materials.end())
					{
						UE_LOG(ModoMaterialImporter, Log, TEXT("Set Material: %s"), *material->GetName());
						staticMesh->Materials[i] =  mat_itr->second;
					}
				}
			}

		}
		else if (asset->GetClass() == refSkeletaMeshClass)
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Scan materials in: %s %s"), *asset->GetName(), *asset->GetClass()->GetDesc());

			USkeletalMesh* skeletalMesh = dynamic_cast<USkeletalMesh*> (asset);

			if (skeletalMesh != NULL)
			{
				for (int i = 0; i < skeletalMesh->Materials.Num(); i++)
				{
					UMaterialInterface* material = skeletalMesh->Materials[i].MaterialInterface;

					// It seems a UE4 bug, GetNumMaterials contains NULL materials!
					if (material == NULL)
						continue;

					std::string strName = (TCHAR_TO_UTF8(*material->GetName()));

					std::map<std::string, UMaterial*>::iterator mat_itr = Materials.find(strName);
					if (mat_itr != Materials.end())
					{
						UE_LOG(ModoMaterialImporter, Log, TEXT("Set Material: %s"), *material->GetName());
						skeletalMesh->Materials[i].MaterialInterface = mat_itr->second;
					}
				}
			}
		}
	}
}