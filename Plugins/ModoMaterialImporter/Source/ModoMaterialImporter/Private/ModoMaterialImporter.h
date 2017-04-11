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
 
#include "ModuleManager.h"
#include "SharedPointer.h"

#include "ModoMaterialImporterCommands.h"

#include "Internationalization.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "AssertionMacros.h"
#include "AssetEditorToolkit.h"

class ModoMaterialImporterModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void ButtonClicked();
	void OnAssetPathChanged(const FString& newPath);
	
private:

	void AddToolbarExtension(FToolBarBuilder &);

	FString	 mLastPath;
	FString	 mContentBrowserPath;

	TSharedPtr< const FExtensionBase > _ToolbarExtension;
	TSharedPtr<FExtensibilityManager> _ExtensionManager;
	TSharedPtr<FUICommandList> _PluginCommands;
	TSharedPtr<FExtender> _ToolbarExtender;
};

