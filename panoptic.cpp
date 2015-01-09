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

#include <SDL_mouse.h>

#include <cmath>

#include <ogl-opengl.hpp>

#include <etc-log.hpp>
#include <etc-socket.hpp>
#include <etc-vector.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-view.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include <util3d/math3d.h>
#include <scm-log.hpp>

#include "panoptic.hpp"

//------------------------------------------------------------------------------

panoptic::panoptic(const std::string& exe,
                   const std::string& tag)
    : view_app(exe, tag), report_sock(INVALID_SOCKET)
{
    // Initialize all interaction state.

    speed_min   = ::conf->get_f("panoptic_speed_min",    0.0);
    speed_max   = ::conf->get_f("panoptic_speed_max",    0.2);
    minimum_agl = ::conf->get_f("panoptic_minimum_agl", 50.0);
    stick_timer = 0.0;

    // Initialize the reportage socket.

    int         port = ::conf->get_i("panoptic_report_port", 8111);
    std::string host = ::conf->get_s("panoptic_report_host");

    if (port && !host.empty())
    {
        if (init_sockaddr(report_addr, host.c_str(), port))
            report_sock = socket(AF_INET, SOCK_DGRAM, 0);
    }
}

panoptic::~panoptic()
{
    if (report_sock != INVALID_SOCKET)
        close(report_sock);
}

// This is a potentially troublesome function that solves a tough problem in
// an unsatisfactory fashion. It determines whether the current scene is or is
// not a panorama.
//
// Currently, a large sphere implies a planet and a small sphere implies a
// panorama. The dividing line is 100 meters.
//
// The mapping of user input onto view state changes depending on the outcome.

bool panoptic::pan_mode() const
{
    return (get_minimum_ground() < 100.0);
}

//------------------------------------------------------------------------------

// The report mechanism transmits the current view location to a remote host,
// as configured in options.xml. This allows the creation of an external map
// display showing the user in the context of the globe.

void panoptic::report()
{
    // If a report destination has been configured...

    if (report_addr.sin_addr.s_addr != INADDR_NONE &&
        report_sock                 != INVALID_SOCKET)
    {
        // Compute the current longitude, latitude, and altitude.

        double p[3], alt = here.get_distance();

        here.get_position(p);

        double lon = atan2(p[0], p[2]) * 180.0 / M_PI;
        double lat =  asin(p[1])       * 180.0 / M_PI;

        // Encode these to an ASCII string.

        char buf[128];
        sprintf(buf, "%+12.8f %+13.8f %17.8f\n", lat, lon, alt);

        // And send the string to the configured host.

        sendto(report_sock, buf, strlen(buf) + 1, 0,
               (const sockaddr *) &report_addr, sizeof (sockaddr_in));
    }
}

//------------------------------------------------------------------------------

ogl::aabb panoptic::prep(int frusc, const app::frustum *const *frusv)
{
    ogl::aabb aabb = view_app::prep(frusc, frusv);
    report();

    if (pan_mode())
        return aabb;
    else
    {
        // Compute a horizon line based on altitude and minimum terrain height.

        const double k =          get_scale();
        const double r = k *      get_current_ground();
        const double m = k *      get_minimum_ground();
        const double d = k * here.get_distance();

        double n = 0.5 *     (d     - r    );
        double f = 1.5 * sqrt(d * d - m * m);

        // Exploit an AABB special case to transmit near and far directly.

        return ogl::aabb(vec3(0, 0, n), vec3(0, 0, f));
    }
}

void panoptic::draw(int frusi, const app::frustum *frusp, int chani)
{
    mat4 M = ::view->get_transform();

    // Set the label clipping plane.

    const double m =      get_minimum_ground();
    const double d = here.get_distance();

    double C[4] = { 0.0, 0.0, 1.0, 0.0 };

    here.get_position(C);

    C[3] = -m * m / d;

    glLoadMatrixd(transpose(M));
    glClipPlane(GL_CLIP_PLANE0, C);

    // Set the light position.

    double  l[3];
    GLfloat L[4];

    here.get_light(l);

    L[0] = GLfloat(l[0]);
    L[1] = GLfloat(l[1]);
    L[2] = GLfloat(l[2]);
    L[3] = 0.0f;

    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, L);

    view_app::draw(frusi, frusp, chani);
    view_app::over(frusi, frusp, chani);
}

//------------------------------------------------------------------------------

// Return an altitude scalar.

double panoptic::get_speed() const
{
    const double d = here.get_distance();
    const double h =      get_current_ground();
    const double k = (d - h) / h;

    if (k > speed_max) return speed_max;
    if (k < speed_min) return speed_min;

    return k;
}

double panoptic::get_scale() const
{
    if (pan_mode())
        return 1.0;
    else
    {
        const double s = ::host->get_distance();
        const double d =    here.get_distance();
        const double h =         get_current_ground();
        const double k = std::min(1.0, s / (d - h));

        return k;
    }
}

//------------------------------------------------------------------------------

quat panoptic::get_local() const
{
    const vec3 p(view_app::get_position());
    const mat3 R(view_app::get_orientation());

    vec3 y = normal(p);
    vec3 x;
    vec3 z;

    if (fabs(xvector(R) * y) < fabs(zvector(R) * y))
    {
        x = normal(xvector(R));
        z = normal(cross(x, y));
        x = normal(cross(y, z));
    }
    else
    {
        z = normal(zvector(R));
        x = normal(cross(y, z));
        z = normal(cross(x, y));
    }

    return quat(mat3(x, y, z));
}

