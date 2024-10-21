#include "Scene.h"
#include <GL/glew.h>
#include "Engine/Subsystem/VideoSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/Input.h"
#include <glm/glm.hpp>
#include "Stats.h"
#include "Graphics/Drawable/Model.h"
#include <iostream>

engine::Scene::Scene()
{
	using namespace graphics;
	Buffer = new Framebuffer();

	Cam = new Camera(1);
	Cam->Position.Z = 2;

	Drawables.push_back(new graphics::Model());

	Engine::GetSubsystem<subsystem::VideoSubsystem>()->OnResizedCallbacks.push_back(
		[this](kui::Vec2ui NewSize)
		{
			Buffer->Resize(NewSize.X, NewSize.Y);
		});
}

void engine::Scene::Draw()
{	
	Cam->Update();

	Buffer->Bind();

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto& i : Drawables)
	{
		i->Draw(Cam);
	}
}

void engine::Scene::Update()
{
	if (input::IsKeyDown(input::Key::w))
	{
		Cam->Position += Vector3::Forward(Cam->Rotation) * Vector3(stats::DeltaTime);
	}
	if (input::IsKeyDown(input::Key::s))
	{
		Cam->Position -= Vector3::Forward(Cam->Rotation) * Vector3(stats::DeltaTime);
	}
	if (input::IsKeyDown(input::Key::d))
	{
		Cam->Position += Vector3::Right(Cam->Rotation) * Vector3(stats::DeltaTime);
	}
	if (input::IsKeyDown(input::Key::a))
	{
		Cam->Position -= Vector3::Right(Cam->Rotation) * Vector3(stats::DeltaTime);
	}

	Cam->Rotation = Cam->Rotation + Vector3(input::MouseMovement.Y, input::MouseMovement.X, 0);
}
