#pragma once
#include <Core/Types.h>
#include <optional>
#include <fstream>

namespace engine::cSharp
{
	class XmlTag
	{
	public:
		string Key;
		string Value;
	};

	class XmlNode
	{
	public:
		string Name;
		string Content;
		std::vector<XmlTag> Tags;
		std::vector<XmlNode> Children;

		void Write(std::ofstream& Out, uint32 Depth);
	};

	class XmlProjectWriter
	{
	public:
		XmlProjectWriter(std::optional<string> SdkVersion);

		XmlNode& AddPropertyGroup(std::optional<string> Condition);
		XmlNode& AddItemGroup();

		void Write(string Path);

		XmlNode RootNode;
	};
}