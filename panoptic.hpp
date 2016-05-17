//  Copyright (C) 2005-2014 Robert Kooima
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

#ifndef PANOPTIC_HPP
#define PANOPTIC_HPP

#include <vector>

#include <app-prog.hpp>
#include <etc-vector.hpp>
#include <etc-socket.hpp>

#include "view-app.hpp"

//-----------------------------------------------------------------------------

class panoptic : public view_app
{
public:

    panoptic(const std::string&, const std::string&);

    virtual ogl::aabb prep(int, const app::frustum * const *);
    virtual void      draw(int, const app::frustum *, int);

    virtual void move_to(int);
    virtual void jump_to(int);
    virtual void fade_to(int);

    virtual quat get_orientation() const;
    virtual void set_orientation(const quat&);
    virtual void offset_position(const vec3&);

    virtual ~panoptic();

private:

    virtual bool process_tick(app::event *);

    // View motion state

    quat   get_local() const;
    bool   pan_mode() const;

    virtual double get_speed()           const;
    virtual double get_scale()           const;
    virtual void   set_pitch(scm_state&) const;

    double speed_min;
    double speed_max;
    double demo_delay;
    double minimum_agl;
    int    auto_pitch;

    // Demo state

    vec3   demo_move;
    double demo_turn;

    double demo_dist_delay;
    double demo_dist_T;
    double demo_dist_t;
    double demo_dist_0;
    double demo_dist_1;

    double demo_turn_delay;
    double demo_turn_value;

    // Report stream configuration

    sockaddr_in report_addr;
    SOCKET      report_sock;
    void        report();
};

//-----------------------------------------------------------------------------

#endif
