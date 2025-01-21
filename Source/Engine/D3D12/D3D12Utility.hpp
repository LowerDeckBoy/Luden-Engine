#pragma once

#include "Core/String.hpp"
#include <source_location>

namespace Luden
{
	// Verify whether given HRESULT has succeded.
	// If not, output debug message what and where operation failed.
	#define VERIFY_D3D12_RESULT(Result, ...) \
		{ HRESULT result = Result;\
			Luden::Util::VerifyD3D12Result(result, #Result, std::source_location::current(), __VA_ARGS__); }

	// Release custom core::Ref<T> pointers of ID3D12 or IDXGI obejcts.
	#define SAFE_RELEASE(Ptr) if (Ptr) { Ptr.Reset(); Ptr = nullptr; }

	// Release raw ID3D12 or IDXGI object pointer.
	#define SAFE_DELETE(RawPtr) if (RawPtr) { RawPtr->Release(); RawPtr = nullptr; }

	#define NAME_D3D12_OBJECT(D3D12Object, Name) \
		Luden::Util::NameD3D12Object(D3D12Object, Name)

	namespace Util
	{
		extern void VerifyD3D12Result(HRESULT hResult, const char* ResultString, std::source_location Source, std::string Message = "");

		extern void NameD3D12Object(ID3D12Object* pD3D12Object, std::string_view DebugName);

	} // namespace Util
} // namespace Luden
