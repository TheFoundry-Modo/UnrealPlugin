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
 
#include "ModoMaterialImporter.h"
#include "DesktopPlatformModule.h"
#include "MaterialEditorModule.h"
#include "InputCoreTypes.h"

#include "ModoMaterialImporterLog.h"
#include "ModoMaterialImporterStyle.h"
#include "ModoMaterialImporterMaterialCreator.h"
#include "ModoMaterialImporterAssignment.h"

IMPLEMENT_MODULE(ModoMaterialImporterModule, ModoMaterialImporter)

void ModoMaterialImporterModule::StartupModule()
{
	ModoMaterialImporterCommands::Register();
	ModoMaterialImporterStyle::Initialize();

	_PluginCommands = MakeShareable(new FUICommandList);

	_PluginCommands->MapAction(
		ModoMaterialImporterCommands::Get().Button,
		FExecuteAction::CreateRaw(this, &ModoMaterialImporterModule::ButtonClicked),
		FCanExecuteAction());

	_ToolbarExtender = MakeShareable(new FExtender);
	_ToolbarExtension = _ToolbarExtender->AddToolBarExtension("Content", EExtensionHook::Before, _PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &ModoMaterialImporterModule::AddToolbarExtension));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(_ToolbarExtender);
	
	_ExtensionManager = LevelEditorModule.GetToolBarExtensibilityManager();

	mLastPath = FString("\\");
}

void ModoMaterialImporterModule::ShutdownModule()
{
	ModoMaterialImporterStyle::Shutdown();

	if (_ExtensionManager.IsValid())
	{
		ModoMaterialImporterCommands::Unregister();

		_ToolbarExtender->RemoveExtension(_ToolbarExtension.ToSharedRef());

		_ExtensionManager->RemoveExtender(_ToolbarExtender);
	}
	else
	{
		_ExtensionManager.Reset();
	}
}

void ModoMaterialImporterModule::ButtonClicked()
{
	TArray< FString > OutFilenames;
	const FString DialogTitle = FString("Load Modo Material");
	const FString DefaultFile = FString("");
	const FString FileTypes = FString("Modo Material (XML)|*.XML;*.xml");

	FDesktopPlatformModule::Get()->OpenFileDialog(NULL, DialogTitle, mLastPath, DefaultFile, FileTypes, 0, OutFilenames);

	if (OutFilenames.Num() != 0)
	{
		UE_LOG(ModoMaterialImporter, Log, TEXT("Load %s"), *OutFilenames[0]);

		FXmlFile* matXml = new FXmlFile(OutFilenames[0]);

		// Find Path of associated resources
		int ilastslash = 0;
		for (ilastslash = OutFilenames[0].Len() - 1; ilastslash >= 0; --ilastslash)
		{

			if (OutFilenames[0][ilastslash] == TCHAR('/'))
				break;
		}

		FString path = OutFilenames[0].Left(ilastslash + 1);
		UE_LOG(ModoMaterialImporter, Log, TEXT("Resource Path %s"), *path);

		// Store the path the file was loaded from for the next time the file requester is opened.
		mLastPath = path;

		if (matXml->IsValid())
		{
			ModoMaterial::Assignment matAssignment;
			ModoMaterial::MaterialCreator::LoadMaterial(matXml, path, &matAssignment);
			matAssignment.ApplyToMeshes();
		}

		delete matXml;
	}
}

void ModoMaterialImporterModule::AddToolbarExtension(FToolBarBuilder &builder)
{
#define LOCTEXT_NAMESPACE "ModoMaterialImporter"

	UE_LOG(ModoMaterialImporter, Log, TEXT("Starting Extension logic"));

	FSlateIcon IconBrush = FSlateIcon(ModoMaterialImporterStyle::Get()->GetStyleSetName(), "Plugins.ModoButtonIcon");

	builder.AddToolBarButton(ModoMaterialImporterCommands::Get().Button, NAME_None, LOCTEXT("Button_Override", "Modo Material"), LOCTEXT("Button_ToolTipOverride", "Load a modo XML material"), IconBrush, NAME_None);

#undef LOCTEXT_NAMESPACE
}