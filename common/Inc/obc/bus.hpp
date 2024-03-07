/* USER CODE BEGIN Header */
/*
 * 401 Ballon OBC
 * Copyright (C) 2023 BLUEsat and contributors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
/* USER CODE END Header */

#pragma once

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <optional>
#include <span>

#include "ipc/callback.hpp"
#include "utils/handle.hpp"

/**
 * @brief Defines a common interface for communication buses.
 *
 * The purpose of a bus is to abstract the network stack up to, but not
 * including, the application layer. Operations are defined in terms of
 * concepts, allowing implementers to choose a subset of supported operations
 * and users to select a subset of required operations. Mixins are provided to
 * aid in implementing some bus operations, though their use is optional.
 *
 * @warning Given its rarity in the majority of embedded communication
 * standards, and due to its limited utility and the complexity of
 * implementation, socket-style interfaces are not supported.
 *
 * Operations involve sending and/or receiving messages. The specific type of
 * message is a template parameter, allowing for additional functionality or
 * metadata to be provided. This approach is preferred over creating additional
 * functions to ensure consistency.
 *
 * TODO: Add error reporting.
 */
namespace obc::bus {
/**
 * @brief Represents a message sent or received by a bus.
 *
 * Must include an address and data (payload).
 *
 * @warning Addresses do not necessarily correspond to a single physical device
 * (or any device at all); buses may define implementation-specific addresses
 * that additional provide functionalities, including but not limited to:
 * multicasting, loopback, and voiding.
 */
template<typename T>
concept Message = requires(T& msg) {
    typename T::Address;
    typename T::Data;

    { msg.address } -> std::same_as<typename T::Address&>;
    { msg.data } -> std::same_as<typename T::Data&>;
} && std::regular<typename T::Address> && std::semiregular<typename T::Data>;

/**
 * @brief Represents a generic message type.
 *
 * If there are no special requirements, this should be used as the type for all
 * bus operations. Where feasible, more specialized message types should be
 * convertible to and from this basic message type, allowing users to ignore
 * more advanced functionality if not required.
 */
struct BasicMessage {
    using Address = std::uint32_t;
    using Data    = std::span<std::byte>;

    Address address;
    Data    data;
};

/**
 * @brief A generic buffer type which behaves like std::span.
 *
 * Used for storing the the payload of packets received in operations where the
 * size of the message may be unbound (from the perspective of the bus).
 *
 * @tparam V Fundamental unit of data stored in the buffer.
 */
template<typename T, typename V = std::byte>
concept Buffer = requires(T& buf, V v, std::size_t i) {
    { buf[i] } -> std::convertible_to<V>;
    { buf[i] = v };
    { buf.size() } -> std::convertible_to<std::size_t>;
} && std::semiregular<V>;

/**
 * @brief Convert a trivial struct into a byte buffer.
 *
 * Can be used to facilitate trivial deserialization of C-style structs sent on
 * a bus.
 *
 * @tparam T type of the struct to translate.
 */
template<typename T>
    requires std::is_trivially_copyable_v<T>
auto StructAsBuffer(T& s) {
    return std::span<std::byte, sizeof(T)> {
        reinterpret_cast<std::byte*>(&s), sizeof(T)};
}

/**
 * @brief Convert a trivial struct into a byte buffer.
 *
 * Can be used to facilitate trivial deserialization of C-style structs sent on
 * a bus.
 *
 * @tparam T type of the struct to translate.
 */
template<typename T>
    requires std::is_trivially_copyable_v<T>
auto StructAsBuffer(T&& s) {
    return std::span<std::byte, sizeof(T)> {
        reinterpret_cast<std::byte*>(&s), sizeof(T)};
}

/**
 * @brief Represents a bus that can send packets of data to an address.
 *
 * Includes a function `Send(msg)` for sending a message on the bus.
 *
 * @tparam M The type of message to send.
 */
template<typename T, typename M = BasicMessage>
concept SendBus = requires(T& bus, const M& msg) {
    { bus.Send(msg) } -> std::same_as<void>;
} && Message<M>;

/**
 * @brief Represents a bus that can listen to all received messages.
 *
 * Includes a function `Listen(cb)` for registering a listener callback that is
 * invoked for every incoming message.
 *
 * @tparam M The type of message received.
 */
template<typename T, typename M = BasicMessage>
concept ListenBus =
    requires(T& bus, ipc::Callback<void, const M&>::DummyProvider&& cb) {
        typename T::ListenHandle;

        { bus.Listen(cb.Func) } -> std::same_as<typename T::ListenHandle>;
    } && std::semiregular<typename T::ListenHandle> && Message<M>;

/**
 * @brief A helper class for implementing listening functionality.
 *
 * `FeedListeners` should be called for each incoming message. Typically
 * this should be invoked before feeding requesters and processors to
 * avoid them invalidating the data buffer which the message is potentially
 * stored in.
 *
 * @tparam M The type of message received.
 */
template<Message M = BasicMessage>
class ListenBusMixin {
    using ListenCallback = ipc::Callback<void, const M&>;

