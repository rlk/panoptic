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

#ifndef VIEW_GUI_HPP
#define VIEW_GUI_HPP

#include <gui-gui.hpp>

class view_app;

//------------------------------------------------------------------------------
// Panel selection button.

class panel_button : public gui::button
{
    gui::option *state;
    int          index;

public:

    panel_button(std::string s, gui::option *w, int i) :
        button(s), state(w), index(i) { }

    void apply() { state->set_index(index); }
};

//------------------------------------------------------------------------------
// Panels.

class about_panel : public gui::vgroup
{
public:
    about_panel(view_app *, gui::widget *);
};

class data_panel : public gui::vgroup
{
public:
    data_panel(view_app *, gui::widget *);

    std::string get_dir()                { return selector->get_dir();    }
    void        set_dir(const std::string &dir) { selector->set_dir(dir); }

private:
    gui::selector *selector;
};

class config_panel : public gui::vgroup
{
public:
    config_panel(view_app *, gui::widget *);

    std::string get_dir()                { return selector->get_dir();    }
    void        set_dir(const std::string &dir) { selector->set_dir(dir); }

private:
    gui::selector *selector;
};

//------------------------------------------------------------------------------
// Top level dialog.

class view_gui : public gui::dialog
{
    gui::option *state;

    data_panel   *datapan;
    config_panel *confpan;

public:
    view_gui(view_app *, int, int);

    int  get_index() { return state->get_index( ); }
    void set_index(int i)   { state->set_index(i); }

    std::string get_conf() { return confpan->get_dir(); }
    std::string get_data() { return datapan->get_dir(); }

    void set_conf(const std::string &dir) { confpan->set_dir(dir); }
    void set_data(const std::string &dir) { datapan->set_dir(dir); }
};

//------------------------------------------------------------------------------

#endif
