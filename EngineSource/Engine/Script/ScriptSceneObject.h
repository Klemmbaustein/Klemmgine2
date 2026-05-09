#include <Engine/Objects/SceneObject.h>
#include <ds/reflection.hpp>
#include <Engine/Script/ScriptObject.h>

namespace engine::script
{
	/// Object running script logic
	class ScriptSceneObject : public SceneObject, public ScriptObject
	{
	public:

		ScriptSceneObject(const ds::TypeInfo& Class, ds::InterpretContext* Interpreter);

		void LoadScriptData() override;
		void InitializeScriptPointer() override;

		void Begin() override;
		void OnDestroyed() override;
		void Update() override;
		void BeginHotReload() override;
		void EndHotReload(ds::ReflectInfo* ClassData) override;
		void InitializePropertyFlags(ObjPropertyBase* p, const string& FlagsString);

	private:
		void LoadProperties();
	};
}