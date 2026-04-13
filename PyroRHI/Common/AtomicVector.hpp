#pragma once

#include <PyroCommon/Core.hpp>

#include <EASTL/hash_map.h>
#include <EASTL/vector.h>

#include <cassert>
#include <condition_variable>
#include <mutex>

namespace PyroshockStudios {
    namespace Common {
        namespace internal_AtomicVector {
            template <typename T, typename V>
            concept RefActionConcept = requires(T lambda, V& p1) {
                { lambda(p1) } -> std::same_as<void>;
            };
            template <typename T, typename V>
            concept ConstRefActionConcept = requires(T lambda, const V& p1) {
                { lambda(p1) } -> std::same_as<void>;
            };
        } // namespace internal_AtomicVector
        template <typename T>
        class AtomicVector {
        public:
            AtomicVector() = default;
            ~AtomicVector() { Clear(); }

            template <typename... Args>
            void EmplaceBack(Args... args) {
                std::scoped_lock lk(mObjectLock);
                mVector.emplace_back(eastl::forward<Args>(args)...);
                Notify();
            }
            void PushBack(const T& value) {
                std::scoped_lock lk(mObjectLock);
                mVector.push_back(value);
                Notify();
            }

            PYRO_NODISCARD const T& At(usize index) const {
                std::scoped_lock lk(mObjectLock);
                return mVector.at(index);
            }

            void Assign(usize index, const T& value) {
                std::scoped_lock lk(mObjectLock);
                mVector[index] = value;
                Notify();
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicVector::RefActionConcept<T> TAction>
            void ForEach(TAction&& action) {
                std::scoped_lock lk(mObjectLock);
                for (auto& v : mVector) {
                    action(v);
                }
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicVector::ConstRefActionConcept<T> TAction>
            void ForEach(TAction&& action) const {
                std::scoped_lock lk(mObjectLock);
                for (const auto& v : mVector) {
                    action(v);
                }
            }
            void Erase(usize index) {
                std::scoped_lock lk(mObjectLock);
                mVector.erase(mVector.begin() + index);
            }
            void Erase(usize begin, usize end) {
                std::scoped_lock lk(mObjectLock);
                mVector.erase(mVector.begin() + begin, mVector.begin() + end);
            }
            void EraseFirstItem(const T& v) {
                std::scoped_lock lk(mObjectLock);
                mVector.erase(eastl::find(mVector.begin(), mVector.end(), v));
            }

            T PopBack() {
                std::scoped_lock lk(mObjectLock);
                T v = mVector.back();
                mVector.pop_back();
                return v;
            }

            void Clear() {
                std::scoped_lock lk(mObjectLock);
                mVector.clear();
            }

            PYRO_NODISCARD bool Empty() const {
                std::scoped_lock lk(mObjectLock);
                return mVector.empty();
            }

            PYRO_NODISCARD size_t Size() const {
                std::scoped_lock lk(mObjectLock);
                return mVector.size();
            }

            PYRO_NODISCARD bool Contains(const T& v) const {
                std::scoped_lock lk(mObjectLock);
                return eastl::find(mVector.begin(), mVector.end(), v) != mVector.end();
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
            void Notify() const {
                {
                    std::lock_guard lck(mBlockingLock);
                    mWaitReady = true;
                }
                mCv.notify_one();
            }

            PYRO_NODISCARD std::mutex& GetLock() { return mObjectLock; }

            PYRO_NODISCARD eastl::vector<T>& UnderlyingVector() { return mVector; }

        private:
            mutable std::condition_variable mCv;
            mutable std::mutex mBlockingLock;
            mutable std::mutex mObjectLock;
            mutable bool mWaitReady = false;

            eastl::vector<T> mVector;
        };
    } // namespace Common
} // namespace PyroshockStudios