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
#include "ModoMaterialImporterMaterialCreator.h"
#include "ModoMaterialImporterHelper.h"
#include "ModoMaterialImporterAssignment.h"
#include "ModoMaterialImporterLog.h"

#include "IMaterialEditor.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant.h"

#include "AssetRegistryModule.h"

//#define UNKNOWN_CONSTANT_NODES

using namespace ModoMaterial;

#include <vector>
using std::vector;

FString MaterialCreator::_path = FString();
FString MaterialCreator::_rootPath = FString();
TArray< MaterialCreator::ImageInfo> MaterialCreator::_imageInfo;
bool MaterialCreator::_usePtagMaterialName = false;

bool isExt(const FString &str, const FString &ext)
{
	return str.Equals(ext, ESearchCase::IgnoreCase);
}

bool isTextureFileName(const FString &content)
{
	unsigned extStart = 0;

	for (int i = content.Len() - 1; i >= 0; --i)
	{
		if (content[i] == TCHAR('.'))
		{
			extStart = i;
			FString extension = content.Right(content.Len() - i - 1);
			UE_LOG(ModoMaterialImporter, Log, TEXT("%s might be a filename"), *content);

			if (isExt(extension, TEXT("bmp")) || isExt(extension, TEXT("float")) 
				|| isExt(extension, TEXT("pcx")) || isExt(extension, TEXT("png"))
				|| isExt(extension, TEXT("psd")) || isExt(extension, TEXT("tga")) || isExt(extension, TEXT("jpg"))
				|| isExt(extension, TEXT("exr")) || isExt(extension, TEXT("dds")) || isExt(extension, TEXT("hdr")))
			{
				return true;
			}
		}
	}

	return false;
}

void getDigitalNumber(const FString &source, FString *left, FString *right)
{
	if (source.Contains(TEXT(",")))
		source.Split(source, left, right, ESearchCase::CaseSensitive, ESearchDir::FromStart);

	if (left->Contains(TEXT(",")))
	{
		FString leftstr;
		FString rightstr;
		getDigitalNumber(*left, &leftstr, &rightstr);
	}

	if (right->Contains(TEXT(",")))
	{
		FString leftstr;
		FString rightstr;
		getDigitalNumber(*left, &leftstr, &rightstr);
	}

}

FVector4 processDigitalNumbers(const FString &content)
{
	FString symbol = TEXT(",");
	FString leftstr, rightstr;

	float color[4];

	color[0] = color[1] = color[2] = color[3] = 0.0f;

	int start = 0;
	int index = 0;

	for (int i = 0; i < content.Len(); ++i)
	{
		if (content[i] == TCHAR(','))
		{
			FString subString = content.Mid(start, i - start);
			UE_LOG(ModoMaterialImporter, Log, TEXT("Convert substring %s to float"), *subString);
			color[index++] = FCString::Atof(*subString);

			start = i + 1;
		}
	}

	// the last number
	FString subString = content.Mid(start, content.Len() - start);
	UE_LOG(ModoMaterialImporter, Log, TEXT("Convert substring %s to float"), *subString);
	color[index++] = FCString::Atof(*subString);

	FVector4 vec(color[0], color[1], color[2], color[3]);

	return vec;
}

int channelOutputIndex(const FString& swizzling)
{
	if (swizzling == "Red" || swizzling == "R" || swizzling == "r")
		return 1;

	if (swizzling == "Green" || swizzling == "G" || swizzling == "g")
		return 2;

	if (swizzling == "Blue" || swizzling == "B" || swizzling == "b")
		return 3;

	if (swizzling == "Alpha" || swizzling == "A" || swizzling == "a")
		return 4;

	return 0;
}

