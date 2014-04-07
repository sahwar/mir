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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 *              Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#include "default_display_buffer_compositor.h"

#include "mir/compositor/scene.h"
#include "mir/compositor/renderer.h"
#include "mir/graphics/renderable.h"
#include "mir/graphics/display_buffer.h"
#include "mir/graphics/buffer.h"
#include "mir/compositor/buffer_stream.h"
#include "bypass.h"
#include "occlusion.h"
#include <mutex>
#include <cstdlib>
#include <algorithm>

namespace mc = mir::compositor;
namespace mg = mir::graphics;

mc::DefaultDisplayBufferCompositor::DefaultDisplayBufferCompositor(
    mg::DisplayBuffer& display_buffer,
    std::shared_ptr<mc::Scene> const& scene,
    std::shared_ptr<mc::Renderer> const& renderer,
    std::shared_ptr<mc::CompositorReport> const& report)
    : display_buffer(display_buffer),
      scene{scene},
      renderer{renderer},
      report{report},
      last_pass_rendered_anything{false}
{
}

bool mc::DefaultDisplayBufferCompositor::composite()
{
    report->began_frame(this);

    static bool const bypass_env{[]
    {
        auto const env = getenv("MIR_BYPASS");
        return !env || env[0] != '0';
    }()};
    bool bypassed = false;
    bool uncomposited_buffers{false};

    auto const& view_area = display_buffer.view_area();
    auto renderable_list = scene->generate_renderable_list();

    if (bypass_env && display_buffer.can_bypass())
    {
        // It would be *really* nice not to lock the scene for a composite pass.
        // (C.f. lp:1234018)
        // A compositor shouldn't know anything about navigating the scene,
        // it should be passed a collection of objects to render. (And any
        // locks managed by the scene - which can just lock what is needed.)
        std::unique_lock<Scene> lock(*scene);

        mc::BypassMatch bypass_match(view_area);
        auto bypass_it = std::find_if(renderable_list.rbegin(), renderable_list.rend(), bypass_match);
        if (bypass_it != renderable_list.rend())
        {
            /*
             * Notice the user_id we pass to buffer() here has to be
             * different to the one used in the Renderer. This is in case
             * the below if() fails we want to complete the frame using the
             * same buffer (different user_id required).
             */
            auto bypass_buf = (*bypass_it)->buffer(this);
            if (bypass_buf->can_bypass())
            {
                uncomposited_buffers = (*bypass_it)->buffers_ready_for_compositor() > 1;

                lock.unlock();
                display_buffer.post_update(bypass_buf);
                bypassed = true;
                renderer->suspend();
            }
        }
    }

    if (!bypassed)
    {
        //preserves buffers backing GL textures until after post_update
        std::vector<std::shared_ptr<mg::Buffer>> saved_resources;

        display_buffer.make_current();

        mc::filter_occlusions_from(renderable_list, view_area);

        renderer->set_rotation(display_buffer.orientation());
        renderer->begin();

        for(auto const& renderable : renderable_list)
        {
            uncomposited_buffers |= (renderable->buffers_ready_for_compositor() > 1);

            //'renderer.get()' serves as an ID to distinguish itself from other compositors
            auto buffer = renderable->buffer(renderer.get());
            renderer->render(*renderable, *buffer);
            saved_resources.push_back(buffer);
        }

        renderer->end();

        display_buffer.post_update();

        // This is a frig to avoid lp:1286190
        if (last_pass_rendered_anything && renderable_list.empty())
            uncomposited_buffers = true;

        last_pass_rendered_anything = !renderable_list.empty();
        // End of frig
    }

    report->finished_frame(bypassed, this);
    return uncomposited_buffers;
}
