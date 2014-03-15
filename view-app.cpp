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

#include <cmath>
#include <iomanip>
#include <sstream>

#include <ogl-opengl.hpp>

#include <etc-vector.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "scm/util3d/math3d.h"
#include "scm/scm-cache.hpp"

#include "view-app.hpp"

//------------------------------------------------------------------------------

view_app::view_app(const std::string& exe,
                   const std::string& tag) : app::prog(exe, tag),
    now(0),
    delta(0),
    record(false),
    zoom(0.0),
    zoom_min(-2.0),
    zoom_max( 3.0),
    draw_cache(false),
    draw_path (false),
    gui_index(0),
    gui_w(0),
    gui_h(0),
    gui(0)
{
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    // Add the static data archive.

    extern unsigned char data_zip[];
    extern unsigned int  data_zip_len;

    ::data->add_pack_archive(data_zip, data_zip_len);

    // Configure the SCM caches.

    scm_cache::cache_size      = ::conf->get_i("scm_cache_size",
                                         scm_cache::cache_size);
    scm_cache::cache_threads   = ::conf->get_i("scm_cache_threads",
                                         scm_cache::cache_threads);
    scm_cache::need_queue_size = ::conf->get_i("scm_need_queue_size",
                                         scm_cache::need_queue_size);
    scm_cache::load_queue_size = ::conf->get_i("scm_load_queue_size",
                                         scm_cache::load_queue_size);
    scm_cache::loads_per_cycle = ::conf->get_i("scm_loads_per_cycle",
                                         scm_cache::loads_per_cycle);
}

view_app::~view_app()
{
    ::data->free(::conf->get_s("sans_font"));
}

//------------------------------------------------------------------------------

// The host is coming up. The OpenGL context will be available soon.

void view_app::host_up(std::string name)
{
    app::prog::host_up(name);

    // Create the SCM rendering system.

    int w = ::host->get_buffer_w();
    int h = ::host->get_buffer_h();

    sys = new scm_system(w, h, 32, 256);

    // Preload data as requested.

    if (char *name = getenv("SCMINIT"))
        load_file(name);
    else
        gui_show();
}

// The host is going down. Release any OpenGL context state.

void view_app::host_dn()
{
    delete sys;
    sys = 0;
    gui_hide();

    app::prog::host_dn();
}

//------------------------------------------------------------------------------

// Set the Thumb view to match the given step.

void view_from_step(scm_step& step)
{
    double p[3];
    double q[4];
    double r = step.get_distance();

    step.get_orientation(q);
    step.get_position   (p);

    ::view->set_orientation(quat(q[0], q[1], q[2], q[3]));
    ::view->set_position   (vec3(p[0], p[1], p[2]) * r);
}

// Set the given step to match the current Thumb view.

void step_from_view(scm_step& step)
{
    quat q = ::view->get_orientation();
    vec3 p = ::view->get_position();

    step.set_orientation(q);
    step.set_position   (p);
    step.set_distance   (length(p));
}

//------------------------------------------------------------------------------

// Initialize a step object using the given node.

static void step_from_xml(scm_step *s, app::node n)
{
    double q[4];
    double p[3];
    double l[3];

    q[0] = n.get_f("q0", 0.0);
    q[1] = n.get_f("q1", 0.0);
    q[2] = n.get_f("q2", 0.0);
    q[3] = n.get_f("q3", 1.0);

    p[0] = n.get_f("p0", 0.0);
    p[1] = n.get_f("p1", 0.0);
    p[2] = n.get_f("p2", 1.0);

    l[0] = n.get_f("l0", 0.0);
    l[1] = n.get_f("l1", 0.0);
    l[2] = n.get_f("l2", 1.0);

    s->set_name       (n.get_s("name"));
    s->set_foreground (n.get_s("foreground"));
    s->set_background (n.get_s("background"));
    s->set_orientation(q);
    s->set_position   (p);
    s->set_light      (l);
    s->set_speed      (n.get_f("s", 1.0));
    s->set_distance   (n.get_f("r", 0.0));
    s->set_tension    (n.get_f("t", 0.0));
    s->set_bias       (n.get_f("b", 0.0));
    s->set_zoom       (n.get_f("z", 1.0));
}

// Create a new step object for each step node.

