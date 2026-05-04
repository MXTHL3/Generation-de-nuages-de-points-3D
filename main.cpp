#include <gtkmm.h>
#include "main_window.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(
        argc,
        argv,
        "org.demo.gtkmm"
    );

    MainWindow window;

    return app->run(window);
}