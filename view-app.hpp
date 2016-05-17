//  Copyright (C) 2005-2011 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#ifndef VIEW_APP_HPP
#define VIEW_APP_HPP

#include <vector>

#include <app-prog.hpp>
#include <app-file.hpp>

#include <scm-system.hpp>
#include <scm-sphere.hpp>
#include <scm-render.hpp>
#include <scm-image.hpp>
#include <scm-label.hpp>
#include <scm-state.hpp>
#include <scm-deque.hpp>

#include "view-gui.hpp"

//-----------------------------------------------------------------------------

class view_app : public app::prog
{
public:

    view_app(const std::string&, const std::string&);
   ~view_app();

    virtual ogl::aabb prep(int, const app::frustum * const *);
    virtual void      lite(int, const app::frustum * const *);
    virtual void      draw(int, const app::frustum *, int);
    virtual void      over(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual void unload();
    virtual void   load_file(const std::string&);
    virtual void   load_path(const std::string&);
    virtual void   save_path(const std::string&);
    virtual void   play_path(bool);

    virtual void host_up(std::string);
    virtual void host_dn();

    void cancel();
    void flag();
    void step();

    virtual void set_orientation(const quat&);
    virtual quat get_orientation() const;
    virtual vec3 get_position   () const;

    double get_current_ground() const;
    double get_minimum_ground() const;

    virtual void move_to(int);
    virtual void jump_to(int);
    virtual void fade_to(int);

    // SCM content queries

    int               get_location_count()   const;
    const std::string get_location_name(int) const;

protected:

    static const int max_location = 12;

    // The SCM system and view states.

    scm_system *sys;
    scm_state   here;
    scm_state_v sequence;
    scm_deque   location[max_location];

    // Recording and playback

    bool        record;
    bool        play;
    scm_state_i head;

    void import_mov(const std::string&);
    void export_mov(      std::string&);

    // Zooming

    double zoom;
    double zoom_min;
    double zoom_max;
    double zoom_rate;

    // Event handlers

    virtual bool process_key   (app::event *);
    virtual bool process_user  (app::event *);
    virtual bool process_tick  (app::event *);
    virtual bool process_click (app::event *);
    virtual bool process_button(app::event *);

private:

    void load_images(app::node, scm_scene *);
    void load_scenes(app::node);
    void load_states(app::node);
    void free_states();

    bool draw_cache;

    bool process_function(int, bool, bool);

    virtual double get_speed() const { return 1.0; }
    virtual double get_scale() const { return 1.0; }

    // Keyboard configuration

    int key_location_0;
    int key_location_1;
    int key_location_2;
    int key_location_3;
    int key_location_4;
    int key_location_5;
    int key_location_6;
    int key_location_7;

    // Joystick state

    int button_zoom_in;
    int button_zoom_out;
    int button_zoom_home;
    int button_gui;
    int button_select;

    // Sphere GUI State

    std::string gui_conf;
    std::string gui_data;
    int         gui_index;
    int         gui_w;
    int         gui_h;
    double      gui_dx;
    double      gui_dy;
    view_gui   *gui;

    void gui_show();
    void gui_hide();
    void gui_draw();
    bool gui_event(app::event *);
};

//-----------------------------------------------------------------------------

void view_from_step(scm_state&);
void step_from_view(scm_state&);

//-----------------------------------------------------------------------------

#endif
