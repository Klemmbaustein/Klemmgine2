#include "ColorPicker.h"
#include <kui/Resource.h>
#include <Editor/UI/EditorUI.h>
#include <Engine/Graphics/VideoSubsystem.h>
#include <Engine/MainThread.h>

using namespace kui;
using namespace engine;
using namespace engine::editor;

struct RgbColor
{
	uByte r;
	uByte g;
	uByte b;

	Vector3 Vec()
	{
		return Vector3(r, g, b) / 255.0f;
	}
};

static RgbColor HsvToRgb(ColorPicker::HsvColor hsv)
{
	RgbColor rgb{};
	uByte region, remainder, p, q, t;

	if (hsv.s == 0)
	{
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}

	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;

	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

	switch (region)
	{
	case 0:
		rgb.r = hsv.v; rgb.g = t; rgb.b = p;
		break;
	case 1:
		rgb.r = q; rgb.g = hsv.v; rgb.b = p;
		break;
	case 2:
		rgb.r = p; rgb.g = hsv.v; rgb.b = t;
		break;
	case 3:
		rgb.r = p; rgb.g = q; rgb.b = hsv.v;
		break;
	case 4:
		rgb.r = t; rgb.g = p; rgb.b = hsv.v;
		break;
	default:
		rgb.r = hsv.v; rgb.g = p; rgb.b = q;
		break;
	}

	return rgb;
}

static ColorPicker::HsvColor RgbToHsv(RgbColor rgb)
{
	ColorPicker::HsvColor hsv{};
	uByte rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

	hsv.v = rgbMax;
	if (hsv.v == 0)
	{
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}

	hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
	if (hsv.s == 0)
	{
		hsv.h = 0;
		return hsv;
	}

	if (rgbMax == rgb.r)
		hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if (rgbMax == rgb.g)
		hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

	return hsv;
}


engine::editor::ColorPicker::ColorPicker(Vector3 RgbColorValue, std::function<void(Vector3)> OnSubmit,
	std::function<void(Vector3)> OnChange)
	: IDialogWindow("Color picker", { Option{
		.Name = "Ok",
		.OnClicked = [this] {
			if (this->OnSubmit)
			{
				this->OnSubmit(this->RgbColorValue);
			}
		},
		.Close = true,
		},
		Option{
		.Name = "Cancel",
		.OnClicked = [this] {
			if (this->OnSubmit)
			{
				this->OnSubmit(this->InitialValue);
			}
		},
		.Close = true,
		} }, Vec2ui(380, 280))
{
	this->OnSubmit = OnSubmit;
	this->OnChange = OnChange;
	this->InitialValue = RgbColorValue;
	this->RgbColorValue = RgbColorValue;
	this->SelectedHsv = RgbToHsv(RgbColor(RgbColorValue.X * 255.0f, RgbColorValue.Y * 255.0f, RgbColorValue.Z * 255.0f));
	Open();
}

