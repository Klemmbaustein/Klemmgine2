#include <KlemmginePlugin.hpp>
#include <Engine/Input.h>
#include <DebugUI.kui.hpp>

using namespace engine;

static uint32 FPS = 0;
static uint32 FPSCounter = 0;
static float Time = 0;
static bool UpdateFPS = false;

class DebugUICanvas : public plugin::PluginCanvasInterface
{
	size_t LastLogSize = 0;
	PluginDebugOverlay* Overlay = nullptr;
	PluginDebugConsole* Console = nullptr;

	void Begin() override
	{
		Overlay = new PluginDebugOverlay();
		UIObject->AddChild(Overlay);
	}

	~DebugUICanvas() override
	{
	}

	void UpdateLogEntries(plugin::EnginePluginInterface* Interface)
	{
		size_t MessageSize = 0;
		plugin::LogEntry* Entries = Interface->GetLogMessages(&MessageSize);
		Console->bg->DeleteChildren();
		for (int64 i = int64(MessageSize) - 1; i >= std::max(int64(0), int64(MessageSize - 29)); i--)
		{
			string Displayed;

			for (size_t j = 0; j < Entries[i].PrefixSize; j++)
			{
				Displayed.append(str::Format("[%s]: ", Entries[i].Prefixes[j].Text));
			}

			Displayed.append(Entries[i].Message);

			Console->bg->AddChild(new kui::UIText(12_px, 1, Displayed,
				Overlay->GetParentWindow()->Markup.GetFont("mono")));
		}
		LastLogSize = MessageSize;
	}

	void Update() override
	{
		auto Interface = plugin::GetInterface();

		if (UpdateFPS && !Interface->IsEditorActive())
		{
			Overlay->fpsText->SetText(str::Format("FPS: %i", int(FPS)));
			UpdateFPS = false;
		}

		if (Interface->GameHasFocus() && Interface->InputIsKeyPressed(int(input::Key::RETURN)) && !Console)
		{
			Console = new PluginDebugConsole();
			UIObject->AddChild(Console);

			Console->field->Edit();
			Console->field->OnChanged = [this] {
				ProcessCommand();
			};

			UpdateLogEntries(Interface);
		}

		if (Console && LastLogSize != Interface->GetLogSize())
		{
			UpdateLogEntries(Interface);
		}
	}

	void ProcessCommand()
	{
		auto Interface = plugin::GetInterface();
		string Text = Console->field->GetText();

		if (Text.empty() || !Interface->InputIsKeyDown(int(input::Key::RETURN)))
		{
			delete Console;
			Console = nullptr;
			return;
		}

		log::Info("> " + Text);
		Console->field->SetText("");
		Console->field->Edit();
		Interface->ConsoleExecuteCommand(Text.c_str());
	}
};

ENGINE_EXPORT void RegisterTypes()
{
}

ENGINE_EXPORT void OnSceneLoaded(engine::Scene* New)
{
	kui::UIContext::SetActive(plugin::GetInterface()->GetUIContext());

	plugin::GetInterface()->GetMainWindow()->SetWindowActive();
	plugin::GetInterface()->CreateUICanvas(new DebugUICanvas());
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