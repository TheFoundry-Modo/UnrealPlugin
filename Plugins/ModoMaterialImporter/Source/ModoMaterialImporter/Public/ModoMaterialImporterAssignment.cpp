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
	{
		return Materials.begin()->second;
	}
	else
	{
		return NULL;
	}
}

void Assignment::UpdateMaterialFlags(const FString &path)
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

	UpdateMaterialFlags (MeshAssetList);
}

void Assignment::UpdateMaterialFlags(const TArray<FAssetData> &MeshAssetList)
{
	UClass *refMeshClass = UStaticMesh::StaticClass();
	UClass *refSkeletaMeshClass = USkeletalMesh::StaticClass();

	UE_LOG(ModoMaterialImporter, Log, TEXT("Update Material Flag according to usage"));

	for (int j = 0; j < MeshAssetList.Num(); j++)
	{
		UObject* asset = MeshAssetList[j].GetAsset();

		if (asset == NULL)
		{
			continue;
		}

		if (asset->GetClass() == refMeshClass)
		{
			UStaticMesh* staticMesh = dynamic_cast<UStaticMesh*> (asset);

			if (staticMesh != NULL)
			{
				for (int i = 0; i < staticMesh->StaticMaterials.Num(); i++)
				{
					UMaterial* material = staticMesh->StaticMaterials[i].MaterialInterface->GetMaterial();

					if (material == NULL)
					{
						continue;
					}

					material->bUsedWithSkeletalMesh = false;
				}
			}
		}
	}

	for (int j = 0; j < MeshAssetList.Num(); j++)
	{
		UObject* asset = MeshAssetList[j].GetAsset();

		if (asset == NULL)
		{
			continue;
		}

		if (asset->GetClass() == refSkeletaMeshClass)
		{
			USkeletalMesh* skeletalMesh = dynamic_cast<USkeletalMesh*> (asset);

			if (skeletalMesh != NULL)
			{
				for (int i = 0; i < skeletalMesh->Materials.Num(); i++)
				{
					UMaterial* material = skeletalMesh->Materials[i].MaterialInterface->GetMaterial();

					if (material == NULL)
					{
						continue;
					}

					material->bUsedWithSkeletalMesh = true;
				}
			}
		}
	}
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
		{
			continue;
		}

		if (asset->GetClass() == refMeshClass)
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Scan materials in: %s %s"), *asset->GetName(), *asset->GetClass()->GetDesc());

			UStaticMesh* staticMesh = dynamic_cast<UStaticMesh*> (asset);

			if (staticMesh != NULL)
			{
				for (int i = 0; i < staticMesh->StaticMaterials.Num(); i++)
				{
					UMaterialInterface* material = staticMesh->StaticMaterials[i].MaterialInterface;

					// It seems a UE4 bug, GetNumMaterials contains NULL materials!
					if (material == NULL)
					{
						continue;
					}

					std::string strName = (TCHAR_TO_UTF8(*material->GetName()));

					std::map<std::string, UMaterial*>::iterator mat_itr = Materials.find(strName);
					if (mat_itr != Materials.end())
					{
						UE_LOG(ModoMaterialImporter, Log, TEXT("Set Material: %s"), *material->GetName());
						staticMesh->StaticMaterials[i].MaterialInterface =  mat_itr->second;
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
					{
						continue;
					}

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

	UpdateMaterialFlags (MeshAssetList);
}