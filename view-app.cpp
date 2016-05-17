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
#include <cassert>
#include <iomanip>
#include <sstream>

#include <ogl-opengl.hpp>

#include <etc-log.hpp>
#include <etc-dir.hpp>
#include <etc-vector.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include <util3d/math3d.h>
#include <scm-cache.hpp>

#include "view-app.hpp"

//------------------------------------------------------------------------------

view_app::view_app(const std::string& exe,
                   const std::string& tag) : app::prog(exe, tag),
    record(false),
    play  (false),

    zoom     ( 0.0),
    zoom_min (-3.0),                    // How far can we zoom in
    zoom_max ( 2.0),                    // How far can we zoom out
    zoom_rate( 0.0),

    draw_cache(false),

    gui_index(0),
    gui_w(0),
    gui_h(0),
    gui_dx(0),
    gui_dy(0),
    gui(0)
{
    // Add the static data archive.

    extern unsigned char panoptic_data[];
    extern unsigned int  panoptic_data_len;

    ::data->add_pack_archive(panoptic_data,
                             panoptic_data_len);

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

    // Configure the keyboard interface.

    key_location_0 = ::conf->get_i("view_key_location_0", 39);
    key_location_1 = ::conf->get_i("view_key_location_1", 30);
    key_location_2 = ::conf->get_i("view_key_location_2", 31);
    key_location_3 = ::conf->get_i("view_key_location_3", 32);
    key_location_4 = ::conf->get_i("view_key_location_4", 33);
    key_location_5 = ::conf->get_i("view_key_location_5", 34);
    key_location_6 = ::conf->get_i("view_key_location_6", 35);
    key_location_7 = ::conf->get_i("view_key_location_7", 36);

    // Configure the joystick interface. (XBox 360 defaults)

    button_zoom_in   = ::conf->get_i("view_button_zoom_in",   0);
    button_zoom_out  = ::conf->get_i("view_button_zoom_out",  1);
    button_zoom_home = ::conf->get_i("view_button_zoom_home", 6);
    button_gui       = ::conf->get_i("view_button_gui",       7);
    button_select    = ::conf->get_i("view_button_select",    0);
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

// The view handler API is overloaded to manipulate scm_state variables instead
// of the global app::view.

void view_app::set_orientation(const quat &q)
{
    here.set_orientation(q);
}

quat view_app::get_orientation() const
{
    double q[4];
    here.get_orientation(q);
    return quat(q[0], q[1], q[2], q[3]);
}

vec3 view_app::get_position() const
{
    double p[3], r = here.get_distance();
    here.get_position(p);
    return vec3(p[0], p[1], p[2]) * r;
}

//------------------------------------------------------------------------------

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

            GLubyte labelr = n.get_i("labelr", 0x00);
            GLubyte labelg = n.get_i("labelg", 0x00);
            GLubyte labelb = n.get_i("labelb", 0x00);
            GLubyte labela = n.get_i("labela", 0xFF);

            GLubyte clearr = n.get_i("clearr", 0x00);
            GLubyte clearg = n.get_i("clearg", 0x00);
            GLubyte clearb = n.get_i("clearb", 0x00);
            GLubyte cleara = n.get_i("cleara", 0x00);

            f->set_color(labelr << 24 | labelg << 16 | labelb << 8 | labela);
            f->set_clear(clearr << 24 | clearg << 16 | clearb << 8 | cleara);
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

            if (app::node a = n.find("atmosphere"))
            {
                scm_atmo atmo;

                atmo.c[0] = GLfloat(a.get_f("r", 1.0));
                atmo.c[1] = GLfloat(a.get_f("g", 1.0));
                atmo.c[2] = GLfloat(a.get_f("b", 1.0));
                atmo.H    = GLfloat(a.get_f("H", 0.0));
                atmo.P    = GLfloat(a.get_f("P", 1.0));

                f->set_atmo(atmo);
            }
        }
    }
}

//------------------------------------------------------------------------------

