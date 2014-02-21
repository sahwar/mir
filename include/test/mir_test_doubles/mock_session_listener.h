/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

#ifndef MIR_TEST_DOUBLES_MOCK_SESSION_LISTENER_H_
#define MIR_TEST_DOUBLES_MOCK_SESSION_LISTENER_H_

#include "mir/shell/session_listener.h"
#include "mir/shell/trusted_session.h"

#include <gmock/gmock.h>

namespace mir
{
namespace test
{
namespace doubles
{

struct MockSessionListener : public shell::SessionListener
{
    virtual ~MockSessionListener() noexcept(true) {}

    MOCK_METHOD1(starting, void(std::shared_ptr<shell::Session> const&));
    MOCK_METHOD1(stopping, void(std::shared_ptr<shell::Session> const&));
    MOCK_METHOD1(focused, void(std::shared_ptr<shell::Session> const&));
    MOCK_METHOD0(unfocused, void());

    MOCK_METHOD2(surface_created, void(shell::Session&, std::shared_ptr<shell::Surface> const&));
    MOCK_METHOD2(destroying_surface, void(shell::Session&, std::shared_ptr<shell::Surface> const&));

    MOCK_METHOD2(trusted_session_started, void(std::shared_ptr<shell::Session> const&, std::shared_ptr<shell::TrustedSession> const&));
    MOCK_METHOD1(trusted_session_stopped, void(std::shared_ptr<shell::Session> const&));
};

}
}
} // namespace mir

#endif // MIR_TEST_DOUBLES_MOCK_SESSION_LISTENER_H_
