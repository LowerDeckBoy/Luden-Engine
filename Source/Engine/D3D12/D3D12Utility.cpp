#include "Core/Logger.hpp"
#include <D3D12AgilitySDK/d3d12.h>
#include <Core/RefPtr.hpp>
#include <Windows.h>
#include "D3D12Utility.hpp"
#include <exception>

namespace Luden
{
	void D3D::Delete(IUnknown* pInterface)
	{
		if (pInterface)
		{
			pInterface->Release();
			pInterface = nullptr;
		}
	}

	void Util::VerifyD3D12Result(HRESULT hResult, const char* ResultString, std::source_location Source, std::string Message)
	{
		if (SUCCEEDED(hResult))
		{
			return;
		}

		char hResultError[512]{};
		::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hResult, 0, hResultError, (sizeof(hResultError) / sizeof(char)), nullptr);

		const char* filename = strrchr(Source.file_name(), '\\') ? strrchr(Source.file_name(), '\\') + 1 : Source.file_name();

		std::string where = std::format(
			"File: {0} ({1}:{2})\nFunction: {3}\n",
			/* 0 */ filename,
			/* 1 */ Source.line(),
			/* 2 */ Source.column(),
			/* 3 */ Source.function_name());

		std::string message = std::format("{0}\n{1}\n\n{2}", hResultError, ResultString, where);

		if (!Message.empty())
		{
			message.insert(0, std::format("{0}\n\n", Message.data()));
		}

		::MessageBoxA(nullptr, message.data(), "D3D12 Error", MB_OK);

		// Temporarly
		throw std::exception();
	}

	void Util::NameD3D12Object(ID3D12Object* pD3D12Object, std::string_view DebugName)
	{
		if (!pD3D12Object)
		{
			LOG_WARNING("Trying to set name: '{}' for empty D3D12 object!", DebugName);

			return;
		}

		if (DebugName.empty())
		{
			return;
		}

		pD3D12Object->SetName(String::ToWide(DebugName).c_str());
	}
} // namespace Luden::Util
