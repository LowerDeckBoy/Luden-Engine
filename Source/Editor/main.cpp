#include "Application.hpp"

int main()
{
	Luden::Application app;
	app.Initialize();
	app.Run();
	app.Shutdown();

	return 0;
}
