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
#include <scm-step.hpp>

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

    virtual void   load_file(const std::string&);
    virtual void   load_path(const std::string&);
    virtual void   save_path(const std::string&);
    virtual void unload();

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

protected:

    // The SCM system and current view.

    scm_system *sys;
    scm_step   here;

    // Recording and playback

    double     now;
    double     delta;
    bool       record;

    void play(bool);

    // Zooming

    double zoom;
    double zoom_min;
    double zoom_max;
    double zoom_rate;

    // Modifier state

    bool mod_shift;
    bool mod_control;

private:

    void save_steps (app::node);
    void load_steps (app::node);
    void load_images(app::node, scm_scene *);
    void load_scenes(app::node);

    bool draw_cache;
    bool draw_path;
    int  curr_step;

    bool dostep(int, bool, bool);
    bool numkey(int, bool, bool);
    bool funkey(int, bool, bool);

    bool process_key   (app::event *);
    bool process_user  (app::event *);
    bool process_tick  (app::event *);
    bool process_click (app::event *);
    bool process_button(app::event *);

    virtual double get_speed() const { return 1.0; }
    virtual double get_scale() const { return 1.0; }

    // Joystick state

    int button_next;
    int button_prev;
    int button_zoom_in;
    int button_zoom_out;
    int button_control;
    int button_shift;

    // Sphere GUI State

    int       gui_index;
    int       gui_w;
    int       gui_h;
    view_gui *gui;

    void gui_show();
    void gui_hide();
    void gui_draw();
    bool gui_event(app::event *);
};

//-----------------------------------------------------------------------------

void view_from_step(scm_step&);
void step_from_view(scm_step&);

//-----------------------------------------------------------------------------

#endif