void view_app::load_steps(app::node p)
{
    for (app::node n = p.find("step"); n; n = p.next(n, "step"))
    {
        if (scm_step *s = sys->get_step(sys->add_step(sys->get_step_count())))
        {
            step_from_xml(s, n);
        }
    }
}

// Create a new image object for each image node.

void view_app::load_images(app::node p, scm_scene *f)
{
    for (app::node n = p.find("image"); n; n = p.next(n, "image"))
    {
        if (scm_image *p = f->get_image(f->add_image(f->get_image_count())))
        {
            p->set_scm             (n.get_s("scm"));
            p->set_name            (n.get_s("name"));
            p->set_channel         (n.get_i("channel", -1));
            p->set_normal_min(float(n.get_f("k0", 0.0)));
            p->set_normal_max(float(n.get_f("k1", 1.0)));
        }
    }
}

// Create a new scene object for each scene node.

void view_app::load_scenes(app::node p)
{
    for (app::node n = p.find("scene"); n; n = p.next(n, "scene"))
    {
        if (scm_scene *f = sys->get_scene(sys->add_scene(sys->get_scene_count())))
        {
            load_images(n, f);

            GLubyte r = n.get_i("r", 0xFF);
            GLubyte g = n.get_i("g", 0xBF);
            GLubyte b = n.get_i("b", 0x00);
            GLubyte a = n.get_i("a", 0xFF);

            f->set_color(r << 24 | g << 16 | b << 8 | a);
            f->set_name (n.get_s("name"));
            f->set_label(n.get_s("label"));

            const std::string& vert_name = n.get_s("vert");
            const std::string& frag_name = n.get_s("frag");

            if (!vert_name.empty())
            {
                f->set_vert((const char *) ::data->load(vert_name));
                ::data->free(vert_name);
            }
            if (!vert_name.empty())
            {
                f->set_frag((const char *) ::data->load(frag_name));
                ::data->free(frag_name);
            }
        }
    }
}

// Initialize the SCM system using the named XML file.

void view_app::load_file(const std::string& name)
{
    // If the named file exists and contains an XML sphere definition...

    app::file file(name);

    if (app::node root = file.get_root().find("sphere"))
    {
        sys->get_sphere()->set_detail(root.get_i("detail", 32));
        sys->get_sphere()->set_limit (root.get_i("limit", 256));

        // Load the new data.

        int scenes = sys->get_scene_count();
        int steps  = sys->get_step_count();

        load_scenes(root);
        load_steps (root);

        // Delete the old data.

        for (int i = 0; i < scenes; ++i) sys->del_scene(0);
        for (int i = 0; i < steps;  ++i) sys->del_step (0);

        // Dismiss the GUI and display the first loaded scene.

        jump_to(0);
    }
}

// Create a path from a series of camera configurations in the named file.

void view_app::load_path(const std::string& name)
{
    // Load the contents of the named file to a string.

    const char *path;

    if ((path = (const char *) ::data->load(name)))
    {
        // Import a camera path from the loaded string.

        sys->import_queue(path);

        ::data->free(name);
    }
}

// Store a path as a series of camera configurations in the named file.

void view_app::save_path(const std::string& stem)
{
    // Export the path to a string.

    std::string path;
    sys->export_queue(path);

    // Find an usused file name and write the string to it.

    for (int i = 1; true; i++)
    {
        std::stringstream name;

        name << stem << std::setw(3) << std::setfill('0') << i << ".mov";

        if (::data->find(name.str()) == false)
        {
            ::data->save(name.str(), path.c_str(), 0);
            break;
        }
    }
}

// Delete all scenes and steps (and by extension ALL data) in the system.

void view_app::unload()
{
    for (int i = 0; i < sys->get_scene_count(); ++i) sys->del_scene(0);
    for (int i = 0; i < sys->get_step_count();  ++i) sys->del_step (0);
}

//------------------------------------------------------------------------------

// Report the globe's radius at the current location. This is a sketchy hack
// that allows the locations of flags to be determined manually.

void view_app::flag()
{
    double pos[3], rad = get_current_ground();

    here.get_position(pos);

    double lon = atan2(pos[0], pos[2]) * 180.0 / M_PI;
    double lat =  asin(pos[1])         * 180.0 / M_PI;

    printf("%.12f\t%.12f\t%.1f\n", lat, lon, rad);
}

