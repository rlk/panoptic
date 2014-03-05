// Copyright (C) 2011-2014 Robert Kooima
//
// PANOPTIC is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.

#include <app-font.hpp>
#include <app-host.hpp>
#include <app-data.hpp>

#include "view-gui.hpp"
#include "view-app.hpp"

//-----------------------------------------------------------------------------

class button_load_data : public gui::button
{
    view_app      *V;
    gui::selector *S;

public:
    button_load_data(view_app *V, gui::selector *S)
        : gui::button("Load Data"), V(V), S(S) { }

    void apply()
    {
        if (!S->value().empty())
            V->load_file(S->value());
    }
};

class button_load_path : public gui::button
{
    view_app      *V;
    gui::selector *S;

public:
    button_load_path(view_app *V, gui::selector *S)
        : gui::button("Load Path"), V(V), S(S) { }

    void apply()
    {
        if (!S->value().empty())
            V->load_path(S->value());
    }
};

class button_cancel : public gui::button
{
    view_app *V;

public:
    button_cancel(view_app *V)
        : gui::button("Cancel"), V(V) { }

    void apply()
    {
        V->cancel();
    }
};

class button_load_config : public gui::button
{
    gui::widget *name;

public:
    button_load_config(gui::widget *n) :
        gui::button("Load Configuration"), name(n) { }

    void apply()
    {
        if (!name->value().empty())
            ::host->reconfig(name->value());
    }
};

//-----------------------------------------------------------------------------
// The About panel

about_panel::about_panel(view_app *V, gui::widget *w) : gui::vgroup()
{
    std::string text;

    try
    {
        text = std::string((const char *) ::data->load("ABOUT.md"));
        ::data->free("ABOUT.md");
    }
    catch (std::runtime_error& e)
    {
        text = "## ABOUT.md not found";
    }

    add((new gui::scroll)->
        add(new gui::pager(text)));
}

//-----------------------------------------------------------------------------
// The Data panel

data_panel::data_panel(view_app *V, gui::widget *w) : gui::vgroup()
{
    gui::selector *S = new gui::selector("scm", ".xml");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(S)->
            add((new gui::harray)->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new button_cancel   (V   ))->
//              add(new button_load_path(V, S))->
                add(new button_load_data(V, S)))));
}

//-----------------------------------------------------------------------------
// The Config panel

config_panel::config_panel(view_app *V, gui::widget *w) : gui::vgroup()
{
    const std::string a((const char *) glGetString(GL_VERSION));
    const std::string b((const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

    const std::string s = "OpenGL " + a + " GLSL " + b;

    gui::selector *S = new gui::selector("host/common", ".xml");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(S)->
            add((new gui::hgroup)->
                add(new gui::filler(true, false))->
                add(new gui::string(s, gui::string::mono, 0, 0, 0, 0))->
                add(new button_load_config(S)))));
}

//-----------------------------------------------------------------------------
// The toplevel control panel

view_gui::view_gui(view_app *V, int w, int h)
{
    gui::widget *title = new gui::string("Panoptic", gui::string::sans, 0, 0xFF, 0xC0, 0x40);

    state = new gui::option;

    root = ((new gui::vgroup)->
            add((new gui::harray)->
                add(title)->
                add(new panel_button("About",  state, 0))->
                add(new panel_button("Data",   state, 1))->
                add(new panel_button("Config", state, 2))->
                add(new gui::spacer))->
            add(new gui::spacer)->
            add(state->
                add(new  about_panel(V, state))->
                add(new   data_panel(V, state))->
                add(new config_panel(V, state))));

    root->layup();

    int ww = std::max(root->get_w(), title->get_w() *  6);
    int hh = std::max(root->get_h(), title->get_h() * 12);

    root->laydn((w - ww) / 2,
                (h - hh) / 2, ww, hh);
}

//------------------------------------------------------------------------------
