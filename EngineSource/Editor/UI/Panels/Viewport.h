#pragma once
#include "EditorPanel.h"
#include <Editor/UI/Elements/Toolbar.h>
#include <Editor/UI/Gizmos/TranslateGizmo.h>
#include <Editor/UI/EditorUI.h>
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
			ObjectID Object = 0;
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
		physics::HitResult RayAtCursor(float Distance, float FallbackDistance);

		Vector3 GetCursorDirection();

		void ShiftSelected(Vector3 Direction);
		void Run();

		float GridSize = 0.1f;

	private:

		void OnItemDropped(EditorUI::DraggedItem Item);

		void HighlightObject(SceneObject* Target, bool Highlighted);
		void HighlightComponents(DrawableComponent* Target, bool Highlighted);

		void UndoChange(Change& Target, Scene* Current);
		void UpdateSelection();

		bool PolledForText = false;
		bool SceneLoaded = false;

		Toolbar* ViewportToolbar = nullptr;
		MeshComponent* Grid = nullptr;
		TranslateGizmo* Translate = nullptr;

		kui::UIText* ViewportStatusText = nullptr;
		kui::UIBox* StatusBarBox = nullptr;
		kui::UIBackground* LoadingScreenBox = nullptr;
	};
}