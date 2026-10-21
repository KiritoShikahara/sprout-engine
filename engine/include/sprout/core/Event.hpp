#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace sprout {

/// @brief 購読の所有者を表す ID。
///
/// GameObject のスロット番号など、破棄時にまとめて購読を解除したい単位を表す。
using EventOwner = std::uint64_t;

/// @brief 購読を識別する ID。0 は無効値。
using SubscriptionId = std::uint64_t;

/// @brief 無効な購読 ID。
inline constexpr SubscriptionId kInvalidSubscription = 0;

/// @brief 所有者を持たない購読に使う ID。
inline constexpr EventOwner kNoOwner = 0;

/// @brief 型付きイベントのペイロードを包む値型。
///
/// EventBus::publish(Event<T>) で包んだペイロードを発行できる。
///
/// @code
/// struct Damaged { int amount; };
/// bus.publish(sprout::Event<Damaged>{Damaged{5}});
/// @endcode
/// @tparam T ペイロードの型。
template <typename T>
struct Event {
    /// ペイロードの型。
    using Payload = T;

    /// イベントの内容。
    T payload;

    /// ペイロードを指定して生成する。
    /// @param value イベントの内容。
    explicit Event(T value)
        : payload{std::move(value)}
    {
    }
};

namespace detail {

class EventBusImpl;

using ErasedHandler = std::function<void(const void*)>;

[[nodiscard]] SubscriptionId subscribeErased(EventBusImpl& impl, EventOwner owner, std::type_index type,
                                             ErasedHandler handler);
bool unsubscribeErased(EventBusImpl& impl, SubscriptionId id);
std::size_t unsubscribeOwnerErased(EventBusImpl& impl, EventOwner owner);
std::size_t publishErased(EventBusImpl& impl, std::type_index type, const void* event);
std::size_t subscriberCountErased(const EventBusImpl& impl, std::type_index type);
void clearErased(EventBusImpl& impl);

}  // namespace detail

/// @brief 型ごとに購読者を持つイベントバス。
///
/// 購読は SubscriptionId で解除するか、EventOwner 単位でまとめて解除する。
/// 発行中に購読・解除しても現在の発行は安全に継続する。発行中に追加された購読は
/// その発行では呼ばれず、解除された購読は以降の呼び出しをスキップする。
/// ハンドラは publish を呼んだスレッドで、購読順に呼ばれる。
///
/// @code
/// struct Damaged { int amount; };
///
/// sprout::EventBus bus;
/// const auto id = bus.subscribe<Damaged>(owner, [](const Damaged& event) { /* ... */ });
/// bus.publish(Damaged{5});
/// bus.unsubscribe(id);
/// @endcode
class EventBus {
public:
    EventBus();
    ~EventBus();

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    /// @brief イベント型 TEvent の購読を登録する。
    /// @param owner 購読の所有者。unsubscribeAll() でまとめて解除できる。
    /// @param handler `const TEvent&` を受け取る呼び出し可能オブジェクト。
    /// @return 登録した購読の ID。必ず kInvalidSubscription 以外。
    template <typename TEvent, typename THandler>
    SubscriptionId subscribe(EventOwner owner, THandler&& handler)
    {
        using Payload = std::decay_t<TEvent>;
        static_assert(std::is_invocable_v<THandler&, const Payload&>,
                      "EventBus::subscribe のハンドラは const TEvent& で呼び出せる必要があります");
        static_assert(std::is_copy_constructible_v<std::decay_t<THandler>>,
                      "EventBus::subscribe のハンドラはコピー構築できる必要があります");

        return detail::subscribeErased(
            *m_impl, owner, std::type_index{typeid(Payload)},
            [callback = std::forward<THandler>(handler)](const void* event) mutable {
                callback(*static_cast<const Payload*>(event));
            });
    }

    /// @brief 購読 ID を指定して解除する。
    /// @param id 解除する購読の ID。
    /// @return 解除できたら true。未登録・解除済みなら false。
    bool unsubscribe(SubscriptionId id);

    /// @brief 所有者に属する購読をすべて解除する。
    /// @param owner 解除する購読の所有者。
    /// @return 解除した購読の数。
    std::size_t unsubscribeAll(EventOwner owner);

    /// @brief すべての購読を解除する。
    void clear();

    /// @brief イベントを発行する。
    /// @param event 発行するイベント。
    /// @return 実際に呼び出したハンドラの数。
    template <typename TEvent>
    std::size_t publish(const TEvent& event)
    {
        using Payload = std::decay_t<TEvent>;
        return detail::publishErased(*m_impl, std::type_index{typeid(Payload)}, std::addressof(event));
    }

    /// @brief Event<TEvent> に包んだイベントを発行する。
    /// @param event 発行するイベント。
    /// @return 実際に呼び出したハンドラの数。
    template <typename TEvent>
    std::size_t publish(const Event<TEvent>& event)
    {
        return publish<TEvent>(event.payload);
    }

    /// @brief イベント型 TEvent の購読者数を返す。
    /// @return 登録されている購読の数。
    template <typename TEvent>
    [[nodiscard]] std::size_t subscriberCount() const
    {
        return detail::subscriberCountErased(*m_impl, std::type_index{typeid(std::decay_t<TEvent>)});
    }

private:
    std::unique_ptr<detail::EventBusImpl> m_impl;
};

}  // namespace sprout