void engine::editor::ColorPicker::Begin()
{
	IDialogWindow::Begin();
	VideoSubsystem::Current->InitGLErrors();

	PickerShader = new Shader(
		resource::GetStringFile("res:shaders/uishader.vert"),
		resource::GetStringFile("res:ui/colorPicker.frag"));

	HueShader = new Shader(
		resource::GetStringFile("res:shaders/uishader.vert"),
		resource::GetStringFile("res:ui/colorPicker.frag"));

	Background->SetHorizontal(true);

	this->PickerBackground = new UIBackground(true, 0, 1, 200_px, PickerShader);

	Background->AddChild(PickerBackground
		->SetPadding(20_px));

	this->HueBackground = new UIBackground(true, 0, 1, SizeVec(32_px, 200_px), HueShader);

	Background->AddChild(HueBackground
		->SetPadding(20_px, 20_px, 0, 0));
	PickerShader->Bind();
	PickerShader->SetInt("u_mode", 0);
	PickerShader->SetFloat("u_selectedHue", SelectedHsv.h / 255.0f);
	PickerShader->SetVec2("selectedPos", Vec2f(SelectedHsv.s, SelectedHsv.v) / 255.0f);
	HueShader->Bind();
	HueShader->SetInt("u_mode", 1);
	HueShader->SetFloat("u_selectedHue", SelectedHsv.h / 255.0f);

	UIBox* SideBox = new UIBox(false);

	std::array<char, 3> ColorComponents = {
		'R',
		'G',
		'B'
	};

	for (uint32 i = 0; i < 3; i++)
	{
		UIBox* b = new UIBox(true);
		b->SetVerticalAlign(UIBox::Align::Centered);

		auto f = new UITextField(0, EditorUI::Theme.DarkBackground, DefaultFont, [this, i] {
			try
			{
				this->RgbColorValue[i] = std::stof(this->PartFields[i]->GetText());
				this->SelectedHsv = RgbToHsv(RgbColor(RgbColorValue.X * 255.0f,
					RgbColorValue.Y * 255.0f, RgbColorValue.Z * 255.0f));
				UpdateColor(false);
				HueBackground->RedrawElement();
			}
			catch (std::exception& e)
			{

			}
		});

		f->SetText(str::FloatToString(RgbColorValue[i]));
		f->SetTextSize(12_px);

		SideBox->AddChild(b
			->AddChild((new UIText(12_px, EditorUI::Theme.Text, { ColorComponents[i], ':' }, DefaultFont))
				->SetTextWidthOverride(20_px))
			->AddChild(f
				->SetMinWidth(60_px))
			->SetPadding(5_px));

		this->PartFields[i] = f;
	}

	SideBox->AddChild((new UIText(12_px, EditorUI::Theme.Text, "Selected:", DefaultFont))
		->SetPadding(5_px, 0, 5_px, 5_px));

	PreviewColor = new UIBackground(true, 0, Vec3f(RgbColorValue.X, RgbColorValue.Y, RgbColorValue.Z), SizeVec(80_px, 30_px));

	SideBox->AddChild(PreviewColor
		->SetPadding(5_px));

	Background->AddChild(SideBox
		->SetPadding(10_px));
}

void engine::editor::ColorPicker::Update()
{
	auto Win = Window::GetActiveWindow();

	if (this->PickerBackground->IsBeingHovered() && Win->Input.IsLMBClicked)
	{
		DraggingPicker = true;
	}
	else if (!Win->Input.IsLMBDown)
	{
		DraggingPicker = false;
	}

	if (DraggingPicker && Background->IsBeingHovered())
	{
		Vec2f Relative = Win->Input.MousePosition - this->PickerBackground->GetScreenPosition();
		Relative = (Relative / this->PickerBackground->GetUsedSize().GetScreen()).Clamp(0, 1);

		uByte NewSaturation = Relative.X * 255;
		uByte NewValue = Relative.Y * 255;

		if (NewValue != SelectedHsv.v || NewSaturation != SelectedHsv.s)
		{
			SelectedHsv.s = NewSaturation;
			SelectedHsv.v = NewValue;

			PickerShader->Bind();
			PickerShader->SetFloat("u_selectedHue", SelectedHsv.h / 255.0f);
			PickerShader->SetVec2("selectedPos", Relative);

			PickerBackground->RedrawElement();
			UpdateColor();
		}
	}

	if (this->HueBackground->IsBeingHovered() && Win->Input.IsLMBClicked)
	{
		DraggingHue = true;
	}
	else if (!Win->Input.IsLMBDown)
	{
		DraggingHue = false;
	}

	if (DraggingHue && Background->IsBeingHovered())
	{
		Vec2f Relative = Win->Input.MousePosition - this->HueBackground->GetScreenPosition();
		Relative = (Relative / this->HueBackground->GetUsedSize().GetScreen()).Clamp(0, 1);

		uByte NewHue = Relative.Y * 255;

		if (NewHue != SelectedHsv.h)
		{
			SelectedHsv.h = NewHue;

			PickerShader->Bind();
			PickerShader->SetFloat("u_selectedHue", Relative.Y);
			PickerBackground->RedrawElement();

			HueShader->Bind();
			HueShader->SetFloat("u_selectedHue", Relative.Y);
			HueBackground->RedrawElement();
			UpdateColor();
		}
	}
}

void engine::editor::ColorPicker::Destroy()
{
	delete PickerShader;
	delete HueShader;
}

void engine::editor::ColorPicker::UpdateColor(bool UpdateFields)
{
	RgbColorValue = HsvToRgb(SelectedHsv).Vec();

	PreviewColor->SetColor(Vec3f(RgbColorValue.X, RgbColorValue.Y, RgbColorValue.Z));

	if (UpdateFields)
	{
		for (uint32 i = 0; i < 3; i++)
		{
			PartFields[i]->SetText(str::FloatToString(RgbColorValue[i], 3));
		}
	}

	if (OnChange)
	{
		thread::ExecuteOnMainThread(std::bind(OnChange, this->RgbColorValue));
	}
}
