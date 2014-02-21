/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored By: Robert Carr <robert.carr@canonical.com>
 */

#include "default_session_container.h"

#include <boost/throw_exception.hpp>
#include <mir/shell/session.h>

#include <memory>
#include <cassert>
#include <algorithm>
#include <stdexcept>

namespace ms = mir::scene;
namespace msh = mir::shell;

void ms::DefaultSessionContainer::insert_session(std::shared_ptr<msh::Session> const& session)
{
    std::unique_lock<std::mutex> lk(guard);

    apps.push_back(session);
    printf("DefaultSessionContainer::insert_session( %s ) -> count = %d\n", session->name().c_str(), (int)apps.size());
}

void ms::DefaultSessionContainer::remove_session(std::shared_ptr<msh::Session> const& session)
{
    std::unique_lock<std::mutex> lk(guard);

    auto it = std::find(apps.begin(), apps.end(), session);
    if (it != apps.end())
    {
        apps.erase(it);
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::logic_error("Invalid Session"));
    }
    printf("DefaultSessionContainer::remove_session( %s ) -> count = %d\n", session->name().c_str(), (int)apps.size());
}

void ms::DefaultSessionContainer::clear()
{
    std::unique_lock<std::mutex> lk(guard);

    apps.clear();
}

void ms::DefaultSessionContainer::for_each(std::function<void(std::shared_ptr<msh::Session> const&)> f) const
{
    std::unique_lock<std::mutex> lk(guard);

    for (auto const ptr : apps)
    {
        f(ptr);
    }
}

void ms::DefaultSessionContainer::for_each(std::function<bool(std::shared_ptr<msh::Session> const&)> f, bool reverse) const
{
    std::unique_lock<std::mutex> lk(guard);

    if (reverse)
    {
        for (auto rit = apps.rbegin(); rit != apps.rend(); ++rit)
        {
            if (!f(*rit))
                break;
        }
    }
    else
    {
        for (auto it = apps.begin(); it != apps.end(); ++it)
        {
            if (!f(*it))
                break;
        }
    }
}

std::shared_ptr<msh::Session> ms::DefaultSessionContainer::successor_of(std::shared_ptr<msh::Session> const& session) const
{
    std::shared_ptr<msh::Session> result, first;

    if (!session && apps.size())
        return apps.back();
    else if(!session)
        return std::shared_ptr<msh::Session>();

    for (auto it = apps.begin(); it != apps.end(); it++)
    {
        if (*it == session)
        {
            auto successor = ++it;
            if (successor == apps.end())
                return *apps.begin();
            else return *successor;
        }
    }

    BOOST_THROW_EXCEPTION(std::logic_error("Invalid session"));
}