// Report the current view configuration as an XML step element. This is a
// sketchy hack that allows the manual construction of step lists.

void view_app::step()
{
    double q[4];
    double p[3];
    double l[3];

    here.get_orientation(q);
    here.get_position   (p);
    here.get_light      (l);

    printf("<step q0=\"%+.12f\" q1=\"%+.12f\" q2=\"%+.12f\" q3=\"%+.12f\" "
                 "p0=\"%+.12f\" p1=\"%+.12f\" p2=\"%+.12f\" "
                 "l0=\"%+.12f\" l1=\"%+.12f\" l2=\"%+.12f\" r=\"%f\"/>\n",
                q[0], q[1], q[2], q[3],
                p[0], p[1], p[2],
                l[0], l[1], l[2], here.get_distance());
}

//------------------------------------------------------------------------------

// Return the radius of the globe at the current position.

double view_app::get_current_ground() const
{
    double v[3];
    here.get_position(v);
    return sys->get_current_ground(v);
}

// Return the global minimum radius of the current SCM system.

double view_app::get_minimum_ground() const
{
    return sys->get_minimum_ground();
}

//------------------------------------------------------------------------------

// Prepare for rendering.

ogl::aabb view_app::prep(int frusc, const app::frustum *const *frusv)
{
    if (gui)
        glClearColor(0.3, 0.3, 0.3, 0.0);
    else
        glClearColor(0.0, 0.0, 0.0, 0.0);

    // Handle the zoom. Not all subclasses will appreciate this.

    const vec3 v = ::view->get_point_vec(quat());
    sys->get_sphere()->set_zoom(v[0], v[1], v[2], pow(2.0, zoom));

    // Cycle the SCM cache. This is super-important.

    sys->update_cache();

    // Return a world-space bounding volume for the sphere. This simple default
    // will be over-ridden by any decent subclass.

    double r = 2.0 * get_minimum_ground();

    return ogl::aabb(vec3(-r, -r, -r),
                     vec3(+r, +r, +r));
}

// Perform any pre-render lighting.

void view_app::lite(int frusc, const app::frustum *const *frusv)
{
}

// Render the scene.

void view_app::draw(int frusi, const app::frustum *frusp, int chani)
{
    mat4 P =  frusp->get_transform();
    mat4 M = ::view->get_transform();
    mat4 S = scale(vec3(get_scale(),
                        get_scale(),
                        get_scale()));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO: Necessary?

    sys->render_sphere(transpose(P), transpose(S * M), chani);
}

// Render the GUI and debugging overlays.

void view_app::over(int frusi, const app::frustum *frusp, int chani)
{
    frusp->load_transform();
   ::view->load_transform();

    glScaled(get_scale(),
             get_scale(),
             get_scale());

    if (draw_path)  sys->render_queue();
    if (draw_cache) sys->render_cache();

    if (gui) gui_draw();
}

//------------------------------------------------------------------------------

// Construct a path from the current location to the step with the given index.
// The behavior of this is highly application-specific, so the default move is
// just a jump.

void view_app::move_to(int n)
{
    jump_to(n);
}

// Teleport the view configuration and scene to the step with the given index.

void view_app::jump_to(int n)
{
    if (n < sys->get_step_count())
    {
        sys->flush_queue();
        sys->append_queue(new scm_step(sys->get_step(n)));

        here = sys->get_step_blend(1.0);
        sys->set_scene_blend(1.0);

        view_from_step(here);
    }
}

// Construct a fade from the current scene to the scene of the step with the
// given index. Do so without moving the view.

void view_app::fade_to(int n)
{
    if (n < sys->get_step_count())
    {
        scm_step *there = new scm_step(&here);

        there->set_foreground(sys->get_step(n)->get_foreground());
        there->set_background(sys->get_step(n)->get_background());

        sys->flush_queue();
        sys->append_queue(there);

        here = sys->get_step_blend(1.0);
        sys->set_scene_blend(1.0);

        view_from_step(here);
    }
}

//------------------------------------------------------------------------------

// Toggle playback of the current step queue. Movie mode ensures that all frames
// have equal step size, that all data access is performed synchronously, and
// that each frame is written to a sequentially-numbered image file.

