/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef MIR_TEST_DOUBLES_STUB_DISPLAY_H_
#define MIR_TEST_DOUBLES_STUB_DISPLAY_H_

#include "mir_test_doubles/null_display.h"
#include "mir_test_doubles/null_display_sync_group.h"
#include "mir_test_doubles/stub_display_buffer.h"
#include "mir_test_doubles/stub_display_configuration.h"

#include "mir/geometry/rectangle.h"

#include <vector>

namespace mir
{
namespace test
{
namespace doubles
{

class StubDisplay : public NullDisplay
{
public:
    StubDisplay(std::vector<geometry::Rectangle> const& output_rects) :
        output_rects(output_rects)
    {
        for (auto const& rect : output_rects)
            groups.emplace_back(new StubDisplaySyncGroup({rect}));
    }

    StubDisplay(unsigned int nbuffers) :
        StubDisplay(generate_stub_rects(nbuffers))
    {
    }

    void for_each_display_sync_group(std::function<void(graphics::DisplaySyncGroup&)> const& f) override
    {
        for (auto& group : groups)
            f(*group);
    }

    std::unique_ptr<graphics::DisplayConfiguration> configuration() const override
    {
        return std::unique_ptr<graphics::DisplayConfiguration>(
            new StubDisplayConfig(output_rects)
        );
    }

    std::vector<geometry::Rectangle> const output_rects;
private:
    std::vector<geometry::Rectangle> generate_stub_rects(unsigned int nbuffers)
    {
        std::vector<geometry::Rectangle> rects;
        for (auto i = 0u; i < nbuffers; i++)
            rects.push_back(geometry::Rectangle{{0,0},{1,1}});
        return rects;
    }

    std::vector<std::unique_ptr<StubDisplaySyncGroup>> groups;
};

}
}
}

#endif