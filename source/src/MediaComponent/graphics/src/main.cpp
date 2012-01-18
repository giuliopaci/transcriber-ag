///////////////////////////////
// -----------------
// PutPixel - Sample
// -----------------
//
// main.cpp
//
// Author : ferry@bertin.fr
// 2011 (c) Bertin Technologies
///////////////////////////////

#include "GraphicsCanvas.h"

/**
 * PutPixel API - Sample example (main)
 */
int main(int argc, char** argv)
{
    /** Gtk main thread */
    Gtk::Main kit(argc, argv);

    /** Items resources (NB : PNG file is available in project 'resources' directory) */
    Glib::ustring img = "/tmp/france24.png";
    Glib::ustring str1 = "France24 International News";
    Glib::ustring str2 = "Angela Merkel";
    Glib::ustring str3 = "Christophe Robeet";

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(img, 960, 768, true);

    /** Main window instance */
	Gtk::Window* win = new Gtk::Window();
    GraphicsCanvas* canvas = new GraphicsCanvas();


    /*********************
    /*    API Example    *
    /*********************/

    /** Items creation */
    Gnome::Canvas::Text* text1 = canvas->addText(str1, 480, 20);
    Gnome::Canvas::Text* text2 = canvas->addText(str2, 260, 530);
    Gnome::Canvas::Text* text3 = canvas->addText(str3, 710, 530);

    Gnome::Canvas::Rect* rect1 = canvas->addRectangle(160, 200, 280, 320);
    Gnome::Canvas::Rect* rect2 = canvas->addRectangle(660, 150, 820, 370);

    Gnome::Canvas::Pixbuf* pbuf = canvas->addPixbuf(pixbuf,0, 0);

    /** Items settings */

    /** Colors */
    text1->property_fill_color() = "red";
    text2->property_fill_color() = "red";
    text3->property_fill_color() = "red";

    rect1->property_outline_color() = "red";
    rect2->property_outline_color() = "red";

    /** Sizes */
    text1->property_size_points() = 20;
    text2->property_size_points() = 20;
    text3->property_size_points() = 20;

    rect1->property_width_pixels() = 4;
    rect2->property_width_pixels() = 4;

    /** Main image lowered as 'background' */
    pbuf->lower_to_bottom();

    /*********************
    /*   /API Example    *
    /*********************/

    /** Canvas */
    canvas->set_size_request(960, 768);

    /** Window settings (final layout) */
    win->add(*canvas);
    win->set_default_size(960, 768);
    win->show_all_children();


	// -- Startup --
	kit.run(*win);
	return 0;
}

