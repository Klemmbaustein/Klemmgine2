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

	void Begin() override
	{
	}

	~DebugUICanvas() override
	{
		auto Interface = plugin::GetInterface();
		Interface->DeleteUIBox(ConsoleBox);
	}

	void Update() override
	{
		auto Interface = plugin::GetInterface();

		if (UpdateFPS)
		{
			Interface->UITextSetText(
				Interface->GetCanvasChild(UIObject, "fpsText"),
				str::Format("FPS: %i", int(FPS)).c_str());
			UpdateFPS = false;
		}

		if (Interface->InputIsKeyDown(int(input::Key::RETURN)) && !ConsoleBox)
		{
			ConsoleBox = Interface->CreateUIBox("Console", UIObject);

			auto TextField = Interface->GetDynamicChild(ConsoleBox, "field");

			Interface->UITextFieldEdit(TextField);
			Interface->UITextFieldOnChanged(TextField, [](void* UserData)
				{
					DebugUICanvas* Canvas = (DebugUICanvas*)UserData;
					Canvas->ProcessCommand();
				}, this);
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

		Interface->ConsoleExecuteCommand(Text.c_str());
		Interface->UITextFieldSetText(TextField, "");
		Interface->UITextFieldEdit(TextField);
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
	child UIText sceneText
	{
		text = "Scene: probably";
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
		textSize = 14px;
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
		FPS = std::round(float(FPSCounter) / Time);
		FPSCounter = 0;
		Time = 0;
		UpdateFPS = true;
	}
}