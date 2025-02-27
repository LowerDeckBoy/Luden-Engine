#include "Application.hpp"

int main()
{
	auto app = std::make_unique<Luden::Application>();
	app->Run();
	//Luden::Application app;
	//app.Run();

}