// Create a new state object for each state node.

void view_app::load_states(app::node p)
{
    for (app::node n = p.find("state"); n; n = p.next(n, "state"))
    {
        // Initialize a new object.

        scm_state s;

        double q[4];
        double p[3];
        double l[3];

        int i = n.get_i("i", 0);

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

        s.set_name       (n.get_s("name"));
        s.set_foreground0(sys->find_scene(n.get_s("f0")));
        s.set_foreground1(sys->find_scene(n.get_s("f1")));
        s.set_background0(sys->find_scene(n.get_s("b0")));
        s.set_background1(sys->find_scene(n.get_s("b1")));
        s.set_orientation(q);
        s.set_position   (p);
        s.set_light      (l);
        s.set_distance   (n.get_f("r", 0.0));
        s.set_zoom       (n.get_f("z", 1.0));
        s.set_fade       (n.get_f("k", 0.0));

        // Add it to one of the location queues as specified.

        if (0 <= i && i < max_location)
            location[i].push_back(s);

        // If we don't have an initial renderable, consider this one.

        if (!here.renderable())
            here = s;
    }
}


// Delete all states.

void view_app::free_states()
{
    for (int i = 0; i < max_location; i++)
        location[i].clear();

    sequence.clear();
}

//------------------------------------------------------------------------------

// Delete all states and scenes (and by extension ALL data) in the system.

void view_app::unload()
{
    free_states();

    for (int i = 0; i < sys->get_scene_count(); ++i)
        sys->del_scene(0);
}

// Initialize the SCM system using the named XML file.

void view_app::load_file(const std::string& name)
{
    // If the given scene file name includes a directory, that scene's images
    // are likely in the same directory. Temporarily push it onto the path.

    std::string path = name;
    bool pushed      = false;

    if (std::string::size_type s = path.rfind(PATH_SEPARATOR))
    {
        if (s != std::string::npos)
        {
            pushed = true;
            path.erase(s);
            sys->push_path(path);
        }
    }

    // If the named file exists and contains an XML sphere definition...

    app::file file(name);

    if (app::node root = file.get_root().find("sphere"))
    {
        sys->get_sphere()->set_detail(root.get_i("detail", 32));
        sys->get_sphere()->set_limit (root.get_i("limit", 256));

        // Free the states to ensure that their scene references don't dangle.

        free_states();

        here = scm_state();

        // Load the new scenes before deleting the old scenes to ensure that
        // relevant data isn't flushed and reloaded unnecessarily.

        int scenes = sys->get_scene_count();

        load_scenes(root);

        for (int i = 0; i < scenes; ++i)
            sys->del_scene(0);

        // Load the states.

        load_states(root);

        // Bounce the GUI to update it with new data.

        gui_hide();
        gui_show();
    }

    if (pushed) sys->pop_path();
}

//------------------------------------------------------------------------------

/// Parse the given string as a series of camera states. Enqueue each. This
/// function ingests Maya MOV exports.

void view_app::import_mov(const std::string& data)
{
    std::stringstream file(data);
    std::string       line;

    sequence.clear();

    while (std::getline(file, line))
    {
        std::stringstream in(line);

        double t[3] = { 0, 0, 0 };
        double r[3] = { 0, 0, 0 };
        double l[3] = { 0, 0, 0 };

        if (in) in >> t[0] >> t[1] >> t[2];
        if (in) in >> r[0] >> r[1] >> r[2];
        if (in) in >> l[0] >> l[1] >> l[2];

        r[0] = radians(r[0]);
        r[1] = radians(r[1]);
        r[2] = radians(r[2]);

        l[0] = radians(l[0]);
        l[1] = radians(l[1]);
        l[2] = radians(l[2]);

        scm_state s(t, r, l);

        s.set_foreground0(here.get_foreground0());
        s.set_foreground1(here.get_foreground1());
        s.set_background0(here.get_background0());
        s.set_background1(here.get_background1());

        sequence.push_back(s);
    }
}

