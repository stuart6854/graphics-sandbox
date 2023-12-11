#include "Core/App.hpp"
#include "Core/Logging.hpp"

int main(int /*argc*/, char** /*argv*/)
{
	LOG_INFO("Graphics Sandbox");

	App app{};
	app.Run();

	return 0;
}