void view_app::play(bool movie)
{
    if (delta > 0)
    {
      ::host->set_movie_mode(false);
        sys->set_synchronous(false);
        delta = 0;
    }
    else
    {
      ::host->set_movie_mode(movie ? 1 : 0);
        sys->set_synchronous(movie);
        delta = 1;
        now   = 0;
    }
}

//------------------------------------------------------------------------------

// Handle a press of number key n with control status c and shift status s.

bool view_app::numkey(int n, int c, int s)
{
    if (s == 0)
    {
        if (c == 0)
            move_to(n);
        else
            fade_to(n);
    }
    else
    {
        if (c == 0)
            jump_to(n);
        else
        {
            if (n == 1) flag();
            if (n == 2) step();
        }
    }
    return true;
}

// Handle a press of function key n with control status c and shift status s.

bool view_app::funkey(int n, int c, int s)
{
    if (s == 0)
    {
        if (c == 0)
        {
            scm_render *ren = sys->get_render();

            switch (n)
            {
                case 1: // Toggle the GUI

                    if (gui)
                        gui_hide();
                    else
                        gui_show();
                    return true;

                case 2: // Toggle the wire frame

                    ren->set_wire(!ren->get_wire());
                    return true;

                case 3: // Toggle the motion blur

                    ren->set_blur(ren->get_blur() ? 0 : 16);
                    return true;

                case 4: // Toggle the cache view

                    draw_cache = !draw_cache;
                    return true;

                case 5: // Flush the cache

                    sys->flush_cache();
                    return true;

//              case  9: Default GL flush key defined by app:prog
//              case 10: Default screenshot key defined by app::prog

                case 11: // Begin recording the view motion

                    sys->flush_queue();
                    record = true;
                    return true;

                case 12: // Stop recording the view motion

                    record = false;
                    save_path("scm/path");
                    return true;
            }
        }
    }
    return false;
}

// Handle a keyboard event.

bool view_app::process_key(app::event *E)
{
    const int c = E->data.key.m & KMOD_CTRL;
    const int s = E->data.key.m & KMOD_SHIFT;

    if (E->data.key.d)
    {
        switch (E->data.key.k)
        {
            case SDL_SCANCODE_0:   return numkey(0,  c, s);
            case SDL_SCANCODE_1:   return numkey(1,  c, s);
            case SDL_SCANCODE_2:   return numkey(2,  c, s);
            case SDL_SCANCODE_3:   return numkey(3,  c, s);
            case SDL_SCANCODE_4:   return numkey(4,  c, s);
            case SDL_SCANCODE_5:   return numkey(5,  c, s);
            case SDL_SCANCODE_6:   return numkey(6,  c, s);
            case SDL_SCANCODE_7:   return numkey(7,  c, s);
            case SDL_SCANCODE_8:   return numkey(8,  c, s);
            case SDL_SCANCODE_9:   return numkey(9,  c, s);

            case SDL_SCANCODE_F1:  return funkey(1,  c, s);
            case SDL_SCANCODE_F2:  return funkey(2,  c, s);
            case SDL_SCANCODE_F3:  return funkey(3,  c, s);
            case SDL_SCANCODE_F4:  return funkey(4,  c, s);
            case SDL_SCANCODE_F5:  return funkey(5,  c, s);
            case SDL_SCANCODE_F6:  return funkey(6,  c, s);
            case SDL_SCANCODE_F7:  return funkey(7,  c, s);
            case SDL_SCANCODE_F8:  return funkey(8,  c, s);
            case SDL_SCANCODE_F9:  return funkey(9,  c, s);
            case SDL_SCANCODE_F10: return funkey(10, c, s);
            case SDL_SCANCODE_F11: return funkey(11, c, s);
            case SDL_SCANCODE_F12: return funkey(12, c, s);
            case SDL_SCANCODE_F13: return funkey(13, c, s);
            case SDL_SCANCODE_F14: return funkey(14, c, s);
            case SDL_SCANCODE_F15: return funkey(15, c, s);

            case SDL_SCANCODE_SPACE:  play(s);    return true;
            case SDL_SCANCODE_RETURN: zoom = 0.0; return true;
        }
    }
    return prog::process_event(E);
}

// Handle a user-defined event. SCM viewers treat this as a named move-to.