/// Print all steps on the current queue to the given string using the same
/// format expected by import.

void view_app::export_mov(std::string& data)
{
    std::stringstream file;

    file << std::setprecision(std::numeric_limits<long double>::digits10);

    for (scm_state_c i = sequence.begin(); i != sequence.end(); ++i)
    {
        double d = i->get_distance();
        double p[3];
        double q[4];
        double r[3];

        i->get_position(p);
        i->get_orientation(q);

        p[0] *= d;
        p[1] *= d;
        p[2] *= d;

        equaternion(r, q);

        file << p[0] << " "
             << p[1] << " "
             << p[2] << " "
             << degrees(r[0]) << " "
             << degrees(r[1]) << " "
             << degrees(r[2]) << " "
             << "0.0 0.0 0.0" << std::endl;
    }
    data = file.str();
}

//------------------------------------------------------------------------------

// Create a path from a series of camera configurations in the named file.

void view_app::load_path(const std::string& name)
{
    // Load the contents of the named file to a string.

    const char *path;

    if ((path = (const char *) ::data->load(name)))
    {
        import_mov(path);
        ::data->free(name);
    }
}

// Store a path as a series of camera configurations in the named file.

void view_app::save_path(const std::string& name)
{
    // Export the path to a string.

    std::string path;

    export_mov(path);

    // Write the string to the file.

    ::data->save(name, path.c_str(), 0);
}

// Toggle playback of the current step queue. Movie mode ensures that all frames
// have equal step size, that all data access is performed synchronously, and
// that each frame is written to a sequentially-numbered image file.

