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
 
#include "ModoPluginPCH.h"
#include "ModoPlugin.h"

#define LOCTEXT_NAMESPACE "ModoPlugin"

DEFINE_LOG_CATEGORY(ModoPlugin);

class ModoPluginModule : public IModoPlugin
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(ModoPluginModule, ModoPlugin)

void ModoPluginModule::StartupModule()
{
	// Startup
}

void ModoPluginModule::ShutdownModule()
{
	// Shutdown
}