bool view_app::process_user(app::event *E)
{
    // Extract the destination name from the user event structure.

    char name[sizeof (long long) + 1];

    memset(name, 0, sizeof (name));
    memcpy(name, &E->data.user.d, sizeof (long long));

    // Scan the steps for a matching name.

    for (int i = 0; i < sys->get_step_count(); i++)
    {
        if (sys->get_step(i)->get_name().compare(name) == 0)
        {
            move_to(i);
            return true;
        }
    }
    return false;
}

// Handle the passing of time by continuing the recording or playback of the
// step queue.

bool view_app::process_tick(app::event *E)
{
    if (delta)
    {
        double prev = now;
        double next = now + delta;

        here = sys->get_step_blend(next);
        now = sys->set_scene_blend(next);

        view_from_step(here);

        if (now == prev)
        {
          ::host->set_movie_mode(false);
            sys->set_synchronous(false);
            delta = 0;
        }
    }
    if (record)
        sys->append_queue(new scm_step(here));

    return false;
}

// Handle a mouse button event.

bool view_app::process_click(app::event *E)
{
    if (gui == 0)
    {
        // Minor hack: forward left click as a view-motion right click.

        if (E->data.click.b == SDL_BUTTON_LEFT)
        {
            E->data.click.b =  SDL_BUTTON_RIGHT;
            return prog::process_event(E);
        }

        // Zoom with the mouse wheel.

        if (E->data.click.b == -2)
        {
            zoom = zoom + E->data.click.d / 10.0;
            zoom = std::max(zoom, zoom_min);
            zoom = std::min(zoom, zoom_max);
            return true;
        }
    }
    return false;
}

// Delegate the handling of an event, or pass it to the superclass.

bool view_app::process_event(app::event *E)
{
    if (prog::process_event(E)) return true;

    switch (E->get_type())
    {
        case E_KEY:   if (process_key  (E)) return true; else break;
        case E_USER:  if (process_user (E)) return true; else break;
        case E_TICK:  if (process_tick (E)) return true; else break;
        case E_CLICK: if (process_click(E)) return true; else break;
    }
    if (gui && gui_event(E)) return true;

    return false;
}

//------------------------------------------------------------------------------

// Initialize the file selection GUI.

void view_app::gui_show()
{
    gui_w = ::host->get_window_w();
    gui_h = ::host->get_window_h();

    gui = new view_gui(this, gui_w, gui_h);

    gui->set_index(gui_index);
    gui->show();

    SDL_StartTextInput();
}

// Release the file selection GUI.

void view_app::gui_hide()
{
    if (gui)
    {
        SDL_StopTextInput();

        gui->hide();
        gui_index = gui->get_index();

        delete gui;
        gui = 0;
    }
}

// Draw the file selection GUI in the overlay.

void view_app::gui_draw()
{
    if (const app::frustum *overlay = ::host->get_overlay())
    {
        glEnable(GL_DEPTH_CLAMP_NV);
        {
            const vec3 *p = overlay->get_corners();

            const double w = length(p[1] - p[0]) / gui_w;
            const double h = length(p[2] - p[0]) / gui_h;
            const vec3   x = normal(p[1] - p[0]);
            const vec3   y = normal(p[2] - p[0]);
            const vec3   z = normal(cross(x, y));

            mat4 T = inverse(::view->get_tracking())
                   * translation(p[0])
                   *   transpose(mat3(x, y, z))
                   *       scale(vec3(w, h, 1));

            glLoadMatrixd(transpose(T));
            gui->draw();
        }
        glDisable(GL_DEPTH_CLAMP_NV);
    }
}

// Handle an event while the GUI is visible.

bool view_app::gui_event(app::event *E)
{
    double x;
    double y;

    switch (E->get_type())
    {
        case E_POINT:

            if (const app::frustum *overlay = ::host->get_overlay())
            {
                if (E->data.point.i == 0 && overlay->pointer_to_2D(E, x, y))
                {
                    gui->point(toint(x * gui_w),
                               toint(y * gui_h));
                    return true;
                }
            }
            return false;

        case E_KEY:

            if (gui && E->data.key.d)
                gui->key(E->data.key.k, E->data.key.m);
            return true;

        case E_CLICK:

            if (gui)
                gui->click(E->data.click.m, E->data.click.d);
            return true;

        case E_TEXT:

            if (gui)
                gui->glyph(E->data.text.c);
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
