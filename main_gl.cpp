#include "main_gl.h"
#include <iostream>

MainGL::MainGL() : Gtk::GLArea(){
    // version requise OpenGL 3.0
    set_required_version(3, 0);

    // buffer de profondeur pour affichage 3d
    set_has_depth_buffer(true);
}

void MainGL::on_realize(){
    // Creation de la zone de dessin et du contexte OpenGL
    Gtk::GLArea::on_realize();
    
    // Rend le contexte courant
    make_current();

    // Vérifier le contexte   
    throw_if_error();
}

void MainGL::on_unrealize(){
    make_current();
}

void MainGL::on_resize(int width, int height){
    make_current();

    // redimension zone de dessin
    glViewport(0, 0, width, height);
}

bool MainGL::on_render(const Glib::RefPtr<Gdk::GLContext>& context){
    
    // définition background color
    glClearColor(0.2f, 0.8f, 0.3f, 1.0f);
    
    // update color et z-buffer 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
}