#include <gtkmm.h>
#include <memory>

#include "main_window.h"
#include "gl.h"
#include "cgal_shape.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.demo.gtkmm");
    auto scene = std::make_unique<CgalShape>();
    auto gl = std::make_unique<Gl>(std::move(scene));
    MainWindow window(std::move(gl));
    return app->run(window);
}