template <typename T>
void LinkConstant(UMaterial* mat, const vector<float>& value, FMaterialInput<T>* matInput, int& position)
{
	if (mat != nullptr)
	{
		UMaterialExpression* Expression = NULL;

		switch (value.size())
		{
		case 0:
			return;
		case 1:
		{
				  UMaterialExpressionConstant* expConst = NewObject<UMaterialExpressionConstant>(mat);
				  expConst->R = value[0];
				  Expression = expConst;
		}
			break;
		case 2:
		{
				  UMaterialExpressionConstant2Vector* expConst = NewObject<UMaterialExpressionConstant2Vector>(mat);
				  expConst->R = value[0];
				  expConst->G = value[1];
				  Expression = expConst;
		}
			break;
		case 3:
		{
				  UMaterialExpressionConstant3Vector* expConst = NewObject<UMaterialExpressionConstant3Vector>(mat);
				  expConst->Constant = FLinearColor(value[0], value[1], value[2]);
				  Expression = expConst;
		}
			break;
		default:
		{
				   UMaterialExpressionConstant4Vector* expConst = NewObject<UMaterialExpressionConstant4Vector>(mat);
				   expConst->Constant = FLinearColor(value[0], value[1], value[2], value[3]);
				   Expression = expConst;
		}
			break;
		}
		mat->Expressions.Add(Expression);

		Expression->MaterialExpressionEditorX = -200;
		Expression->MaterialExpressionEditorY = position;
		position += 64;

		TArray<FExpressionOutput> Outputs;
		Outputs = Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		if (matInput) {
			matInput->Expression = Expression;
			matInput->Mask = Output->Mask;
			matInput->MaskR = Output->MaskR;
			matInput->MaskG = Output->MaskG;
			matInput->MaskB = Output->MaskB;
			matInput->MaskA = Output->MaskA;
		}

		mat->PostEditChange();
	}
}

template <typename T>
void LinkTexture(
	UMaterial* mat, 
	UTexture* tex, 
	FMaterialInput<T>* matInput, 
	int& position, 
	float tiling[2], 
	int uvIndex,
	int outIndex, 
	EMaterialSamplerType samplerType = EMaterialSamplerType::SAMPLERTYPE_Color)
{
	if (mat != nullptr && tex != nullptr)
	{
		// An initial texture was specified, add it and assign it to the BaseColor
		UMaterialExpressionTextureSample* Expression = NULL;

		// Look into current material and get textureSampler if exists
		TArray<UMaterialExpressionTextureSample*> texSampleExpressions;
		mat->GetAllExpressionsInMaterialAndFunctionsOfType<UMaterialExpressionTextureSample>(texSampleExpressions);

		for (int i = 0; i < texSampleExpressions.Num(); i++)
		{
			UMaterialExpressionTextureSample* texSampleExpression = texSampleExpressions[i];

			// check texture
			if (tex == texSampleExpression->Texture)
			{
				// check tiling ... maybe check more in the future
				UMaterialExpressionTextureCoordinate* texCoordExpression = dynamic_cast<UMaterialExpressionTextureCoordinate*>(texSampleExpression->Coordinates.Expression);
				if (texCoordExpression != NULL)
				{
					if (	texCoordExpression->UTiling == tiling[0] && 
							texCoordExpression->VTiling == tiling[1] &&
							texCoordExpression->CoordinateIndex == uvIndex
							)
					{
						Expression = texSampleExpression;
						break;
					}
				}
				else
				{
					if (tiling[0] == 1.0 && tiling[1] == 1.0 && uvIndex == 0)
					{
						Expression = texSampleExpression; // this line is not needed
						break;
					}
				}
			}
		}

		// If Expression can not be found, we create a new one
		if (Expression == NULL)
		{
			Expression = NewObject<UMaterialExpressionTextureSample>(mat);
			mat->Expressions.Add(Expression);
			Expression->MaterialExpressionEditorX = -380;
			Expression->MaterialExpressionEditorY = position;
			position += 64;

			Expression->Texture = tex;
			Expression->SamplerType = samplerType;

			if (tiling[0] != 1.0 || tiling[1] != 1.0 || uvIndex != 0)
			{
				TArray<UMaterialExpressionTextureCoordinate*> texCoordExpressions;

				mat->GetAllExpressionsInMaterialAndFunctionsOfType<UMaterialExpressionTextureCoordinate>(texCoordExpressions);
				FExpressionInput texCoordInput;
				bool texCoordExists = false;

				for (int i = 0; i < texCoordExpressions.Num(); i++)
				{
					UMaterialExpressionTextureCoordinate* texCoordExpression = texCoordExpressions[i];
					if (	texCoordExpression->UTiling == tiling[0] && 
							texCoordExpression->VTiling == tiling[1] &&
							texCoordExpression->CoordinateIndex == uvIndex)
					{
						texCoordInput.Expression = texCoordExpression;
						texCoordExists = true;
						break;
					}
				}

				if (texCoordExists == false)
				{
					UMaterialExpressionTextureCoordinate *texCoordExpression = NewObject<UMaterialExpressionTextureCoordinate>(mat);
					texCoordExpression->UTiling = tiling[0];
					texCoordExpression->VTiling = tiling[1];
					texCoordExpression->CoordinateIndex = uvIndex;
					mat->Expressions.Add(texCoordExpression);
					texCoordExpression->MaterialExpressionEditorX = Expression->MaterialExpressionEditorX - 100;
					texCoordExpression->MaterialExpressionEditorY = position;
					texCoordInput.Expression = texCoordExpression;
				}

				Expression->Coordinates = texCoordInput;
			}
		}

		if (matInput != NULL)
			matInput->Connect(outIndex, Expression);

		mat->PostEditChange();
	}
}

