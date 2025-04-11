#include <KlemmginePlugin.hpp>
#include <Engine/Input.h>

using namespace engine;

static uint32 FPS = 0;
static uint32 FPSCounter = 0;
static float Time = 0;
static bool UpdateFPS = false;

class DebugUICanvas : public plugin::PluginCanvasInterface
{
	plugin::kuiUIBox* ConsoleBox = nullptr;
	size_t LastLogSize = 0;
	plugin::kuiUIBox* ConsoleBackground = nullptr;

	void Begin() override
	{
	}

	~DebugUICanvas() override
	{
		auto Interface = plugin::GetInterface();
		Interface->DeleteUIBox(ConsoleBox);
	}

	void UpdateLogEntries(plugin::EnginePluginInterface* Interface)
	{
		size_t MessageSize = 0;
		plugin::LogEntry* Entries = Interface->GetLogMessages(&MessageSize);
		Interface->DeleteUIBoxChildren(ConsoleBackground);
		for (int64 i = int64(MessageSize) - 1; i >= std::max(int64(0), int64(MessageSize - 29)); i--)
		{
			string Displayed;

				for (size_t j = 0; j < Entries[i].PrefixSize; j++)
				{
					Displayed.append(str::Format("[%s]: ", Entries[i].Prefixes[j].Text));
				}

				Displayed.append(Entries[i].Message);

			auto Text = Interface->CreateUIBox("ConsoleEntry", UIObject);
			Interface->UITextSetText(Interface->GetDynamicChild(Text, "text"), Displayed.c_str());
			Interface->AddChild(ConsoleBackground, Text);
		}
		LastLogSize = MessageSize;
	}

	void Update() override
	{
		auto Interface = plugin::GetInterface();

		if (UpdateFPS && !Interface->IsEditorActive())
		{
			Interface->UITextSetText(
				Interface->GetCanvasChild(UIObject, "fpsText"),
				str::Format("FPS: %i", int(FPS)).c_str());
			UpdateFPS = false;
		}

		if (Interface->GameHasFocus() && Interface->InputIsKeyDown(int(input::Key::RETURN)) && !ConsoleBox)
		{
			ConsoleBox = Interface->CreateUIBox("Console", UIObject);
			ConsoleBackground = Interface->GetDynamicChild(ConsoleBox, "bg");

			auto TextField = Interface->GetDynamicChild(ConsoleBox, "field");

			Interface->UITextFieldEdit(TextField);
			Interface->UITextFieldOnChanged(TextField, [](void* UserData)
				{
					DebugUICanvas* Canvas = (DebugUICanvas*)UserData;
					Canvas->ProcessCommand();
				}, this);
			UpdateLogEntries(Interface);
		}

		if (ConsoleBox && LastLogSize != Interface->GetLogSize())
		{
			UpdateLogEntries(Interface);
		}
	}

	void ProcessCommand()
	{
		auto Interface = plugin::GetInterface();
		auto TextField = Interface->GetDynamicChild(ConsoleBox, "field");
		string Text = Interface->UITextFieldGetText(TextField);

		if (Text.empty() || !Interface->InputIsKeyDown(int(input::Key::RETURN)))
		{
			Interface->DeleteUIBox(ConsoleBox);
			ConsoleBox = nullptr;
			return;
		}

		log::Info("> " + Text);
		Interface->UITextFieldSetText(TextField, "");
		Interface->UITextFieldEdit(TextField);
		Interface->ConsoleExecuteCommand(Text.c_str());
	}
};

ENGINE_EXPORT void RegisterTypes()
{
}

ENGINE_EXPORT void OnSceneLoaded(engine::Scene* New)
{
	plugin::GetInterface()->CreateUICanvas("Overlay", R"(
element Overlay
{
	width = 100%;
	height = 100%;

	padding = 5px;

	orientation = vertical;

	child UIText fpsText
	{
		text = "";
		color = (1, 1, 0.2);
		size = 15px;
	}

	child UIBox consoleBox
	{}
}

element Console
{
	position = -1;
	size = 2;

	verticalAlign = default;
	orientation = vertical;

	child UITextField field
	{
		textColor = 1;
		color = 0.1;
		width = 2;
		textSize = 12px;
		font = "mono";
	}

	child UIBackground bg
	{
		border = 1px;
		borderColor = 0.5;
		leftBorder = false;
		rightBorder = false;

		width = 2;
		color = 0;
		height = 500px;
		opacity = 0.8;
		orientation = vertical;
		verticalAlign = default;
	}
}

element ConsoleEntry
{
	child UIText text
	{
		size = 11px;
		color = 1;
		font = "mono";
		downPadding = 2px;
	}
}

)", new DebugUICanvas());
}

ENGINE_EXPORT void Update(float Delta)
{
	Time += Delta;
	FPSCounter++;

	if (Time >= 0.5f)
	{
		FPS = uint32(std::round(float(FPSCounter) / Time));
		FPSCounter = 0;
		Time = 0;
		UpdateFPS = true;
	}
}