#include "Core.hpp"
#include <comdef.h>
#include <libassert/assert.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        void CheckD3DResult(HRESULT result) {
            ASSERT(result == S_OK, ("Expected S_OK but found " + eastl::string(_com_error(result).ErrorMessage())).c_str());
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios