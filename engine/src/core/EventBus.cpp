#include "sprout/core/Event.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sprout::detail {
namespace {

struct Subscription {
    Subscription(SubscriptionId subscriptionId, EventOwner subscriptionOwner, std::type_index eventType)
        : id{subscriptionId}
        , owner{subscriptionOwner}
        , type{eventType}
    {
    }

    SubscriptionId id;
    EventOwner owner;
    std::type_index type;
    std::atomic<bool> active{true};
    ErasedHandler handler;
};

struct Channel {
    std::vector<std::shared_ptr<Subscription>> subscriptions;
};

}  // namespace

class EventBusImpl {
public:
    mutable std::mutex mutex;
    std::unordered_map<std::type_index, Channel> channels;
    std::unordered_map<SubscriptionId, std::shared_ptr<Subscription>> subscriptions;
    SubscriptionId nextId{1};
};

namespace {

void eraseFromChannel(EventBusImpl& impl, const std::shared_ptr<Subscription>& subscription)
{
    const auto channelIt = impl.channels.find(subscription->type);
    if (channelIt == impl.channels.end()) {
        return;
    }

    auto& subscriptions = channelIt->second.subscriptions;
    subscriptions.erase(std::remove(subscriptions.begin(), subscriptions.end(), subscription), subscriptions.end());
}

}  // namespace

SubscriptionId subscribeErased(EventBusImpl& impl, EventOwner owner, std::type_index type, ErasedHandler handler)
{
    const std::lock_guard lock(impl.mutex);
    if (impl.nextId == kInvalidSubscription) {
        impl.nextId = 1;
    }

    const SubscriptionId id = impl.nextId++;
    auto subscription = std::make_shared<Subscription>(id, owner, type);
    subscription->handler = std::move(handler);
    impl.channels[type].subscriptions.push_back(subscription);
    impl.subscriptions.emplace(id, std::move(subscription));
    return id;
}

bool unsubscribeErased(EventBusImpl& impl, SubscriptionId id)
{
    const std::lock_guard lock(impl.mutex);
    const auto it = impl.subscriptions.find(id);
    if (it == impl.subscriptions.end()) {
        return false;
    }

    const std::shared_ptr<Subscription> subscription = it->second;
    impl.subscriptions.erase(it);
    subscription->active.store(false, std::memory_order_release);
    eraseFromChannel(impl, subscription);
    return true;
}

std::size_t unsubscribeOwnerErased(EventBusImpl& impl, EventOwner owner)
{
    const std::lock_guard lock(impl.mutex);

    std::vector<std::shared_ptr<Subscription>> removed;
    for (auto it = impl.subscriptions.begin(); it != impl.subscriptions.end();) {
        if (it->second->owner == owner) {
            removed.push_back(it->second);
            it = impl.subscriptions.erase(it);
        } else {
            ++it;
        }
    }

    for (const auto& subscription : removed) {
        subscription->active.store(false, std::memory_order_release);
        eraseFromChannel(impl, subscription);
    }
    return removed.size();
}

std::size_t publishErased(EventBusImpl& impl, std::type_index type, const void* event)
{
    std::vector<std::shared_ptr<Subscription>> snapshot;
    {
        const std::lock_guard lock(impl.mutex);
        const auto it = impl.channels.find(type);
        if (it == impl.channels.end()) {
            return 0;
        }
        snapshot = it->second.subscriptions;
    }

    std::size_t invoked = 0;
    for (const auto& subscription : snapshot) {
        if (!subscription->active.load(std::memory_order_acquire)) {
            continue;
        }
        subscription->handler(event);
        ++invoked;
    }
    return invoked;
}

std::size_t subscriberCountErased(const EventBusImpl& impl, std::type_index type)
{
    const std::lock_guard lock(impl.mutex);
    const auto it = impl.channels.find(type);
    return it == impl.channels.end() ? 0 : it->second.subscriptions.size();
}

void clearErased(EventBusImpl& impl)
{
    const std::lock_guard lock(impl.mutex);
    for (auto& entry : impl.channels) {
        for (const auto& subscription : entry.second.subscriptions) {
            subscription->active.store(false, std::memory_order_release);
        }
    }
    impl.channels.clear();
    impl.subscriptions.clear();
}

}  // namespace sprout::detail

namespace sprout {

EventBus::EventBus()
    : m_impl{std::make_unique<detail::EventBusImpl>()}
{
}

EventBus::~EventBus() = default;

bool EventBus::unsubscribe(SubscriptionId id)
{
    return detail::unsubscribeErased(*m_impl, id);
}

std::size_t EventBus::unsubscribeAll(EventOwner owner)
{
    return detail::unsubscribeOwnerErased(*m_impl, owner);
}

void EventBus::clear()
{
    detail::clearErased(*m_impl);
}

}  // namespace sprout
