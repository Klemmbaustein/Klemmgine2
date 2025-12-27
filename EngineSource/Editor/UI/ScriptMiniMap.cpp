#include "ScriptMiniMap.h"
#include <kui/Image.h>
#include <kui/UI/UIButton.h>
#include <Core/ThreadPool.h>
#include <Engine/MainThread.h>
#include <Core/Log.h>

using namespace engine::editor;
using namespace kui;

ScriptMiniMap::ScriptMiniMap(kui::UITextEditor* Editor, ScriptEditorProvider* Provider)
{
	this->Editor = Editor;
	this->Provider = Provider;

	Provider->OnUpdated.Add(this, [this] {
		ReGenerate = true;
	});

	this->BackgroundColor = Editor->EditorScrollBox->GetScrollBarBackground()->GetColor();

	Editor->EditorScrollBox->UseDefaultColors = false;
	Editor->EditorScrollBox->GetScrollBarBackground()
		->SetHoveredColor(1)
		->SetPressedColor(1)
		->SetColor(1)
		->SetBorder(0, 1);
	Editor->EditorScrollBox->ScrollBarWidth = 60;
	Editor->EditorScrollBox->GetScrollBarSlider()->SetOpacity(0.5f);
	Editor->UpdateContent();
	ReGenerate = true;
}

engine::editor::ScriptMiniMap::~ScriptMiniMap()
{
	if (Image)
	{
		image::UnloadImage(Image);
	}
	Editor->EditorScrollBox->UseDefaultColors = true;
	Editor->EditorScrollBox->ScrollBarWidth = 10;
	Editor->EditorScrollBox->ScrollDownPadding = 0;
	Editor->EditorScrollBox->GetScrollBarBackground()->SetUseImage(false);
	Editor->EditorScrollBox->Update();
}

void ScriptMiniMap::Update()
{
	if (OldLength != Editor->GetLoadedLines())
	{
		OldLength = Editor->GetLoadedLines();
		ReGenerate = true;
	}

	if (ReGenerate && !IsGenerating)
	{
		ReGenerate = false;
		OldLength = Editor->GetLoadedLines();

		uint32 h = Editor->EditorScrollBox->GetUsedSize().GetPixels().Y;

		ThreadPool::Main()->AddJob([this, h] {
			IsGenerating = true;

			GenerateTexture(h);

			thread::ExecuteOnMainThread([this] {
				if (Image)
				{
					image::UnloadImage(Image);
				}
				Image = image::LoadImage(this->Texture.data(), Width, Height * 4);
				this->Editor->EditorScrollBox->GetScrollBarBackground()->SetUseImage(true, Image);
				IsGenerating = false;
				Editor->EditorScrollBox->ScrollDownPadding = DownPadding;
				this->Editor->EditorScrollBox->RedrawElement();
			});
		});
	}
}

void engine::editor::ScriptMiniMap::GenerateTexture(uint32 ScrollBoxHeight)
{
	Width = 60;
	Height = Provider->GetLineCount();

	if (Height * 4 < ScrollBoxHeight)
	{
		Height = ScrollBoxHeight / 4;
		size_t Remnant = Height - Provider->GetLineCount();

		DownPadding = Remnant * 4;
	}
	else
	{
		DownPadding = 0;
	}

	this->Texture.resize((Width * 4) * (Height * 4));

	BackgroundB = uByte(BackgroundColor.Z * 255.0f);
	BackgroundR = uByte(BackgroundColor.X * 255.0f);
	BackgroundG = uByte(BackgroundColor.Y * 255.0f);

	for (size_t y = 0; y < Height; y++)
	{
		std::vector<TextSegment> l;

		Editor->Get(EditorPosition(0, y), SIZE_MAX, l, true, true);

		TextSegment* CurrentSegment = l.size() ? & *l.begin() : nullptr;

		size_t CurrentCharIndex = 0;
		size_t CurrentSegmentIndex = 0;

		for (size_t x = 0; x < Width; x++)
		{
			if (CurrentSegment && CurrentCharIndex >= CurrentSegment->Text.size())
			{
				CurrentSegmentIndex++;
				CurrentCharIndex = 0;
				CurrentSegment = (l.size() > CurrentSegmentIndex) ? &l[CurrentSegmentIndex] : nullptr;
			}

			char c = 0;

			if (CurrentSegment)
			{
				c = CurrentSegment->Text[CurrentCharIndex];
				CurrentCharIndex++;
			}

			if (c == '\t')
			{
				for (size_t i = 0; i <= 4; i++)
				{
					SetPixel(x, y, BackgroundR, BackgroundG, BackgroundB);
					x++;
				}
				x--;
			}

			else if (CurrentSegment && c != ' ')
			{
				Vec3f Color = CurrentSegment->Color;

				SetPixel(x, y, Color.X * 255.0f, Color.Y * 255.0f, Color.Z * 255.0f);
			}
			else
			{
				SetPixel(x, y, BackgroundR, BackgroundG, BackgroundB);
			}
		}
	}
}

void engine::editor::ScriptMiniMap::SetPixel(size_t x, size_t y, uByte R, uByte G, uByte B)
{
	y *= 4;

	for (size_t yOff = y; yOff < y + 3; yOff++)
	{
		size_t Offset = ((Height * 4 - yOff - 1) * Width + x) * 4;

		Texture[Offset] = R;
		Texture[Offset + 1] = G;
		Texture[Offset + 2] = B;
		Texture[Offset + 3] = 255;
	}

	size_t Offset = ((Height * 4 - (y + 4)) * Width + x) * 4;
	Texture[Offset] = BackgroundR;
	Texture[Offset + 1] = BackgroundG;
	Texture[Offset + 2] = BackgroundB;
	Texture[Offset + 3] = 255;
}
