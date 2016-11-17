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
#include "ModoMaterialImporterCommands.h"

#define LOCTEXT_NAMESPACE "ModoMaterialImporter"
 
PRAGMA_DISABLE_OPTIMIZATION
void ModoMaterialImporterCommands::RegisterCommands()
{
	UI_COMMAND(Button, "ModoMaterialImporter", "Import a modo material description file (XML)", EUserInterfaceActionType::Button, FInputGesture());
}
PRAGMA_ENABLE_OPTIMIZATION