  public:
    using ListenHandle = utils::Handle<ListenCallback>;

    /**
     * @brief Adds a listener to be notified upon receiving a message.
     *
     * @param cb The listener callback to add.
     * @return An opaque handle that must be retained for the listener to remain
     * active.
     */
    ListenHandle Listen(ListenCallback&& cb) {
        return ListenHandle(m_listeners, std::move(cb));
    }

  protected:
    /**
     * @brief Forwards a message to all listeners.
     *
     * Should be called upon receiving any incoming message.
     *
     * @param msg The message to be forwarded.
     */
    void FeedListeners(const M& msg) {
        for (const auto& callback : m_listeners) callback(msg);
    }

  private:
    utils::HandleChainRoot<ListenCallback> m_listeners;
};

/**
 * @brief Represents a bus capable of requesting data from a remote device.
 *
 * Includes a function `Request(req, cb)` for sending a request and registering
 * a callback that is invoked when a response arrives. Multiple responses per
 * request are allowed.
 *
 * @tparam Req The type of message sent to request data.
 * @tparam Res The type of message received in response.
 * @tparam BufUnit Fundemental type of data stored in the response buffer.
 * @tparam Buf The type of buffer used by the response.
 */
template<
    typename T, typename Req = BasicMessage, typename Res = BasicMessage,
    typename BufUnit = std::byte, typename Buf = std::span<BufUnit>>
concept RequestBus =
    requires(
        T& bus, const Req& req,
        ipc::Callback<void, const Res&>::DummyProvider cb, Buf& buf
    ) {
        typename T::RequestHandle;

        {
            bus.Request(req, cb.Func, buf)
        } -> std::same_as<typename T::RequestHandle>;
    } &&
    std::semiregular<typename T::RequestHandle> && Message<Req> &&
    Message<Res> && Buffer<Buf, BufUnit>;

/**
 * @brief Represents a callable that returns a bool in response to a packet.
 *
 * Used to match an arbitrary criteria on a packet to filter them.
 */
template<typename T, typename M = BasicMessage>
concept MessageFilter = requires(T& filter, const M& msg) {
    { filter(msg) } -> std::convertible_to<bool>;
} && std::semiregular<T> && Message<M>;

/**
 * @brief Specifies the required methods for the class \ref RequestBusMixin is
 * applied to.
 *
 * Must define `IssueRequest`, a function that dispatches the request to the
 * bus; it should return a filter that identifies the response to the
 * request.
 *
 * @tparam Req The type of message sent to request data.
 * @tparam Res The type of message received in response.
 * @tparam Flt The object used to determine if an incoming message is a response
 */
template<
    typename T, typename Req = BasicMessage, typename Res = BasicMessage,
    typename Flt = ipc::Callback<bool, const Res&>>
concept RequestBusMixinRequirements = requires(T& bus, const Req& req) {
    { bus.IssueRequest(req) } -> std::same_as<Flt>;
} && Message<Req> && Message<Res> && MessageFilter<Flt>;

/**
 * @brief A helper class for implementing request functionality.
 *
 * @tparam D CRTP derived class, must satisfy \ref RequestBusMixinRequirements.
 * @tparam Req The type of message sent to request data.
 * @tparam Res The type of message received in response.
 * @tparam Flt The object used to determine if an incoming message is a response
 * to a particular request.
 */
template<
    typename D, Message Req = BasicMessage, Message Res = BasicMessage,
    MessageFilter<Res> Flt     = ipc::Callback<bool, const Res&>,
    std::semiregular   BufUnit = std::byte,
    Buffer<BufUnit>    Buf     = std::span<BufUnit>>
class RequestBusMixin {
    using RequestCallback = ipc::Callback<void, const Res&>;

