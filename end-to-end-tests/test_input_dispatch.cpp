/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include "mock_input_device.h"

#include "mir/time_source.h"
#include "mir/input/device.h"
#include "mir/input/dispatcher.h"
#include "mir/input/event.h"
#include "mir/input/filter.h"
#include "mir/input/logical_device.h"
#include "mir/input/position_info.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace mi = mir::input;

namespace
{

template<typename T>
class MockFilter : public T
{
 public:
    MockFilter()
    {
        using namespace testing;
        using ::testing::_;
        using ::testing::Return;
        
        ON_CALL(*this, accept(_)).WillByDefault(Return(mi::Filter::Result::continue_processing));
    }
    
    MOCK_METHOD1(accept, mi::Filter::Result(mi::Event*));
};

class MockTimeSource : public mir::TimeSource
{
 public:
    MOCK_CONST_METHOD0(sample, mir::Timestamp());
};

class InputDispatchFixture : public ::testing::Test
{
 public:
    InputDispatchFixture()
            : mock_shell_filter(new MockFilter<mi::ShellFilter>()),
              mock_grab_filter(new MockFilter<mi::GrabFilter>()),
              mock_app_filter(new MockFilter<mi::ApplicationFilter>()),
              dispatcher(&time_source,
                         std::move(std::unique_ptr<mi::ShellFilter>(mock_shell_filter)),
                         std::move(std::unique_ptr<mi::GrabFilter>(mock_grab_filter)),
                         std::move(std::unique_ptr<mi::ApplicationFilter>(mock_app_filter)))
    {
        mir::Timestamp ts;
        ::testing::DefaultValue<mir::Timestamp>::Set(ts);
    }
 protected:
    MockTimeSource time_source;
    MockFilter<mi::ShellFilter>* mock_shell_filter;
    MockFilter<mi::GrabFilter>* mock_grab_filter;
    MockFilter<mi::ApplicationFilter>* mock_app_filter;
    mi::Dispatcher dispatcher;
};

}

TEST_F(InputDispatchFixture, incoming_input_triggers_filter)
{
    using namespace testing;
    using ::testing::_;
    using ::testing::Return;
    
    mi::MockInputDevice* mock_device = new mi::MockInputDevice(&dispatcher);
    std::unique_ptr<mi::LogicalDevice> device(mock_device);

    EXPECT_CALL(*mock_device, start()).Times(AtLeast(1));
    mi::Dispatcher::DeviceToken token = dispatcher.register_device(std::move(device));
    
    EXPECT_CALL(*mock_shell_filter, accept(_)).Times(AtLeast(1));
    EXPECT_CALL(*mock_grab_filter, accept(_)).Times(AtLeast(1));
    EXPECT_CALL(*mock_app_filter, accept(_)).Times(AtLeast(1));

    mock_device->trigger_event();

    EXPECT_CALL(*mock_device, stop()).Times(AtLeast(1));
    dispatcher.unregister_device(token);
}

TEST_F(InputDispatchFixture, incoming_input_is_timestamped)
{
    using namespace testing;

    mir::Timestamp ts;
    DefaultValue<mir::Timestamp>::Set(ts);
    
    mi::MockInputDevice* mock_device = new mi::MockInputDevice(&dispatcher);
    std::unique_ptr<mi::LogicalDevice> device(mock_device);

    EXPECT_CALL(*mock_device, start()).Times(AtLeast(1));
    mi::Dispatcher::DeviceToken token = dispatcher.register_device(std::move(device));
    
    EXPECT_CALL(time_source, sample()).Times(AtLeast(1));
    mock_device->trigger_event();

    EXPECT_EQ(mock_device->event.get_system_timestamp(), ts);

    EXPECT_CALL(*mock_device, stop()).Times(AtLeast(1));
    dispatcher.unregister_device(token);
}

TEST_F(InputDispatchFixture, device_registration_starts_and_stops_event_producer)
{
    mi::MockInputDevice* mock_device = new mi::MockInputDevice(&dispatcher);
    std::unique_ptr<mi::LogicalDevice> device(mock_device);

    EXPECT_CALL(*mock_device, start()).Times(AtLeast(1));
    mi::Dispatcher::DeviceToken token = dispatcher.register_device(std::move(device));

    EXPECT_EQ(mi::EventProducer::State::running, mock_device->current_state());

    EXPECT_CALL(*mock_device, stop()).Times(AtLeast(1));
    device = dispatcher.unregister_device(token);
    EXPECT_EQ(mock_device, device.get());
}
