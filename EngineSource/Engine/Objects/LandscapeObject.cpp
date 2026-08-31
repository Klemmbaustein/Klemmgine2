#include "LandscapeObject.h"
#include <Editor/UI/EditorUI.h>

void engine::LandscapeObject::Begin()
{
	Component = new LandscapeComponent();

	Component->LandscapeMaterial = new graphics::Material(Material.Value);

	Material.OnChanged = [this] {
		delete Component->LandscapeMaterial;
		Component->LandscapeMaterial = new graphics::Material(Material.Value);
	};

	Component->LodFalloff = LodFalloff.Value;

	LodFalloff.OnChanged = [this] {
		Component->LodFalloff = LodFalloff.Value;
		Component->IsDirty = true;
	};

	this->Attach(Component);

	Component->Load(this->HeightMap.Value);

	this->HeightMap.OnChanged = [this] {
		Component->Load(this->HeightMap.Value);
	};
}

void engine::LandscapeObject::OnDestroyed()
{
}

void engine::LandscapeObject::OnSaved()
{
#if EDITOR
	if (Component && Component->Data && Component->Data->Changed && HeightMap.Value.IsValid())
	{
		auto Stream = editor::EditorUI::Instance->AssetsProvider->GetFileSaveStream(HeightMap.Value.FilePath);

		Component->Data->SaveToStream(Stream);
		Component->Data->Changed = false;
		delete Stream;
	}
#endif
}
