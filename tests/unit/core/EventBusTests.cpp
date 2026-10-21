#include "sprout/core/Event.hpp"

#include <cstddef>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace {

struct Damage {
    int amount{0};
};

struct Healed {
    int amount{0};
};

using sprout::Event;
using sprout::EventBus;
using sprout::kInvalidSubscription;
using sprout::SubscriptionId;

TEST(EventBusTests, Publish_TypedEvent_InvokesOnlyMatchingSubscribers)
{
    EventBus bus;
    int damageAmount = 0;
    int healedAmount = 0;

    bus.subscribe<Damage>(1, [&damageAmount](const Damage& event) { damageAmount += event.amount; });
    bus.subscribe<Healed>(1, [&healedAmount](const Healed& event) { healedAmount += event.amount; });

    const std::size_t invoked = bus.publish(Damage{5});

    EXPECT_EQ(invoked, std::size_t{1});
    EXPECT_EQ(damageAmount, 5);
    EXPECT_EQ(healedAmount, 0);
}

TEST(EventBusTests, Publish_MultipleSubscribers_InvokesInRegistrationOrder)
{
    EventBus bus;
    std::vector<int> order;

    bus.subscribe<Damage>(1, [&order](const Damage&) { order.push_back(1); });
    bus.subscribe<Damage>(2, [&order](const Damage&) { order.push_back(2); });
    bus.subscribe<Damage>(3, [&order](const Damage&) { order.push_back(3); });

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{3});

    ASSERT_EQ(order.size(), std::size_t{3});
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST(EventBusTests, Publish_NoSubscribers_ReturnsZero)
{
    EventBus bus;

    EXPECT_EQ(bus.publish(Damage{1}), std::size_t{0});
}

TEST(EventBusTests, Subscribe_ReturnsDistinctNonZeroIds)
{
    EventBus bus;

    const SubscriptionId first = bus.subscribe<Damage>(1, [](const Damage&) {});
    const SubscriptionId second = bus.subscribe<Damage>(2, [](const Damage&) {});

    EXPECT_NE(first, kInvalidSubscription);
    EXPECT_NE(second, kInvalidSubscription);
    EXPECT_NE(first, second);
}

TEST(EventBusTests, Unsubscribe_RemovesOnlyThatSubscriber)
{
    EventBus bus;
    int firstCalls = 0;
    int secondCalls = 0;

    const SubscriptionId first = bus.subscribe<Damage>(1, [&firstCalls](const Damage&) { ++firstCalls; });
    bus.subscribe<Damage>(2, [&secondCalls](const Damage&) { ++secondCalls; });

    EXPECT_TRUE(bus.unsubscribe(first));
    EXPECT_FALSE(bus.unsubscribe(first));
    EXPECT_FALSE(bus.unsubscribe(kInvalidSubscription));

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{1});
    EXPECT_EQ(firstCalls, 0);
    EXPECT_EQ(secondCalls, 1);
    EXPECT_EQ(bus.subscriberCount<Damage>(), std::size_t{1});
}

TEST(EventBusTests, UnsubscribeAll_RemovesOwnerAcrossEventTypes)
{
    EventBus bus;
    int ownerOneDamage = 0;
    int ownerOneHealed = 0;
    int ownerTwoDamage = 0;

    bus.subscribe<Damage>(1, [&ownerOneDamage](const Damage& event) { ownerOneDamage += event.amount; });
    bus.subscribe<Healed>(1, [&ownerOneHealed](const Healed& event) { ownerOneHealed += event.amount; });
    bus.subscribe<Damage>(2, [&ownerTwoDamage](const Damage& event) { ownerTwoDamage += event.amount; });

    EXPECT_EQ(bus.unsubscribeAll(1), std::size_t{2});
    EXPECT_EQ(bus.unsubscribeAll(1), std::size_t{0});
    EXPECT_EQ(bus.subscriberCount<Damage>(), std::size_t{1});
    EXPECT_EQ(bus.subscriberCount<Healed>(), std::size_t{0});

    bus.publish(Damage{2});
    bus.publish(Healed{3});

    EXPECT_EQ(ownerOneDamage, 0);
    EXPECT_EQ(ownerOneHealed, 0);
    EXPECT_EQ(ownerTwoDamage, 2);
}

TEST(EventBusTests, UnsubscribeAll_DuringPublish_SkipsRemovedOwner)
{
    EventBus bus;
    int firstCalls = 0;
    int secondCalls = 0;

    bus.subscribe<Damage>(1, [&bus, &firstCalls](const Damage&) {
        ++firstCalls;
        bus.unsubscribeAll(2);
    });
    bus.subscribe<Damage>(2, [&secondCalls](const Damage&) { ++secondCalls; });

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{1});
    EXPECT_EQ(firstCalls, 1);
    EXPECT_EQ(secondCalls, 0);
}

