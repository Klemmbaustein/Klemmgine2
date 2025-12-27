#pragma once
#include <Engine/Objects/Reflection/ObjectReflection.h>
#include <map>

namespace engine::editor
{
	/**
	 * @brief
	 * Manages icons displayed in the editor.
	 */
	class EditorIcons
	{
	public:

		/**
		 * @brief
		 * Adds a file icon.
		 * @param IconPath
		 * The path of the icon image file.
		 * @param ObjType
		 * The type of the icon, usually the extension of the file. (kmdl, kts, png, ...)
		 */
		void AddObjectIcon(string IconPath, ObjectTypeID ObjType);

		/**
		 * @brief
		 * Gets the icon from the given type.
		 * @param ObjType
		 * @return
		 * The icon to show for this type, or the default one if none was found.
		 */
		string GetObjectIcon(ObjectTypeID ObjType);

	private:
		std::map<ObjectTypeID, string> Icons;
		static constexpr const char* DEFAULT_ICON = "Object.png";
	};
}