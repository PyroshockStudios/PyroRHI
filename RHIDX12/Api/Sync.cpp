#include "Sync.hpp"
#include "Device.hpp"

namespace PyroshockStudios {
    namespace RHIDX12 {
        D3DSemaphore::D3DSemaphore(D3DDevice* device, SemaphoreInfo&& info)
            : mDevice(device), mInfo(eastl::move(info)) {
            mDevice->InternalDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
        }
        D3DSemaphore::~D3DSemaphore() {
        }
        const SemaphoreInfo& D3DSemaphore::Info() const {
            return mInfo;
        }
        D3DFence::D3DFence(D3DDevice* device, FenceInfo&& info)
            : mDevice(device), mInfo(eastl::move(info)) {
            mDevice->InternalDevice()->CreateFence(mInfo.initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
            mEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        }
        D3DFence::~D3DFence() {
            if (mEvent)
                CloseHandle(mEvent);
        }

        const FenceInfo& D3DFence::Info() const {
            return mInfo;
        }
        u64 D3DFence::Value() const {
            return mFence->GetCompletedValue();
        }
        void D3DFence::SetValue(u64 value) {
            mCurrentValue = value;
            mFence->Signal(value);
        }
        bool D3DFence::WaitForValue(u64 value, u64 timeoutNs) {
            if (Value() >= value)
                return true;
            DWORD timeoutMs = (timeoutNs == UINT64_MAX) ? INFINITE : static_cast<DWORD>(timeoutNs / 1000000ULL);

            HRESULT hr = mFence->SetEventOnCompletion(value, mEvent);
            if (FAILED(hr))
                return false;

            DWORD result = WaitForSingleObject(mEvent, timeoutMs);
            return result == WAIT_OBJECT_0;
        }
    }
}