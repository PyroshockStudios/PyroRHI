#pragma once

#include <EASTL/deque.h>
#include <EASTL/optional.h>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace PyroshockStudios {
    namespace Common {
        template <typename T>
        class AtomicQueue {
        public:
            AtomicQueue() = default;
            ~AtomicQueue() { Clear(); }

            void Push(const T& value) {
                std::scoped_lock lk(mObjectLock);
                mDeque.push_back(value);
                Notify();
            }
            void Push(T&& value) {
                std::scoped_lock lk(mObjectLock);
                mDeque.emplace_back(value);
                Notify();
            }

            T Pop() {
                std::scoped_lock lk(mObjectLock);
                assert(mDeque.size() && "Cannot pop from empty deque!");

                T value = mDeque.front();
                mDeque.pop_front();
                return value;
            }

            eastl::optional<T> TryPop() {
                std::scoped_lock lk(mObjectLock);
                if (mDeque.empty()) {
                    return eastl::nullopt;
                }
                T value = mDeque.front();
                mDeque.pop_front();
                return eastl::optional<T>(value);
            }

            void Clear() {
                std::scoped_lock lk(mObjectLock);
                mDeque.clear();
            }

            bool Empty() const {
                std::scoped_lock lk(mObjectLock);
                return mDeque.empty();
            }
            void Wait() const {
                std::unique_lock waitLock(mBlockingLock);
                mCv.wait(waitLock, [this] { return mWaitReady; });
                mWaitReady = false;
            }
            template <typename R, typename P>
            void WaitFor(const std::chrono::duration<R, P>& duration) const {
                std::unique_lock waitLock(mBlockingLock);
                mCv.wait_for(waitLock, duration, [this] { return mWaitReady; });
                mWaitReady = false;
            }

            usize Size() const {
                std::scoped_lock lk(mObjectLock);
                return mDeque.size();
            }

            std::mutex& GetLock() { return mObjectLock; }
            void Notify() const {
                {
                    std::lock_guard lck(mBlockingLock);
                    mWaitReady = true;
                }
                mCv.notify_one();
            }

        protected:
            mutable std::condition_variable mCv;
            mutable std::mutex mBlockingLock;
            mutable std::mutex mObjectLock;
            mutable bool mWaitReady = false;

        private:
            eastl::deque<T> mDeque;
        };
    } // namespace Common
} // namespace PyroshockStudios