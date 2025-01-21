#include "Core/Logger.hpp"
#include <D3D12AgilitySDK/d3d12.h>
#include <Windows.h>
#include "D3D12Utility.hpp"
#include <exception>

namespace Luden::Util
{
	void VerifyD3D12Result(HRESULT hResult, const char* ResultString, std::source_location Source, std::string Message)
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

		std::string message = std::format("{}\n{}\n\n{}", hResultError, ResultString, where);

		if (!Message.empty())
		{
			message.insert(0, std::format("{}\n\n", Message.data()));
		}

		::MessageBoxA(nullptr, message.data(), "D3D12 Error", MB_OK);

		// Temporarly
		throw std::exception();
	}

	void NameD3D12Object(ID3D12Object* pD3D12Object, std::string_view DebugName)
	{
		if (!pD3D12Object)
		{
			LOG_WARNING("Trying to set name: '{}' for empty D3D12 object!", DebugName);

			return;
		}

		if (DebugName.empty())
		{
			LOG_WARNING("Trying to set empty name for D3D12 object!");

			return;
		}

		pD3D12Object->SetName(String::ToWide(DebugName).c_str());
	}
} // namespace Luden::Util
