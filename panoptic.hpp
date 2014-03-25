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

//  virtual void move_to(int);

    virtual quat get_orientation() const;
    virtual void set_orientation(const quat&);
    virtual void offset_position(const vec3&);

    virtual ~panoptic();

private:

    // View motion state

    quat   get_local() const;
    bool   pan_mode() const;

    virtual double get_speed() const;
    virtual double get_scale() const;

    double speed_min;
    double speed_max;
    double stick_timer;
    double minimum_agl;

    // Report stream configuration

    sockaddr_in report_addr;
    SOCKET      report_sock;
    void        report();
};

//-----------------------------------------------------------------------------

#endif
