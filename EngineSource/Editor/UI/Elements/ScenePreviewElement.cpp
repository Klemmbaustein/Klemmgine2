#include "ScenePreviewElement.h"

engine::editor::ScenePreviewElement::ScenePreviewElement(Scene* PreviewScene, kui::SizeVec Size)
	: UIBackground(false, 0, 1, Size), PreviewScene(PreviewScene)
{
	PreviewScene->Physics.Active = false;
	PreviewScene->Graphics.SceneCamera->Rotation = Vector3(-37.5f, 45, 0);
	PreviewScene->Graphics.SceneCamera->Position = Vector3(2.25f);
	PreviewScene->Graphics.Resizable = false;
	PreviewScene->Graphics.AlwaysRedraw = false;
	PreviewScene->Graphics.RedrawNextFrame = true;
}

void engine::editor::ScenePreviewElement::Tick()
{
	PreviewScene->Graphics.AlwaysRedraw = AlwaysRedraw;

	auto Texture = PreviewScene->Graphics.GetDrawBuffer();

	if (Texture)
	{
		SetUseImage(true, Texture->GetUITexture());
	}
	if (RedrawNextFrame || AlwaysRedraw)
	{
		RedrawElement();
		RedrawNextFrame = false;
	}
}

void engine::editor::ScenePreviewElement::Update()
{
	kui::Vec2ui& PixelSize = PreviewScene->Graphics.BufferSize;

	kui::Vec2ui Resolution = GetUsedSize().GetPixels() * ParentWindow->GetDPI();

	if (PixelSize != Resolution)
	{
		PixelSize = Resolution;
		PreviewScene->Graphics.OnResized(Resolution);
	}
}

void engine::editor::ScenePreviewElement::RedrawScene()
{
	RedrawNextFrame = true;
	PreviewScene->Graphics.RedrawNextFrame = true;
}
