#pragma once
#include <Core/Networking/Http.h>
#include <functional>

namespace engine::http::internal
{
	HttpResponse* HandleConnection(std::function<int64(int64 toRead, void* to)> readFunction);
}