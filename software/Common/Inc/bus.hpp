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
#include <cstddef>
#include <span>

#include "callback.hpp"
#include "handle.hpp"

namespace obc {
class Bus {
  public:
    using Packet    = std::span<std::byte>;
    using MessageId = std::uint32_t;

    class ProcessCallback : public Callback<void, const Packet> {
      public:
        template<typename T, typename R, MethodPtrAlt<T, const R&> M>
        ProcessCallback(T& instance, std::uint64_t data)
            : Callback<void, const Packet>(
                  &instance, &MethodWrapper<T, R, M>, data
              ) {}

        template<typename T, MethodPtr<T> M>
        ProcessCallback(T& instance, std::uint64_t data)
            : Callback<void, const Packet>(
                  &instance, &DirectMethodWrapper<T, M>, data
              ) {}

      private:
        template<typename T, typename R, MethodPtrAlt<T, const R&> M>
        static void MethodWrapper(
            void* callee, std::uint64_t data, const Packet packet
        ) {
            assert(sizeof(R) == packet.size());
            (static_cast<T*>(callee)->*M)(
                data, *reinterpret_cast<R>(packet.begin())
            );
        }

        template<typename T, MethodPtr<T> M>
        static void DirectMethodWrapper(
            void* callee, std::uint64_t data, const Packet packet
        ) {
            (static_cast<T*>(callee)->*M)(data, packet);
        }
    };

    class TestCallback : public Callback<bool, const Packet> {
      public:
        template<typename T, MethodPtr<T> M>
        TestCallback(T& instance, std::uint64_t data)
            : Callback<bool, const Packet>(
                  &instance, &MethodWrapper<T, M>, data
              ) {}

      private:
        template<typename T, MethodPtr<T> M>
        static bool MethodWrapper(void* callee, const Packet packet) {
            return (static_cast<T*>(callee)->*M)(packet);
        }
    };

    class RequestHandleData {
        friend Bus;

      public:
        inline bool Complete() const { return m_complete; }

      private:
        MessageId       m_id;
        ProcessCallback m_callback;
        bool            m_complete;
    };

    using RequestHandle = Handle<RequestHandleData, Bus>;

    class ListenHandleData {
        friend Bus;

      private:
        TestCallback    m_test;
        ProcessCallback m_process;
    };

    using ListenHandle = Handle<ListenHandleData, Bus>;

    void Send(Packet message);

    template<typename T>
    void Send(const T& message) {
        Send(std::span(reinterpret_cast<std::byte*>(&message), sizeof(message))
        );
    }

    RequestHandle Request(Packet request, ProcessCallback callback);

    template<typename T>
    RequestHandle Request(const T& request, ProcessCallback callback) {
        return Request(
            std::span(reinterpret_cast<std::byte*>(&request), sizeof(request)),
            callback
        );
    }

    ListenHandle Listen(
        TestCallback test_callback, ProcessCallback process_callback
    );

  protected:
    virtual MessageId SendImpl(Packet message) = 0;

    inline void ProcessMessage(Packet message, MessageId id) {
        for (auto rh_ptr = request_chain.Top(); !rh_ptr; *rh_ptr++) {
            auto& rh {*rh_ptr};
            if (rh->m_id == id) {
                rh->m_callback(message);
                rh->m_complete;
                return;
            }
        }

        for (auto lh_ptr = listen_chain.Top(); !lh_ptr; *lh_ptr++) {
            auto& lh {*lh_ptr};
            if (lh->m_test(message)) {
                lh->m_process(message);
                return;
            }
        }

        // TODO: log unhandled message
    }

  private:
    RequestHandle::Chain request_chain {};
    ListenHandle::Chain  listen_chain {};
};
}  // namespace obc
