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
// Step selection button array.

class step_button : public gui::button
{
    view_app *V;
    int       i;

public:
    step_button(view_app *V, int i);

    void apply();
};

class step_array : public gui::harray
{
public:
    step_array(view_app *V);
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
    data_panel(view_app *, gui::widget *, bool);

    std::string get_dir() {
        return selector ? selector->get_dir() : "";
    }

    void set_dir(const std::string &dir) {
        if (selector)
            selector->set_dir(dir);
    }

private:
    gui::selector *selector;
};

class path_panel : public gui::vgroup
{
public:
    path_panel(view_app *, gui::widget *);

    std::string get_dir() {
        return selector ? selector->get_dir() : "";
    }

    void set_dir(const std::string &dir) {
        if (selector)
            selector->set_dir(dir);
    }

private:
    gui::selector *selector;
};

class config_panel : public gui::vgroup
{
public:
    config_panel(view_app *, gui::widget *);

    std::string get_dir() {
        return selector ? selector->get_dir() : "";
    }

    void set_dir(const std::string &dir) {
        if (selector)
            selector->set_dir(dir);
    }

private:
    gui::selector *selector;
};

//------------------------------------------------------------------------------
// Top level dialog.

class view_gui : public gui::dialog
{
    gui::option *state;

    config_panel *confpan;
    data_panel   *datapan;
    path_panel   *pathpan;

public:
    view_gui(view_app *, int, int);

    int  get_index()  { return (state) ? state->get_index( ) : 0; }
    void set_index(int i) { if (state)   state->set_index(i); }

    std::string get_conf() { return confpan ? confpan->get_dir() : ""; }
    std::string get_data() { return datapan ? datapan->get_dir() : ""; }

    void set_conf(const std::string &dir) { if (confpan) confpan->set_dir(dir); }
    void set_data(const std::string &dir) { if (datapan) datapan->set_dir(dir); }
};

//------------------------------------------------------------------------------

#endif
