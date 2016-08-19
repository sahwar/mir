/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#ifndef MIR_GRAPHICS_FRAME_H_
#define MIR_GRAPHICS_FRAME_H_

#include <cstdint>
#include <ctime>

namespace mir { namespace graphics {

/**
 * Frame is a unique identifier for a frame displayed on an output.
 *
 * This "MSC/UST" terminology is used because that's what the rest of the
 * industry calls it:
 *   GLX: https://www.opengl.org/registry/specs/OML/glx_sync_control.txt
 *   EGL: https://bugs.chromium.org/p/chromium/issues/attachmentText?aid=178027
 *   WGL: https://www.opengl.org/registry/specs/OML/wgl_sync_control.txt
 *
 * The simplistic types are intentional as all these values need to be passed
 * unmodified from the server to clients, and clients of clients (even clients
 * of clients of clients like a GLX app under Xmir under Unity8 under USC)!
 */
struct Frame
{
    uint64_t msc = 0;  /**< Media Stream Counter: Counter of the frame
                            displayed by the output (or as close to it as
                            can be estimated). */
    clockid_t clock_id = CLOCK_MONOTONIC;
                       /**< The system clock identifier that 'ust' is measured
                            by. Usually monotonic, but not always. */
    uint64_t ust = 0;  /**< Unadjusted System Time in microseconds of the frame
                            displayed by the output, relative to clock_id.
                            NOTE that comparing 'ust' to the current system
                            time (at least in the server process) you will
                            often find that 'ust' is in the future by a few
                            microseconds and not yet in the past. This is to be
                            expected and simply reflects the reality that
                            scanning out a new frame can take longer than
                            returning from the function that requested it. */
    uint64_t min_ust_interval = 0;
                       /**< The shortest time possible to the next frame you
                            should expect. This value may change over time and
                            should not be assumed to remain constant,
                            especially as variable framerate displays become
                            more common. */
};

}} // namespace mir::graphics

#endif // MIR_GRAPHICS_FRAME_H_
