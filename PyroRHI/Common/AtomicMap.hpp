#pragma once

#include <PyroCommon/Core.hpp>

#include <EASTL/hash_map.h>
#include <EASTL/optional.h>

#include <cassert>
#include <condition_variable>
#include <mutex>

namespace PyroshockStudios {
    namespace Common {
        namespace internal_AtomicMap {
            template <typename T, typename K, typename V>
            concept LoopActionConcept = requires(T lambda, const K& p1, V& p2) {
                { lambda(p1, p2) } -> std::same_as<void>;
            };
            template <typename T, typename K, typename V>
            concept ConstLoopActionConcept = requires(T lambda, const K& p1, const V& p2) {
                { lambda(p1, p2) } -> std::same_as<void>;
            };
            template <typename T, typename V>
            concept RefActionConcept = requires(T lambda, V& p1) {
                { lambda(p1) } -> std::same_as<void>;
            };
            template <typename T, typename V>
            concept ConstRefActionConcept = requires(T lambda, const V& p1) {
                { lambda(p1) } -> std::same_as<void>;
            };
            template <typename T, typename V>
            concept OptionalRefActionConcept = requires(T lambda, eastl::optional<eastl::reference_wrapper<V>> p1) {
                { lambda(p1) } -> std::same_as<void>;
            };
            template <typename T, typename V>
            concept OptionalConstRefActionConcept =
                requires(T lambda, eastl::optional<eastl::reference_wrapper<const V>> p1) {
                    { lambda(p1) } -> std::same_as<void>;
                };
        } // namespace internal_AtomicMap
        template <typename K, typename V>
        class AtomicMap {
        public:
            AtomicMap() = default;
            ~AtomicMap() { Clear(); }

            void Emplace(const K& key, V&& value) {
                std::scoped_lock lk(mObjectLock);
                mMap.emplace(key, eastl::move(value));
                Notify();
            }
            void Emplace(const K& key, const V& value) {
                std::scoped_lock lk(mObjectLock);
                mMap.emplace(key, value);
                Notify();
            }

            PYRO_NODISCARD V Get(const K& key) const {
                std::scoped_lock lk(mObjectLock);
                return mMap.at(key);
            }

            PYRO_NODISCARD V GetOr(const K& key, const V& defaultv = V()) const {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                return it == mMap.end() ? defaultv : it->second;
            }

            PYRO_NODISCARD eastl::optional<V> GetOpt(const K& key) const {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                return it == mMap.end() ? eastl::nullopt : eastl::make_optional(it->second);
            }

            void Set(const K& key, const V& value) {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                if (it == mMap.end()) {
                    mMap.emplace(key, value);
                    Notify();
                }
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicMap::LoopActionConcept<K, V> TAction>
            void ForEach(TAction&& action) {
                std::scoped_lock lk(mObjectLock);
                for (auto& [k, v] : mMap) {
                    action(k, v);
                }
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicMap::ConstLoopActionConcept<K, V> TAction>
            void ConstForEach(TAction&& action) const {
                std::scoped_lock lk(mObjectLock);
                for (const auto& [k, v] : mMap) {
                    action(k, v);
                }
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicMap::RefActionConcept<V> TAction>
            void Ref(const K& key, TAction&& action) {
                std::scoped_lock lk(mObjectLock);
                action(mMap.at(key));
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicMap::ConstRefActionConcept<V> TAction>
            void ConstRef(const K& key, TAction&& action) const {
                std::scoped_lock lk(mObjectLock);
                action(mMap.at(key));
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicMap::OptionalRefActionConcept<V> TAction>
            void RefOr(const K& key, TAction&& action) {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                action(it == mMap.end() ? eastl::nullopt : eastl::make_optional(it->second));
            }

            // do NOT access the map inside the callback!!!
            template <internal_AtomicMap::OptionalConstRefActionConcept<V> TAction>
            void ConstRefOr(const K& key, TAction&& action) const {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                action(it == mMap.end() ? eastl::nullopt : eastl::make_optional(it->second));
            }

            void Erase(const K& key) {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                if (it == mMap.end()) {
                    return;
                }
                mMap.erase(it);
            }

            V Extract(const K& key) {
                std::scoped_lock lk(mObjectLock);
                auto it = mMap.find(key);
                assert(it != mMap.end() && "Cannot extract non existing item!");
                V val = it->second;
                mMap.erase(it);
                return val;
            }

            PYRO_NODISCARD bool Contains(const K& key) const {
                std::scoped_lock lk(mObjectLock);
                return mMap.find(key) != mMap.end();
            }

            void Clear() {
                std::scoped_lock lk(mObjectLock);
                mMap.clear();
            }

            PYRO_NODISCARD bool Empty() const {
                std::scoped_lock lk(mObjectLock);
                return mMap.empty();
            }

            PYRO_NODISCARD size_t Size() const {
                std::scoped_lock lk(mObjectLock);
                return mMap.size();
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

            PYRO_NODISCARD eastl::hash_map<K, V>& UnderlyingMap() { return mMap; }

        private:
            mutable std::condition_variable mCv;
            mutable std::mutex mBlockingLock;
            mutable std::mutex mObjectLock;
            mutable bool mWaitReady = false;

            eastl::hash_map<K, V> mMap;
        };
    } // namespace Common
} // namespace PyroshockStudios