TEST(EventBusTests, UnsubscribeDuringPublish_SkipsSubscriberNotYetCalled)
{
    EventBus bus;
    int firstCalls = 0;
    int secondCalls = 0;
    SubscriptionId secondId = kInvalidSubscription;

    bus.subscribe<Damage>(1, [&bus, &firstCalls, &secondId](const Damage&) {
        ++firstCalls;
        bus.unsubscribe(secondId);
    });
    secondId = bus.subscribe<Damage>(2, [&secondCalls](const Damage&) { ++secondCalls; });

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{1});
    EXPECT_EQ(firstCalls, 1);
    EXPECT_EQ(secondCalls, 0);
}

TEST(EventBusTests, UnsubscribeDuringPublish_SelfUnsubscribeStopsFutureCalls)
{
    EventBus bus;
    SubscriptionId id = kInvalidSubscription;
    int calls = 0;

    id = bus.subscribe<Damage>(1, [&bus, &id, &calls](const Damage&) {
        ++calls;
        bus.unsubscribe(id);
    });

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{1});
    EXPECT_EQ(bus.publish(Damage{}), std::size_t{0});
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(bus.subscriberCount<Damage>(), std::size_t{0});
}

TEST(EventBusTests, SubscribeDuringPublish_DoesNotReceiveCurrentEvent)
{
    EventBus bus;
    int firstCalls = 0;
    int addedCalls = 0;
    bool subscribed = false;

    bus.subscribe<Damage>(1, [&bus, &firstCalls, &addedCalls, &subscribed](const Damage&) {
        ++firstCalls;
        if (!subscribed) {
            subscribed = true;
            bus.subscribe<Damage>(2, [&addedCalls](const Damage&) { ++addedCalls; });
        }
    });

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{1});
    EXPECT_EQ(firstCalls, 1);
    EXPECT_EQ(addedCalls, 0);
    EXPECT_EQ(bus.subscriberCount<Damage>(), std::size_t{2});

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{2});
    EXPECT_EQ(firstCalls, 2);
    EXPECT_EQ(addedCalls, 1);
}

TEST(EventBusTests, Publish_ReentrantFromHandler_DeliversNestedEvent)
{
    EventBus bus;
    int damageCalls = 0;
    int healedAmount = 0;

    bus.subscribe<Damage>(1, [&bus, &damageCalls](const Damage&) {
        ++damageCalls;
        bus.publish(Healed{7});
    });
    bus.subscribe<Healed>(2, [&healedAmount](const Healed& event) { healedAmount += event.amount; });

    EXPECT_EQ(bus.publish(Damage{}), std::size_t{1});
    EXPECT_EQ(damageCalls, 1);
    EXPECT_EQ(healedAmount, 7);
}

TEST(EventBusTests, Publish_EventEnvelope_DeliversPayload)
{
    EventBus bus;
    int amount = 0;

    bus.subscribe<Damage>(1, [&amount](const Damage& event) { amount += event.amount; });

    const std::size_t invoked = bus.publish(Event<Damage>{Damage{9}});

    EXPECT_EQ(invoked, std::size_t{1});
    EXPECT_EQ(amount, 9);
}

TEST(EventBusTests, Event_ExposesPayloadTypeAndValue)
{
    static_assert(std::is_same_v<Event<Damage>::Payload, Damage>);

    const Event<Damage> event{Damage{3}};

    EXPECT_EQ(event.payload.amount, 3);
}

TEST(EventBusTests, Publish_CopiesLatestPayloadValue)
{
    EventBus bus;
    int received = 0;

    bus.subscribe<Damage>(1, [&received](const Damage& event) { received = event.amount; });

    const Damage event{42};
    bus.publish(event);

    EXPECT_EQ(received, 42);
}

TEST(EventBusTests, Clear_RemovesAllSubscribers)
{
    EventBus bus;
    int calls = 0;

    bus.subscribe<Damage>(1, [&calls](const Damage&) { ++calls; });
    bus.subscribe<Healed>(2, [&calls](const Healed&) { ++calls; });

    bus.clear();

    EXPECT_EQ(bus.subscriberCount<Damage>(), std::size_t{0});
    EXPECT_EQ(bus.subscriberCount<Healed>(), std::size_t{0});
    EXPECT_EQ(bus.publish(Damage{}), std::size_t{0});
    EXPECT_EQ(bus.publish(Healed{}), std::size_t{0});
    EXPECT_EQ(calls, 0);
}

}  // namespace