quat panoptic::get_orientation() const
{
    if (pan_mode())
        return                        view_app::get_orientation();
    else
        return inverse(get_local()) * view_app::get_orientation();
}

void panoptic::set_orientation(const quat &q)
{
    if (pan_mode())
        here.set_orientation(q);
    else
        here.set_orientation(get_local() * q);
}

void panoptic::offset_position(const vec3 &d)
{
    if (pan_mode())
        view_app::offset_position(d);
    else
    {
        // Apply the motion vector as rotation of the step.

        const double k = 500000.0 * get_speed();
        const double r = here.get_distance();
        const mat3   B(get_local());

        mat4 zM = mat4(mat3(quat(zvector(B), atan2(-d[0] * k, r))));
        mat4 xM = mat4(mat3(quat(xvector(B), atan2( d[2] * k, r))));

        if (mod_control)
        {
            here.transform_light      (transpose(xM));
            here.transform_light      (transpose(zM));
        }
        else
        {
            here.transform_orientation(transpose(xM));
            here.transform_position   (transpose(xM));
            here.transform_light      (transpose(xM));
            here.transform_orientation(transpose(zM));
            here.transform_position   (transpose(zM));
            here.transform_light      (transpose(zM));

            // Clamp the altitude.

            double v[3];

            here.get_position(v);
            here.set_distance(std::max(d[1] * k + r,
                                    minimum_agl + sys->get_current_ground(v)));
        }
    }
}

//------------------------------------------------------------------------------

// Compute the length of the Archimedean spiral with polar equation r = a theta.

double arclen(double a, double theta)
{
    double d = sqrt(1 + theta * theta);
    return a * (theta * d + log(theta + d)) / 2.0;
}

// Calculate the length of the arc of length theta along an Archimedean spiral
// that begins at radius r0 and ends at radius r1.

double spiral(double r0, double r1, double theta)
{
    double dr = fabs(r1 - r0);

    if (theta > 0.0)
    {
        if (dr > 0.0)
        {
            double a = dr / theta;
            return fabs(arclen(a, r1 / a) - arclen(a, r0 / a));
        }
        return theta * r0;
    }
    return dr;
}

void panoptic::move_to(int i)
{
    view_app::move_to(i);
}

void panoptic::jump_to(int i)
{
    view_app::jump_to(i);
}

void panoptic::fade_to(int i)
{
    view_app::fade_to(i);
#if 0
    bool before = pan_mode();
    view_app::fade_to(i);
    bool after  = pan_mode();

    if (before != after)
        view_app::jump_to(i);
#endif
}

#if 0
void panoptic::move_to(int i)
{
    // Construct a path from here to there.

    if (delta == 0)
    {
        if (0 <= i && i < sys->get_step_count())
        {
            // Set the location and destination.

            scm_step *src = &here;
            scm_step *dst = sys->get_step(i);

            // Determine the beginning and ending positions and altitudes.

            double p0[3];
            double p1[3];

            src->get_position(p0);
            dst->get_position(p1);

            double g0 = sys->get_current_ground(p0);
            double g1 = sys->get_current_ground(p1);

            double d0 = src->get_distance();
            double d1 = dst->get_distance();

            // Compute the ground trace length and orbit length.

            double a = acos(vdot(p0, p1));
            double lg = spiral(g0, g1, a);
            double lo = spiral(d0, d1, a);

            // Calculate a "hump" for a low orbit path.

            double aa = std::min(d0 - g0, d1 - g1);
            double dd = lg ? log10(lg / aa) * lg / 10 : 0;

            // Enqueue the path.

            sys->flush_queue();

            if (lo > 0)
            {
                for (double t = 0.0; t < 1.0; )
                {
                    double p[3];
                    double dt = 0.01;
                    double q = 4 * t - 4 * t * t;

                    // Estimate the current velocity.

                    scm_step t0(src, dst, t);
                    scm_step t1(src, dst, t + dt);

                    t0.set_distance(t0.get_distance() + dd * q);
                    t1.set_distance(t1.get_distance() + dd * q);

                    // Queue this step.

                    if (t < 0.5)
                    {
                        t0.set_foreground(src->get_foreground());
                        t0.set_background(src->get_background());
                    }
                    else
                    {
                        t0.set_foreground(dst->get_foreground());
                        t0.set_background(dst->get_background());
                    }
                    sys->append_queue(new scm_step(t0));

                    // Move forward at a velocity appropriate for the altitude.

                    t0.get_position(p);

                    double g = sys->get_current_ground(p);

                    t += 2 * (t0.get_distance() - g) * dt * dt / (t1 - t0);
                }
            }
            sys->append_queue(new scm_step(dst));

            // Trigger playback.

            play(false);
        }
    }
}
#endif

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        std::string t(DEFAULT_TAG);
        std::string d;

        app::prog *P;

        for (int i = 1; i < argc; i++)
        {
            if (std::string(argv[i]) == "-t" && i < argc - 1)
            {
                t = std::string(argv[i + 1]);
                i++;
            }
            if (std::string(argv[i]) == "-d" && i < argc - 1)
            {
                d = std::string(argv[i + 1]);
                i++;
            }
        }

        P = new panoptic(argv[0], t);
        if (d.size()) P->dump(d);
        P->run();

        delete P;
    }
    catch (std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Uncaught exception", e.what(), 0);
    }
    return 0;
}