// Remove all invalid characters in materialName before passing in
UMaterial* CreateMaterial(FString materialName)
{
	UMaterialFactoryNew* matFactory = NewObject<UMaterialFactoryNew>();
	FString PackageName = TEXT("/Game/") + materialName;

	UE_LOG(ModoMaterialImporter, Log, TEXT("Creating package: %s"), *PackageName);

	// Deal with invalid long name issue
	if (!CommonHelper::GetValidePackageName(PackageName))
		return NULL;

	UPackage* AssetPackage = CreatePackage(NULL, *PackageName);
	EObjectFlags Flags = RF_Public | RF_Standalone;

	UObject* CreatedAsset = matFactory->FactoryCreateNew(UMaterial::StaticClass(), AssetPackage, FName(*materialName), Flags, NULL, GWarn);

	if (CreatedAsset)
	{
		// Mark the package dirty...
		AssetPackage->MarkPackageDirty();
	}

	return Cast<UMaterial>(CreatedAsset);
}

void MaterialCreator::LoadMaterial(FXmlFile *matXml, const FString &path, Assignment* matAssign)
{
	FXmlNode *rootNode = matXml->GetRootNode();

	if (rootNode != NULL)
	{

		const TArray< FXmlNode * > matNodes = rootNode->GetChildrenNodes();

		// The path of the XML file is used as the base path when "Relative Export Path" is checked in the exporter settings.
		_path = FString (path);
		_imageInfo.Empty();
		_usePtagMaterialName = false;

		// Find image nodes firstly, we need image properties for images when creating materials
		for (int j = 0; j < matNodes.Num(); j++)
		{
			FXmlNode *matNode = matNodes[j];
			FString tag = matNode->GetTag();

			if (tag.Equals(TEXT("ImageFiles"), ESearchCase::CaseSensitive))
			{
				TArray<FXmlNode*> imageNodes = matNode->GetChildrenNodes();

				for (int32 i = 0; i < imageNodes.Num(); i++)
				{
					FXmlNode* imageNode = imageNodes[i];

					if (imageNode->GetTag().Equals(TEXT("file"), ESearchCase::IgnoreCase))
					{
						ImageInfo imageInfo;
						imageInfo.filename = imageNode->GetAttribute(FString("filename"));

						FString imageCC = imageNode->GetAttribute(FString("color_correction"));
						UE_LOG(ModoMaterialImporter, Log, TEXT("Load image properties: %s [%s]"), *imageInfo.filename, *imageCC);

						if (imageCC.Equals(TEXT("linear"), ESearchCase::IgnoreCase))
							imageInfo.colorSpace = ColorSpace::Linear;
						else
							imageInfo.colorSpace = ColorSpace::sRGB;

						_imageInfo.Add(imageInfo);
					}
				}
				break;
			}
		}

		for (int j = 0; j < matNodes.Num(); j++)
		{
			FXmlNode *matNode = matNodes[j];
			FString tag = matNode->GetTag();
			FString ptag = FString();

			if (tag.Equals(TEXT("Version"), ESearchCase::CaseSensitive))
			{
				FString Content = matNode->GetContent();
				
				if (!Content.IsEmpty())
				{
					int32 versionNum = FCString::Atoi(*Content);

					if (versionNum > MODO_VER)
						UE_LOG(ModoMaterialImporter, Log, TEXT("WARNING: The importer is out of date (The XML file is exported from a higher version of MODO)."));
				}
			}
			else if (tag.Equals(TEXT("PtagAsID"), ESearchCase::CaseSensitive))
			{
				FString Content = matNode->GetContent();

				if (!Content.IsEmpty())
					_usePtagMaterialName = Content.ToBool();
			}
			else if (tag.Equals(TEXT("ImageFiles"), ESearchCase::CaseSensitive))
			{
				// already done in the pre-process
			}
			else if (tag.Equals(TEXT("useRootPath"), ESearchCase::CaseSensitive))
			{
				// useRootPath is redundant
			}
			else if (tag.Equals(TEXT("RootPath"), ESearchCase::CaseSensitive))
			{
				// useRelativePath is also redundant,
				// If we export XML with relative texture file paths, then their filenames are relative, otherwise absolute, 
				// so testing filenames are enough to know if they are relative.
				// For compatibility, we will still use "RootPath" as a backup solution.
				// See TextureManager::LoadTexture for more details.

				// Search into rootPath when we can not find textures, this is for backward compatibility only
				_rootPath = matNode->GetContent();
			}
			else if (tag.Equals(TEXT("Material"), ESearchCase::CaseSensitive))
			{

				FString matID = matNode->GetAttribute(FString("ID"));
				int graphOffset = -64;

				if (matID.IsEmpty())
				{
					UE_LOG(ModoMaterialImporter, Log, TEXT("Invalid Material ID"));
					return;
				}

				ptag = matNode->GetAttribute(FString("ptag"));

				TArray<FXmlNode*> propertyNodes = matNode->GetChildrenNodes();

				FXmlNode * baseColorNode = NULL;
				FXmlNode * metallicNode = NULL;
				FXmlNode * opacityNode = NULL;
				FXmlNode * emissiveColorNode = NULL;
				FXmlNode * specularNode = NULL;
				FXmlNode * normalNode = NULL;
				FXmlNode * roughnessNode = NULL;
				FXmlNode * clearcoatNode = NULL;
				FXmlNode * clearcoatRoughNode = NULL;
				FXmlNode * ambientOcclusionNode = NULL;
				FXmlNode * subsurfaceColorNode = NULL;

				TArray<FXmlNode *> unkownNodes;

				for (int32 i = 0; i < propertyNodes.Num(); i++)
				{
					FXmlNode* propertyNode = propertyNodes[i];
					if (propertyNode->GetTag().Equals(TEXT("property"), ESearchCase::IgnoreCase))
					{
						FString propertyName = propertyNode->GetAttribute(FString("name"));
						UE_LOG(ModoMaterialImporter, Log, TEXT("Load property %s"), *propertyName);

						if (propertyName.Equals(TEXT("Base Color"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Base Color"), ESearchCase::IgnoreCase))
							baseColorNode = propertyNode;
						else if (propertyName.Equals(TEXT("Metallic"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Metallic"), ESearchCase::IgnoreCase))
							metallicNode = propertyNode;
						else if (propertyName.Equals(TEXT("Opacity"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Opacity"), ESearchCase::IgnoreCase))
							opacityNode = propertyNode;
						else if (propertyName.Equals(TEXT("Emissive Color"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Emissive Color"), ESearchCase::IgnoreCase))
							emissiveColorNode = propertyNode;
						else if (propertyName.Equals(TEXT("Specular"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Specular"), ESearchCase::IgnoreCase))
							specularNode = propertyNode;
						else if (propertyName.Equals(TEXT("Normal"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Normal"), ESearchCase::IgnoreCase))
							normalNode = propertyNode;
						else if (propertyName.Equals(TEXT("Roughness"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Roughness"), ESearchCase::IgnoreCase))
							roughnessNode = propertyNode;
						else if (propertyName.Equals(TEXT("Clearcoat Amount"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Clearcoat Amount"), ESearchCase::IgnoreCase))
							clearcoatNode = propertyNode;
						else if (propertyName.Equals(TEXT("Clearcoat Roughness"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Clearcoat Roughness"), ESearchCase::IgnoreCase))
							clearcoatRoughNode = propertyNode;
						else if (propertyName.Equals(TEXT("Ambient Occlusion"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("Unreal Ambient Occlusion"), ESearchCase::IgnoreCase))
							ambientOcclusionNode = propertyNode;
						else if (propertyName.Equals(TEXT("Subsurface Color"), ESearchCase::IgnoreCase) || propertyName.Equals(TEXT("UNreal Subsurface Color"), ESearchCase::IgnoreCase))
							subsurfaceColorNode = propertyNode;
						else
						{
							unkownNodes.Add(propertyNode);
							UE_LOG(ModoMaterialImporter, Log, TEXT("property %s is not supported, append to unknow nodes"), *propertyName);
						}

						// TODO: Bump
					}
					else
					{
						UE_LOG(ModoMaterialImporter, Log, TEXT("Ignore non-property node"));
					}
				}

				FString materialName;

				if (_usePtagMaterialName)
				{
					// Use the ptag for the material name.
					if (!ptag.IsEmpty())
						materialName = ptag;
					else
						return;

					// Remove invalid characters and any '_skinXX' suffix from the material name.
					CommonHelper::RemoveInvalidCharacters(materialName);
					CommonHelper::RemoveMaterialSlotSuffix(materialName);
				}
				else
				{
					// Use nested material ID for the material name.
					materialName = matID;
					materialName = materialName + FString("_") + ptag;

					// Remove invalid characters
					CommonHelper::RemoveInvalidCharacters(materialName);
				}

				UMaterial* mat = CreateMaterial(materialName);

				if (!mat)
					return;

				if (ptag.IsEmpty())
				{
					UE_LOG(ModoMaterialImporter, Log, TEXT("Material %s doesn't have a ptag for auto assignment!"), *mat->GetName());
				}
				else
				{
					// use ptag for material matching, make sure ptag here is the same as the ptag in FBX file.
					matAssign->AddMaterial(mat, ptag);
				}

				IAssetEditorInstance* OpenEditor = FAssetEditorManager::Get().FindEditorForAsset(mat, true);
				IMaterialEditor* CurrentMaterialEditor = (IMaterialEditor*)OpenEditor;


				bool useTransparent, useClearCoat, useSubsurface;

				useTransparent = false;
				useClearCoat = false;
				useSubsurface = false;

				AddColorParam(baseColorNode, mat, mat->BaseColor, graphOffset);
				AddFloatParam(metallicNode, mat, mat->Metallic, graphOffset);

				useTransparent = AddFloatParam(opacityNode, mat, mat->Opacity, graphOffset);

				AddColorParam(emissiveColorNode, mat, mat->EmissiveColor, graphOffset);
				AddVectorParam(normalNode, mat, mat->Normal, graphOffset, SAMPLERTYPE_Normal);
				AddFloatParam(specularNode, mat, mat->Specular, graphOffset);
				AddFloatParam(roughnessNode, mat, mat->Roughness, graphOffset);
				
				useClearCoat |= AddFloatParam(clearcoatNode, mat, mat->ClearCoat, graphOffset);
				useClearCoat |= AddFloatParam(clearcoatRoughNode, mat, mat->ClearCoatRoughness, graphOffset);

				AddFloatParam(ambientOcclusionNode, mat, mat->AmbientOcclusion, graphOffset);
				
				useSubsurface = AddColorParam(subsurfaceColorNode, mat, mat->SubsurfaceColor, graphOffset);

				if (useTransparent)
					mat->BlendMode = BLEND_Translucent;

				if (useClearCoat)
					mat->SetShadingModel(MSM_ClearCoat);
				
				if (useSubsurface)
					mat->SetShadingModel(MSM_Subsurface);

				if (useTransparent || useClearCoat || useSubsurface)
					mat->PostEditChange();

				for (int i = 0; i < unkownNodes.Num(); i++)
					AddUnkownParam(unkownNodes[i], mat, graphOffset);

				FAssetRegistryModule::AssetCreated(mat);

				if (mat)
				{
					FAssetEditorManager::Get().OpenEditorForAsset(mat);
				}
			}
			else
			{

				UE_LOG(ModoMaterialImporter, Log, TEXT("Invalid Material Tag %s"), *tag);
			}
		}
	}
}

void MaterialCreator::FindTextureNodes(const FXmlNode *Node, TArray<TextureInfo>& txtrInfos)
{
	FString content = Node->GetAttribute("texture");
	if (!content.IsEmpty()) {
		TextureInfo txtrInfo;
		txtrInfo.node = Node;
		txtrInfo.filename = content;
		txtrInfo.isSRGB = true;
		txtrInfos.Add(txtrInfo);
		return;
	}
	const TArray<FXmlNode*> childNodes = Node->GetChildrenNodes();
	for (int i = 0; i < childNodes.Num(); i++)
	{
		if (childNodes[i]->GetTag() == "texture")
		{
			TextureInfo txtrInfo;
			txtrInfo.node = childNodes[i];
			txtrInfo.filename = childNodes[i]->GetAttribute("filename");
			txtrInfo.isSRGB = true;

			FString imgProIdx = childNodes[i]->GetAttribute("fileIndex");

			if (!imgProIdx.IsEmpty())
			{
				int32 index = FCString::Atoi(*imgProIdx);

				txtrInfo.isSRGB = (_imageInfo[index].colorSpace == ColorSpace::sRGB) ? true : false;
			}

			txtrInfos.Add(txtrInfo);
		}
	}
}

bool MaterialCreator::AddFloatParam(FXmlNode *Node, UMaterial* mat, FMaterialInput<float>& matInput, int &graphOffset)
{
	if (Node)
	{
		TArray<TextureInfo> textureNodeInfos;
		FindTextureNodes(Node, textureNodeInfos);
		bool anyTextureUsed = false;

		for (int i = 0; i < textureNodeInfos.Num(); i++)
		{
			const FString& content = textureNodeInfos[i].filename;
			const FXmlNode*	texNode = textureNodeInfos[i].node;
			const bool isSRGB = textureNodeInfos[i].isSRGB;

			if (!content.IsEmpty() && isTextureFileName(content))
			{
				ModoMaterial::TextureManager * texManager = ModoMaterial::TextureManager::Instance();
				UTexture* tex = texManager->LoadTexture(*content, _path, _rootPath, isSRGB, TC_Default);
				if (tex)
				{
					FString wrapU = texNode->GetAttribute("wrapU");
					FString wrapV = texNode->GetAttribute("wrapV");
					float tiling[2];

					tiling[0] = tiling[1] = 1.0;

					if (!wrapU.IsEmpty())
						tiling[0] = processDigitalNumbers(wrapU)[0];

					if (!wrapV.IsEmpty())
						tiling[1] = processDigitalNumbers(wrapV)[0];

					FString swizzling = texNode->GetAttribute("channel");
					int outIndex = channelOutputIndex(swizzling);

					int uvChannelIndex = 0;
					FString uvChannelIndexStr = texNode->GetAttribute("uvindex");
					if (!wrapV.IsEmpty())
						uvChannelIndex = FCString::Atoi(*uvChannelIndexStr);

					FAssetRegistryModule::AssetCreated(tex);

					EMaterialSamplerType type;
					type = (isSRGB == true) ? SAMPLERTYPE_Color : SAMPLERTYPE_LinearColor;
					LinkTexture<float>(mat, tex, &matInput, graphOffset, tiling, uvChannelIndex, outIndex, type);

					anyTextureUsed = true;
				}
			}
		}

		if (!anyTextureUsed)
		{
			FString content = Node->GetAttribute("value");
			FVector4 vec = processDigitalNumbers(content);

			if (vec[0] == matInput.Constant)
				return false;

			matInput.Constant = vec[0];

			vector<float> color = { vec[0] };

			LinkConstant<float>(mat, color, &matInput, graphOffset);
		}

		return true;
	}

	return false;
}

bool MaterialCreator::AddVectorParam(FXmlNode *Node, UMaterial* mat, FMaterialInput<FVector>& matInput, int &graphOffset, EMaterialSamplerType type)
{
	if (Node)
	{
		TArray<TextureInfo> textureNodeInfos;
		FindTextureNodes(Node, textureNodeInfos);
		bool anyTextureUsed = false;

		for (int i = 0; i < textureNodeInfos.Num(); i++)
		{
			const FString& content = textureNodeInfos[i].filename;
			const FXmlNode*	texNode = textureNodeInfos[i].node;
			const bool isSRGB = textureNodeInfos[i].isSRGB;

			if (!content.IsEmpty() && isTextureFileName(content))
			{
				ModoMaterial::TextureManager	*texManager = ModoMaterial::TextureManager::Instance();
				TextureCompressionSettings	 texCompSet = TC_Default;

				if (type == SAMPLERTYPE_Normal)
					texCompSet = TC_Normalmap;

				UTexture* tex = texManager->LoadTexture(*content, _path, _rootPath, isSRGB, texCompSet);
				if (tex)
				{
					FString wrapU = texNode->GetAttribute("wrapU");
					FString wrapV = texNode->GetAttribute("wrapV");
					float tiling[2];

					tiling[0] = tiling[1] = 1.0;

					if (!wrapU.IsEmpty())
						tiling[0] = processDigitalNumbers(wrapU)[0];

					if (!wrapV.IsEmpty())
						tiling[1] = processDigitalNumbers(wrapV)[0];

					FString swizzling = texNode->GetAttribute("channel");
					int outIndex = channelOutputIndex(swizzling);

					int uvChannelIndex = 0;
					FString uvChannelIndexStr = texNode->GetAttribute("uvindex");
					if (!wrapV.IsEmpty())
						uvChannelIndex = FCString::Atoi(*uvChannelIndexStr);

					FAssetRegistryModule::AssetCreated(tex);
					LinkTexture<FVector>(mat, tex, &matInput, graphOffset, tiling, uvChannelIndex, outIndex, type);

					anyTextureUsed = true;
				}
			}
		}

		if (!anyTextureUsed)
		{
			FString content = Node->GetAttribute("value");
			FVector4 vec = processDigitalNumbers(content);

			if (matInput.Constant == FVector(vec[0], vec[1], vec[2]))
				return false;

			matInput.Constant = FVector(vec[0], vec[1], vec[2]);

			vector<float> color = { vec[0], vec[1], vec[2] };

			LinkConstant<FVector>(mat, color, &matInput, graphOffset);
		}

		return true;
	}

	return false;
}

bool MaterialCreator::AddColorParam(FXmlNode *Node, UMaterial* mat, FMaterialInput<FColor>& matInput, int &graphOffset)
{
	if (Node)
	{
		TArray<TextureInfo> textureNodeInfos;
		FindTextureNodes(Node, textureNodeInfos);
		bool anyTextureUsed = false;

		for (int i = 0; i < textureNodeInfos.Num(); i++)
		{
			const FString& content = textureNodeInfos[i].filename;
			const FXmlNode*	texNode = textureNodeInfos[i].node;
			const bool isSRGB = textureNodeInfos[i].isSRGB;

			if (!content.IsEmpty() && isTextureFileName(content))
			{
				ModoMaterial::TextureManager * texManager = ModoMaterial::TextureManager::Instance();
				UTexture* tex = texManager->LoadTexture(*content, _path, _rootPath, isSRGB, TC_Default);
				if (tex)
				{
					FString wrapU = texNode->GetAttribute("wrapU");
					FString wrapV = texNode->GetAttribute("wrapV");
					float tiling[2];

					tiling[0] = tiling[1] = 1.0;

					if (!wrapU.IsEmpty())
						tiling[0] = processDigitalNumbers(wrapU)[0];

					if (!wrapV.IsEmpty())
						tiling[1] = processDigitalNumbers(wrapV)[0];

					FString swizzling = texNode->GetAttribute("channel");
					int outIndex = channelOutputIndex(swizzling);

					int uvChannelIndex = 0;
					FString uvChannelIndexStr = texNode->GetAttribute("uvindex");
					if (!wrapV.IsEmpty())
						uvChannelIndex = FCString::Atoi(*uvChannelIndexStr);

					FAssetRegistryModule::AssetCreated(tex);

					EMaterialSamplerType type;
					type = (isSRGB == true) ? SAMPLERTYPE_Color : SAMPLERTYPE_LinearColor;
					LinkTexture<FColor>(mat, tex, &matInput, graphOffset, tiling, uvChannelIndex, outIndex, type);

					anyTextureUsed = true;
				}
			}
		}

		if (!anyTextureUsed)
		{
			FString content = Node->GetAttribute("value");
			FVector4 vec = processDigitalNumbers(content);

			uint8 R = FMath::Clamp((int)(vec[0] * 255), 0, 255);
			uint8 G = FMath::Clamp((int)(vec[1] * 255), 0, 255);
			uint8 B = FMath::Clamp((int)(vec[2] * 255), 0, 255);
			uint8 A = FMath::Clamp((int)(vec[3] * 255), 0, 255);

			if (matInput.Constant == FColor(R, G, B, A))
				return false;

			matInput.Constant = FColor(R, G, B, A);

			vector<float> color = { vec[0], vec[1], vec[2], vec[3] };

			LinkConstant<FColor>(mat, color, &matInput, graphOffset);
		}

		return true;
	}

	return false;
}

void MaterialCreator::AddUnkownParam(FXmlNode *Node, UMaterial* mat, int &graphOffset)
{
	if (Node)
	{
		TArray<TextureInfo> textureNodeInfos;
		FindTextureNodes(Node, textureNodeInfos);
		bool anyTextureUsed = false;

		for (int i = 0; i < textureNodeInfos.Num(); i++)
		{
			const FString& content = textureNodeInfos[i].filename;
			const FXmlNode*	texNode = textureNodeInfos[i].node;
			const bool isSRGB = textureNodeInfos[i].isSRGB;

			if (!content.IsEmpty() && isTextureFileName(content))
			{
				ModoMaterial::TextureManager * texManager = ModoMaterial::TextureManager::Instance();
				UTexture* tex = texManager->LoadTexture(*content, _path, _rootPath, isSRGB, TC_Default);
				if (tex)
				{
					FString wrapU = texNode->GetAttribute("wrapU");
					FString wrapV = texNode->GetAttribute("wrapV");
					float tiling[2];

					tiling[0] = tiling[1] = 1.0;

					if (!wrapU.IsEmpty())
						tiling[0] = processDigitalNumbers(wrapU)[0];

					if (!wrapV.IsEmpty())
						tiling[1] = processDigitalNumbers(wrapV)[0];

					FString swizzling = texNode->GetAttribute("channel");
					int outIndex = channelOutputIndex(swizzling);

					int uvChannelIndex = 0;
					FString uvChannelIndexStr = texNode->GetAttribute("uvindex");
					if (!wrapV.IsEmpty())
						uvChannelIndex = FCString::Atoi(*uvChannelIndexStr);

					FAssetRegistryModule::AssetCreated(tex);

					EMaterialSamplerType type;
					type = (isSRGB == true) ? SAMPLERTYPE_Color : SAMPLERTYPE_LinearColor;
					LinkTexture<FColor>(mat, tex, NULL, graphOffset, tiling, uvChannelIndex, outIndex, type);

					anyTextureUsed = true;
				}
			}
		}

		if (!anyTextureUsed)
		{
#ifdef UNKNOWN_CONSTANT_NODES
			const FString& content = Node->GetAttribute("value");
			FVector4 vec = processDigitalNumbers(content);

			uint8 R = FMath::Clamp((int)(vec[0] * 255), 0, 255);
			uint8 G = FMath::Clamp((int)(vec[1] * 255), 0, 255);
			uint8 B = FMath::Clamp((int)(vec[2] * 255), 0, 255);
			uint8 A = FMath::Clamp((int)(vec[3] * 255), 0, 255);

			vector<float> color = { vec[0], vec[1], vec[2], vec[3] };

			LinkConstent<FColor>(mat, color, NULL, graphOffset);
#else
			UE_LOG(ModoMaterialImporter, Log, TEXT("Remove unknown property %s, as it doesn't reference any images"), *(Node->GetAttribute("name")));
#endif
		}
	}

}
