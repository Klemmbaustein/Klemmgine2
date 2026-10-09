#include "XmlProjectWriter.h"

using namespace engine::cSharp;

XmlNode& engine::cSharp::XmlProjectWriter::AddPropertyGroup(std::optional<string> Condition)
{
	XmlNode& NewNode = this->RootNode.Children.emplace_back();
	NewNode.Name = "PropertyGroup";
	if (Condition)
	{
		NewNode.Tags = { XmlTag("Condition", *Condition) };
	}
	return NewNode;
}

XmlNode& engine::cSharp::XmlProjectWriter::AddItemGroup()
{
	XmlNode& NewNode = this->RootNode.Children.emplace_back();
	NewNode.Name = "ItemGroup";
	return NewNode;
}

void engine::cSharp::XmlProjectWriter::Write(string Path)
{
	std::ofstream Out = std::ofstream(Path);

	RootNode.Write(Out, 0);
}

engine::cSharp::XmlProjectWriter::XmlProjectWriter(string SdkVersion)
{
	RootNode = XmlNode{
		.Name = "Project",
		.Tags = {XmlTag("Sdk", SdkVersion)},
	};
}

void engine::cSharp::XmlNode::Write(std::ofstream& Out, uint32 Depth)
{
	auto WriteIndent = [&Out, &Depth](uint32 Offset) {
		for (uint32 i = 0; i < Offset + Depth; i++)
		{
			Out << '\t';
		}
	};

	WriteIndent(0);
	Out << "<" << this->Name;
	for (const auto& i : this->Tags)
	{
		Out << " " << i.Key << "=\"" << i.Value << "\"";
	}
	if (Content.empty() && Children.empty())
	{
		Out << "/>\n";
		return;
	}

	if (!Children.empty())
	{
		Out << ">\n";
		for (auto& i : this->Children)
		{
			i.Write(Out, Depth + 1);
		}

		if (!Content.empty())
		{
			Out << Content << "\n";
		}
		WriteIndent(0);
	}
	else
	{
		Out << ">" << Content;
	}

	Out << "</" << this->Name << ">\n";
}
