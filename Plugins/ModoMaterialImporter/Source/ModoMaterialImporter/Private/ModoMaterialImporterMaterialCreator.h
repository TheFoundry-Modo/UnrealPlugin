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

#pragma once

#include "LevelEditor.h"
#include "XmlParser.h"

#include "Materials/MaterialExpression.h"

#include "ModoMaterialImporterAssignment.h"
#include "ModoMaterialImporterTextureManager.h"
#include "ModoMaterialImporterContentPathTracker.h"

namespace ModoMaterial
{
	class MaterialCreator
	{
	public:

		static bool LoadMaterial(FXmlFile *matXml, const FString &path, Assignment* matAssign);
		static void SetContentBrowserPathRaw(const FString &path);
		static FString GetContentBrowserPathRaw();

	private:

		struct ImageInfo
		{
			FString filename;
			ColorSpace colorSpace; 
		};

		struct TextureInfo
		{
			const FXmlNode* node;
			FString filename;
			bool isSRGB; 
		};

		static void ProcessTextureInfo(const TArray<FXmlNode* >& materialNodes);
		static void ProcessMaterial(FXmlNode *matNode, const FString& contentPath, Assignment* matAssign);

		static void FindTextureNodes(const FXmlNode *Node, TArray<TextureInfo>& txtrInfos);
		static bool AddFloatParam(FXmlNode *Node, UMaterial* mat, FMaterialInput<float>& matInput, int &graphOffset);
		static bool AddVectorParam(FXmlNode *Node, UMaterial* mat, FMaterialInput<FVector>& matInput, int &graphOffset, EMaterialSamplerType type);
		static bool AddColorParam(FXmlNode *Node, UMaterial* mat, FMaterialInput<FColor>& matInput, int &graphOffset);
		static void AddUnkownParam(FXmlNode *Node, UMaterial* mat, int &graphOffset);

		static FString _path, _rootPath, _contentBrowserPath;
		static TArray< ImageInfo> _imageInfo;
		static bool _usePtagMaterialName;
		static UContentBrowserPathTracker *_cbPathTracker;
		static TArray<FString> _selectedMaterials;
	};
}