#pragma once
#include "IDialogWindow.h"
#include <kui/UI/UIText.h>
#include <mutex>
#include <memory>
#include <Editor/Build/BuildProject.h>

namespace engine::editor
{
	class BuildWindow : public IDialogWindow
	{
	public:
		BuildWindow();

		void Begin() override;
		void Update() override;
		void Destroy() override;

	private:

		struct BuildJob
		{
			void Update();

			void Start();

			void AddLine(string NewLine, BuildStage Stage);

			void BuildDone();
			void NextBuildStage(string StageName);
			void StartBuildThread();

			string CurrentStatus;
			std::mutex CurrentStatusMutex;

			BuildOptions Options;

			BuildWindow* Parent = nullptr;
			kui::UIText* StatusText = nullptr;
			kui::UIBox* ProgressBox = nullptr;
			kui::UIBox* JobBackground = nullptr;

			BuildStage CurrentStage = BuildStage::Configure;
			BuildStage OldStage = BuildStage::Configure;
			string CurrentStageName;
			bool IsDone = false;

			kui::Timer BuildTimer;
		};

		void StartBuildForPlatform(BuildPlatform Platform, kui::UIScrollBox* ParentScrollBox);
		
		std::vector<std::shared_ptr<BuildJob>> Jobs;

		void AddBuildHeader(string Message, kui::UIBox* To);
		void AddBuildText(string Text, kui::UIBox* To);
		void AddCheckbox(string Name, bool& Value, kui::UIBox* To);
		void AddElement(string Name, kui::UIBox* Element, kui::UIBox* To);

		static string GetPlatformDisplayName(BuildPlatform Platform);

		void AddSeparator(kui::UIBox* To);

#if WINDOWS
		bool BuildForWindows = true;

		bool BuildForWindowsArm = false;
#endif
		bool BuildForLinux = false;

		bool CompressAssets = true;
		bool IncludeDevPlugins = false;
		bool MultiThreaded = true;


		void StartBuild();
	};
}