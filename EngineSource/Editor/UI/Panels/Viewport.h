#ifdef EDITOR
#pragma once
#include "EditorPanel.h"
#include <Editor/UI/Elements/Toolbar.h>
#include <Engine/Objects/Components/MeshComponent.h>
#include <Engine/Objects/SceneObject.h>
#include <Engine/Subsystem/SceneSubsystem.h>
#include <kui/Timer.h>
#include <kui/UI/UIText.h>
#include <set>
#include <stack>

namespace engine::editor
{
	class Viewport : public EditorPanel
	{
	public:
		Viewport();
		~Viewport() override;
		bool MouseGrabbed = false;
		bool UnsavedChanges = false;

		kui::Timer StatsRedrawTimer;
		bool RedrawStats = false;
		uint64 FameCount = 0;

		static Viewport* Current;

		void OnResized() override;
		void Update() override;
		void OnThemeChanged() override;
		void RemoveSelected();
		void ClearSelected();

		void SceneChanged();
		void SaveCurrentScene();

		void OnObjectChanged(SceneObject* Target);
		void OnObjectsChanged(std::vector<SceneObject*> Targets);
		void OnObjectCreated(SceneObject* Target);

		struct Change
		{
			SceneObject* Object = nullptr;
			SerializedValue ObjectData;
		};

		struct Changes
		{
			std::vector<Change> ChangeList;
		};

		std::stack<Changes> ObjectChanges;
		std::set<SceneObject*> SelectedObjects;
		kui::UIBackground* ViewportBackground = nullptr;

		void UndoLast();

	private:

		void Run();

		void HighlightObject(SceneObject* Target, bool Highlighted);
		void HighlightComponents(DrawableComponent* Target, bool Highlighted);

		physics::HitResult RayAtCursor();

		void UndoChange(Change& Target, Scene* Current);
		void UpdateSelection();
		static void HandleKeyPress(kui::Window* w);

		bool PolledForText = false;
		bool Undoing = false;
		bool SceneLoaded = false;

		Toolbar* ViewportToolbar = nullptr;
		MeshComponent* GizmoMesh = nullptr;

		kui::UIText* ViewportStatusText = nullptr;
		kui::UIBox* StatusBarBox = nullptr;
		kui::UIBackground* LoadingScreenBox = nullptr;
	};
}
#endif