void view_app::play_path(bool movie)
{
    if (play)
    {
      ::host->set_movie_mode(false);
        sys->set_synchronous(false);
        play = false;
    }
    else
    {
      ::host->set_movie_mode(movie ? 1 : 0);
        sys->set_synchronous(movie);
        head = sequence.begin();
        play = true;
    }
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

// Report the current view configuration as an XML state element. This is a
// sketchy hack that allows the manual construction of state lists.

void view_app::step()
{
    double q[4];
    double p[3];
    double l[3];

    here.get_orientation(q);
    here.get_position   (p);
    here.get_light      (l);

    printf("<state q0=\"%+.12f\" q1=\"%+.12f\" q2=\"%+.12f\" q3=\"%+.12f\" "
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
    return here.get_current_ground();
}

// Return the global minimum radius of the current SCM system.

double view_app::get_minimum_ground() const
{
    return here.get_minimum_ground();
}

//------------------------------------------------------------------------------

// Prepare for rendering.

ogl::aabb view_app::prep(int frusc, const app::frustum *const *frusv)
{
    // Transfer the current camera state to the view manager.

    ::view->set_orientation(view_app::get_orientation());

    ::view->set_position(get_position());
    ::view->set_scaling (get_scale());

    // Handle the zoom. Not all subclasses will appreciate this.

    const mat4 V =   ::view->get_transform();
    const mat4 P = frusv[0]->get_transform();
    const vec4 v = transpose(P * V) * vec4(0, 0, 1, 0);

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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    sys->render_sphere(&here, transpose(P), transpose(M), chani);
}

// Render the GUI and debugging overlays.

void view_app::over(int frusi, const app::frustum *frusp, int chani)
{
    frusp->load_transform();
   ::view->load_transform();

    if (draw_cache) sys->render_cache();

    if (gui) gui_draw();
}

//------------------------------------------------------------------------------

// Construct a seouence of states from the here to the location with the given
// index. The behavior of this is highly application-specific, so the default
// move is just a jump.

void view_app::move_to(int i)
{
    jump_to(i);
}

// Teleport the view and scene to the location with the given index.

void view_app::jump_to(int i)
{
    if (0 <= i && i < max_location && !location[i].empty())
    {
        here = location[i].front();

        location[i].pop_front();
        location[i].push_back(here);
    }
}

// Construct a fade from here to the location with the given given index. Do so
// without moving the view.

void view_app::fade_to(int i)
{
    // TODO: Plan a sequence to fade over time.

    if (0 <= i && i < max_location && !location[i].empty())
    {
        scm_state there = location[i].front();

        here.set_foreground0(there.get_foreground0());
        here.set_foreground1(there.get_foreground1());
        here.set_background0(there.get_background0());
        here.set_background1(there.get_background1());
    }
}

//------------------------------------------------------------------------------

// These functions allow the viewer GUI to query the scenes of the currently
// loaded SCM system and generate buttons to navigate them.

int view_app::get_location_count() const
{
    return max_location;
}

const std::string view_app::get_location_name(int i) const
{
    if (0 <= i && i < max_location && !location[i].empty())
        return location[i].front().get_name();
    else
        return "";
}

//------------------------------------------------------------------------------

// Handle a press of function key n with control status c and shift status s.

bool view_app::process_function(int k, bool c, bool s)
{
    scm_render *ren = sys->get_render();

    switch (k)
    {
        case SDL_SCANCODE_F2: // Toggle the wire frame

            ren->set_wire(!ren->get_wire());
            return true;

        case SDL_SCANCODE_F3: // Toggle the motion blur

            ren->set_blur(ren->get_blur() ? 0 : 16);
            return true;

        case SDL_SCANCODE_F4: // Toggle the cache view

            draw_cache = !draw_cache;
            return true;

        case SDL_SCANCODE_F5: // Flush the cache

            sys->flush_cache();
            return true;

        case SDL_SCANCODE_F7: // Toggle recording the view motion

            if (record)
            {
                record = false;
            }
            else
            {
                sequence.clear();
                record = true;
            }
            return true;

        case SDL_SCANCODE_F8: // Play the current view motion recording

            play_path(s);
            return true;

//      case SDL_SCANCODE_F11: Default screenshot key defined by app::prog
//      case SDL_SCANCODE_F12: Default GL flush key defined by app:prog
    }
    return false;
}

// Handle a keyboard event.

bool view_app::process_key(app::event *E)
{
    const bool c = (E->data.key.m & KMOD_CTRL)  != 0;
    const bool s = (E->data.key.m & KMOD_SHIFT) != 0;
    const int  k =  E->data.key.k;

    if (E->data.key.d)
    {
        switch (k)
        {
            case SDL_SCANCODE_F1:  case SDL_SCANCODE_F2:
            case SDL_SCANCODE_F3:  case SDL_SCANCODE_F4:
            case SDL_SCANCODE_F5:  case SDL_SCANCODE_F6:
            case SDL_SCANCODE_F7:  case SDL_SCANCODE_F8:
            case SDL_SCANCODE_F9:  case SDL_SCANCODE_F10:
            case SDL_SCANCODE_F11: case SDL_SCANCODE_F12:
            case SDL_SCANCODE_F13: case SDL_SCANCODE_F14:
            case SDL_SCANCODE_F15: return process_function(k, c, s);

            case SDL_SCANCODE_HOME: zoom = 0.0; return true;

            case SDL_SCANCODE_PAGEUP:   step(); return true;
            case SDL_SCANCODE_PAGEDOWN: flag(); return true;
        }

        if (k == key_location_0) { move_to(0); return true; }
        if (k == key_location_1) { move_to(1); return true; }
        if (k == key_location_2) { move_to(2); return true; }
        if (k == key_location_3) { move_to(3); return true; }
        if (k == key_location_4) { move_to(4); return true; }
        if (k == key_location_5) { move_to(5); return true; }
        if (k == key_location_6) { move_to(6); return true; }
        if (k == key_location_7) { move_to(7); return true; }
    }
    return prog::process_event(E);
}

// Handle a user-defined event. SCM viewers treat this as a named move-to.

bool view_app::process_user(app::event *E)
{
    if (sys)
    {
        // Extract the destination name from the user event structure.

        char name[sizeof (long long) + 1];

        memset(name, 0, sizeof (name));
        memcpy(name, &E->data.user.d, sizeof (long long));

        // Scan the locations for a matching name.

        for (int i = 0; i < max_location; i++)
        {
            if (!location[i].empty())
            {
                if (location[i].front().get_name().compare(name) == 0)
                {
                    move_to(i);
                    return true;
                }
            }
        }
    }
    return false;
}

// Handle the passing of time by continuing the recording or playback of the
// step queue.

bool view_app::process_tick(app::event *E)
{
    if (sys)
    {
        if (zoom_rate)
        {
            zoom += zoom_rate * E->data.tick.dt;
            zoom = std::max(zoom, zoom_min);
            zoom = std::min(zoom, zoom_max);
        }

        if (play)
        {
            if (head == sequence.end())
                play_path(false);
            else
            {
                here = *head;
                head++;
            }
        }

        if (record)
            sequence.push_back(here);
    }
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

// Handle a joystick button event.

bool view_app::process_button(app::event *E)
{
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    if (b == button_zoom_in)   { zoom_rate   = d ? -1 : 0;   return true; }
    if (b == button_zoom_out)  { zoom_rate   = d ? +1 : 0;   return true; }
    if (b == button_zoom_home) { zoom        = 0;            return true; }

    return false;
}

// Delegate the handling of an event, or pass it to the superclass.

bool view_app::process_event(app::event *E)
{
    int e = E->get_type();

    if ((e == E_KEY    && E->data.key.d
                       && E->data.key.k == SDL_SCANCODE_F1) ||
        (e == E_BUTTON && E->data.button.d
                       && E->data.button.b == button_gui))
    {
        if (gui)
            gui_hide();
        else
            gui_show();
        return true;
    }

    if (gui &&    gui_event(E)) return true;
    if (prog::process_event(E)) return true;

    switch (e)
    {
        case E_KEY:    if (process_key   (E)) return true; else break;
        case E_USER:   if (process_user  (E)) return true; else break;
        case E_TICK:   if (process_tick  (E)) return true; else break;
        case E_CLICK:  if (process_click (E)) return true; else break;
        case E_BUTTON: if (process_button(E)) return true; else break;
    }

    return false;
}

//------------------------------------------------------------------------------

static double deaden(double k)
{
    if      (k > +0.2) return k + 0.2 * (k - 1);
    else if (k < -0.2) return k + 0.2 * (k + 1);
    else               return 0;
}

// Initialize the file selection GUI.

void view_app::gui_show()
{
    gui_w = ::host->get_window_w();
    gui_h = ::host->get_window_h();

    gui = new view_gui(this, gui_w, gui_h);

    if (!gui_conf.empty()) gui->set_conf(gui_conf);
    if (!gui_data.empty()) gui->set_data(gui_data);

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
        gui_conf  = gui->get_conf();
        gui_data  = gui->get_data();

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

        case E_AXIS:

            if (gui)
            {
                int    a = E->data.axis.a;
                double v = E->data.axis.v;

                if (a == 0) gui_dx = v / 32768.0;
                if (a == 1) gui_dy = v / 32768.0;
            }
            return true;

        case E_TICK:

            if (gui)
            {
                double dx = copysign(pow(deaden(gui_dx), 2.0), gui_dx);
                double dy = copysign(pow(deaden(gui_dy), 2.0), gui_dy);

                if (dx || dy)
                    gui->point(toint(gui->get_last_x() + dx * 16),
                               toint(gui->get_last_y() - dy * 16));
            }
            return false;

        case E_BUTTON:

            if (E->data.button.b == button_select)
            {
                gui->click(0, E->data.button.d != 0);
                return true;
            }

            return false;

        case E_KEY:

            if (E->data.key.d)
                gui->key(E->data.key.k, E->data.key.m);
            return true;

        case E_CLICK:

            gui->click(E->data.click.m, E->data.click.d != 0);
            return true;

        case E_TEXT:

            gui->glyph(E->data.text.c);
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
