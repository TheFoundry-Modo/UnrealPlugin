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

#include "ModoMaterialImporterStyle.h"
#include "SlateStyle.h"
#include "IPluginManager.h"
#include "EditorStyleSet.h"

#include "ModoMaterialImporterLog.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( ModoMaterialImporterStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )

FString ModoMaterialImporterStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("ModoMaterialImporter"))->GetContentDir();

	return ( ContentDir / RelativePath ) + Extension;
}

TSharedPtr< FSlateStyleSet > ModoMaterialImporterStyle::StyleSet;
TSharedPtr< class ISlateStyle > ModoMaterialImporterStyle::Get() { return StyleSet; }

void ModoMaterialImporterStyle::Initialize()
{
	const FVector2D Icon40x40(40.0f, 40.0f);

	// Only register once
	if ( StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable( new FSlateStyleSet("ModoMaterialImporterStyle") );
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// Plugins Manager
	{
		const FTextBlockStyle NormalText = FEditorStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");

		StyleSet->Set( "Plugins.ModoButtonIcon", new IMAGE_BRUSH("modo", Icon40x40));
	}

	FSlateStyleRegistry::RegisterSlateStyle( *StyleSet.Get() );
};

void ModoMaterialImporterStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle( *StyleSet.Get() );
		ensure( StyleSet.IsUnique() );
		StyleSet.Reset();
	}
}