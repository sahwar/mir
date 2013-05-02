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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef MIR_GRAPHICS_ANDROID_HWC_COMMON_DEVICE_H_
#define MIR_GRAPHICS_ANDROID_HWC_COMMON_DEVICE_H_
#include "hwc_device.h"

#include <hardware/hwcomposer.h>
#include <mutex>
#include <condition_variable>

namespace mir
{
namespace graphics
{
namespace android
{

class HWCCommonDevice;
struct HWCCallbacks
{
    hwc_procs_t hooks;
    HWCCommonDevice* self;
};

class HWCCommonDevice : public HWCDevice
{
public:
    virtual ~HWCCommonDevice() noexcept;

    /* from HWCDevice */
    geometry::PixelFormat display_format() const;
    unsigned int number_of_framebuffers_available() const; 
    void wait_for_vsync();

    virtual geometry::Size display_size() const = 0;
    virtual void set_next_frontbuffer(std::shared_ptr<compositor::Buffer> const& buffer) = 0;
    virtual void commit_frame(EGLDisplay dpy, EGLSurface sur) = 0;

    void notify_vsync();

protected:
    HWCCommonDevice(std::shared_ptr<hwc_composer_device_1> const& hwc_device);

    std::shared_ptr<hwc_composer_device_1> const hwc_device;
private:
    HWCCallbacks callbacks;
    std::mutex vsync_wait_mutex;
    std::condition_variable vsync_trigger;
    bool vsync_occurred;
};

}
}
}

#endif /* MIR_GRAPHICS_ANDROID_HWC_COMMON_DEVICE_H_ */
