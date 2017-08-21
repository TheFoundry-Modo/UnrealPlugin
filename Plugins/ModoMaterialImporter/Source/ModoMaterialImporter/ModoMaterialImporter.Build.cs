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
 
using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules
{
    public class ModoMaterialImporter : ModuleRules
    {
        public ModoMaterialImporter(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivateIncludePaths.AddRange(new string[] { "ModoMaterialImporter/Private" });
            PublicIncludePaths.AddRange(new string[] { "ModoMaterialImporter/Public" });

            // Workaround for inconsistency of PCH rules between Engine Plugin and Project Plugin
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PublicDependencyModuleNames.AddRange(new string[] { "Engine", "Core", "LevelEditor", "Slate", "SlateCore", "EditorStyle", "XmlParser", "ImageWrapper", "CoreUObject", "UnrealEd", "MaterialEditor", "DesktopPlatform", "Projects", "InputCore", "ContentBrowser" });
        }
    }
}