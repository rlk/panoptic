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
#include <app-conf.hpp>
#include <etc-dir.hpp>

#include "view-gui.hpp"
#include "view-app.hpp"

#ifdef WIN32
#include <direct.h>
#endif

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

class button_save_path : public gui::button
{
    view_app      *V;
    gui::selector *S;

public:
    button_save_path(view_app *V, gui::selector *S)
        : gui::button("Save Path"), V(V), S(S) { }

    void apply()
    {
        if (!S->value().empty())
            V->save_path(S->value());
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

class button_quit : public gui::button
{
public:
    button_quit() :
        gui::button("Quit") { }

    void apply()
    {
        SDL_Event quit = { SDL_QUIT };
        SDL_PushEvent(&quit);
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
    catch (std::runtime_error&)
    {
        text = "## ABOUT.md not found";
    }

    add((new gui::scroll)->
        add(new gui::pager(text)));
}

//-----------------------------------------------------------------------------
// The Data panel

step_button::step_button(view_app *V, int i) :
    gui::button(V->get_location_name(i)), V(V), i(i)
{
}

void step_button::apply()
{
    V->jump_to(i);
}

step_group::step_group(view_app *V)
{
    gui::harray *A = new gui::harray();

    for (int i = 0; i < V->get_location_count(); i++)
        if (!V->get_location_name(i).empty())
            A->add(new step_button(V, i));

    add(A);
    add(new gui::filler(true, false));
}

data_panel::data_panel(view_app *V, gui::widget *w, bool simple)
    : gui::vgroup(), selector(0)
{
    if (simple)
    {
        add((new gui::frame)->add(new step_group(V)));
    }
    else
    {
        gui::vgroup *G = new gui::vgroup();

        selector = new gui::selector(getcwd(0, 0), ".xml");

        G->add(selector)->
           add((new gui::harray())->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new button_load_data(V, selector)));

        if (V->get_location_count())
        {
            G->add(new gui::spacer);
            G->add(new step_group(V));
        }

        add((new gui::frame)->add(G));
    }
}

path_panel::path_panel(view_app *V, gui::widget *w)
    : gui::vgroup(), selector(0)
{
    gui::vgroup *G = new gui::vgroup();

    selector = new gui::selector(getcwd(0, 0), ".mov");

    G->add(selector)->
       add((new gui::harray())->
            add(new gui::filler(true, false))->
            add(new gui::filler(true, false))->
            add(new gui::filler(true, false))->
            add(new button_save_path(V, selector))->
            add(new button_load_path(V, selector)));

    add((new gui::frame)->add(G));
}

//-----------------------------------------------------------------------------
// The Config panel

config_panel::config_panel(view_app *V, gui::widget *w) : gui::vgroup()
{
    const std::string a((const char *) glGetString(GL_VERSION));
    const std::string b((const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));

    const std::string s = "OpenGL " + a + " GLSL " + b;
    const std::string f = "config" + std::string(1, PATH_SEPARATOR) + "common";

    selector = new gui::selector(f, ".xml");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(selector)->
            add((new gui::hgroup)->
                add(new gui::filler(true, false))->
                add(new gui::string(s, gui::string::mono, 0, 0, 0, 0))->
                add(new button_load_config(selector)))));
}

//-----------------------------------------------------------------------------
// The toplevel control panel

view_gui::view_gui(view_app *V, int w, int h) :
    state(0), confpan(0), datapan(0), pathpan(0)
{
    int ss = ::conf->get_i("sans_size", 16);

    if (::conf->get_i("panoptic_simple_gui", 0))
    {
        root = new data_panel(V, state, true);
    }
    else
    {
        state = new gui::option;

        gui::widget *A = new panel_button("About",  state, 0);
        gui::widget *B = new panel_button("Config", state, 1);
        gui::widget *C = new panel_button("Data",   state, 2);
        gui::widget *D = new panel_button("Path",   state, 3);
        gui::widget *Q = new button_quit();

        confpan = new config_panel(V, state);
        datapan = new   data_panel(V, state, false);
        pathpan = new   path_panel(V, state);

        root = ((new gui::vgroup)->
                add((new gui::harray)->
                    add(A)->
                    add(B)->
                    add(C)->
                    add(D)->
                    add(new gui::spacer)->
                    add(Q))->
                add(new gui::spacer)->
                add(state->
                    add(new about_panel(V, state))->
                    add(confpan)->
                    add(datapan)->
                    add(pathpan)));
    }

    root->layup();

    int ww = std::max(root->get_w(), ss * 50);
    int hh = std::max(root->get_h(), ss * 25);

    root->laydn((w - ww) / 2,
                (h - hh) / 2, ww, hh);

    last_x = w / 2;
    last_y = h / 2;
}

//------------------------------------------------------------------------------