    struct RequestHandleData {
        Flt             filter;
        RequestCallback callback;
    };

  public:
    using RequestHandle = utils::Handle<RequestHandleData>;

    /**
     * @brief Sends a request on the bus.
     *
     * @param req The request message.
     * @param cb The callback to be invoked upon receiving the response.
     * @return An opaque handle that must be retained until the request is
     * fulfilled or the response is no longer desired.
     */
    RequestHandle Request(const Req& req, RequestCallback&& cb, Buf buf) {
        return RequestHandle(
            m_request_handlers,
            {
                .filter   = Derived().IssueRequest(req, buf),
                .callback = std::move(cb),
            }
        );
    }

  protected:
    /**
     * @brief Forwards a message to all pending requests.
     *
     * Should be called upon receiving any messages that could be responses to a
     * request.
     *
     * @param res The response message to forward.
     */
    void FeedRequesters(const Res& res) {
        for (auto& handler : m_request_handlers)
            if (handler.data.filter(res)) handler.data.callback(res);
    }

  private:
    /**
     * @brief Safely gets the CRTP derived class.
     *
     * @return Reference to derived class.
     */
    RequestBusMixinRequirements<Req, Flt> auto& Derived() {
        return static_cast<D&>(*this);
    }

    utils::HandleChainRoot<RequestHandleData> m_request_handlers;
};

/**
 * @brief Represents a bus capable of responding to received requests.
 *
 * Includes a function `Process(cb)` for registering a callback that returns a
 * response if it can handle a given request.
 *
 * TODO: Implement supplying request buffers.
 *
 * @tparam Req The type of message received that requests data.
 * @tparam Res The type of message sent in response.
 */
template<typename T, typename Req = BasicMessage, typename Res = BasicMessage>
concept ProcessBus =
    requires(T& bus, ipc::Callback<std::optional<Res>, Req>::DummyProvider cb) {
        typename T::ProcessHandle;

        { bus.Process(cb.Func) } -> std::same_as<typename T::ProcessHandle>;
    } && std::semiregular<typename T::ProcessHandle> && Message<Req> &&
    Message<Res>;

/**
 * @brief Specifies the required methods for the class \ref ProcessBusMixin is
 * applied to.
 *
 * Must define `IssueResponse`, a function that dispatches the response to
 * the bus.
 *
 * @tparam Req The type of message received that requests data.
 * @tparam Res The type of message sent in response.
 */
template<typename T, typename Req = BasicMessage, typename Res = BasicMessage>
concept ProcessBusMixinRequirements =
    requires(T& bus, const Req& req, const Res& res) {
        { bus.IssueResponse(req, res) } -> std::same_as<void>;
    } && Message<Req> && Message<Res>;

/**
 * @brief A helper class for implementing processing functionality.
 *
 * @tparam D CRTP derived class, must satisfy \ref ProcessBusMixinRequirements.
 * @tparam Req The type of message received that requests data.
 * @tparam Res The type of message sent in response.
 */
template<typename D, Message Req = BasicMessage, Message Res = BasicMessage>
class ProcessBusMixin {
    using ProcessCallback = ipc::Callback<std::optional<Res>, Req>;

  public:
    using ProcessHandle = utils::Handle<ProcessCallback>;

    /**
     * @brief Registers a processor callback.
     *
     * @param cb The callback to be invoked upon receiving a message that may be
     * a request.
     * @return An opaque handle that must be retained for the processor to
     * remain active.
     */
    ProcessHandle Process(ProcessCallback&& cb) {
        return ProcessHandle(m_processors, std::move(cb));
    }

  protected:
    /**
     * @brief Forwards a message to processors.
     *
     * Should be called upon receiving any messages that could be requests.
     *
     * @param req The request message to forward.
     */
    void FeedProcessors(const Req& req) {
        for (const auto& processor : m_processors)
            if (auto res = processor(req)) Derived().IssueResponse(req, *res);
    }

  private:
    /**
     * @brief Safely gets the CRTP derived class.
     *
     * @return Reference to derived class.
     */
    ProcessBusMixinRequirements<Req, Res> auto& Derived() {
        return static_cast<D&>(*this);
    }

    utils::HandleChainRoot<ProcessCallback> m_processors;
};
}  // namespace obc::bus
