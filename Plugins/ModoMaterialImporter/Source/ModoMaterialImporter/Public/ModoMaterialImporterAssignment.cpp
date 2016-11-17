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

void Assignment::ApplyToMeshes()
{
	UClass *refMeshClass= AStaticMeshActor::StaticClass();
	UClass *refSkeletaMeshClass = ASkeletalMeshActor::StaticClass();

	for (TObjectIterator<UObject> Itr; Itr; ++Itr)
	{
		if (Itr->GetClass()->IsChildOf(refMeshClass))
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Scan materials in: %s %s"), *Itr->GetName(), *Itr->GetClass()->GetDesc());

			AStaticMeshActor *aMeshActor = dynamic_cast<AStaticMeshActor*> (*Itr);

			if (aMeshActor != NULL)
			{
				UMeshComponent* meshCompo = aMeshActor->GetStaticMeshComponent();

				if (meshCompo != NULL)
				{
					for (int i = 0; i < meshCompo->GetNumMaterials(); i++)
					{
						UMaterialInterface* material = meshCompo->GetMaterial(i);

						// It seems a UE4 bug, GetNumMaterials contains NULL materials!
						if (material == NULL)
							continue;

						std::string strName = (TCHAR_TO_UTF8(*material->GetName()));

						std::map<std::string, UMaterial*>::iterator mat_itr = Materials.find(strName);
						if (mat_itr != Materials.end())
						{
							UE_LOG(ModoMaterialImporter, Log, TEXT("Set Material: %s"), *material->GetName());
							meshCompo->SetMaterial(i, mat_itr->second);
						}
					}
				}
			}
		}
		else if (Itr->GetClass()->IsChildOf(refSkeletaMeshClass))
		{
			UE_LOG(ModoMaterialImporter, Log, TEXT("Scan materials in: %s %s"), *Itr->GetName(), *Itr->GetClass()->GetDesc());

			ASkeletalMeshActor *aMeshActor = dynamic_cast<ASkeletalMeshActor*> (*Itr);

			if (aMeshActor != NULL)
			{
				UMeshComponent * meshCompo = aMeshActor->GetSkeletalMeshComponent();
				for (int i = 0; i < meshCompo->GetNumMaterials(); i++)
				{
					UMaterialInterface* material = meshCompo->GetMaterial(i);

					// It seems a UE4 bug, GetNumMaterials contains NULL materials!
					if (material == NULL)
						continue;

					std::string strName = (TCHAR_TO_UTF8(*material->GetName()));

					std::map<std::string, UMaterial*>::iterator mat_itr = Materials.find(strName);
					if (mat_itr != Materials.end())
					{
						UE_LOG(ModoMaterialImporter, Log, TEXT("Set Material: %s"), *material->GetName());
						meshCompo->SetMaterial(i, mat_itr->second);
					}
				}
			}
		}
	}
}