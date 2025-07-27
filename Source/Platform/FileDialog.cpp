#include <string>
#include "Platform.hpp"
#include <shobjidl.h>
#include "FileDialog.hpp"
#include <vector>
#include <print>

namespace Luden::Platform
{
	static std::string NarrowStringDown(std::wstring Text)
	{
		std::string narrowed{};
		narrowed.resize(Text.length());

		wcstombs_s(nullptr, &narrowed[0], narrowed.size() + 1, Text.data(), Text.size());
		
		return narrowed;
	}

	static void CheckHRESULT(HRESULT hResult)
	{
		if (SUCCEEDED(hResult))
		{
			return;
		}

		char hResultErrorMessage[512]{};
		::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			static_cast<DWORD>(hResult),
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			hResultErrorMessage, (sizeof(hResultErrorMessage) / sizeof(char)),
			nullptr);

		std::println("HRESULT failed: {}", hResultErrorMessage);
		
	}

	static std::vector<COMDLG_FILTERSPEC> ExtensionFilterToTypes(EExtensionFilter Extension)
	{
		switch (Extension)
		{
		case Luden::Platform::EExtensionFilter::Scene:
		{
			return std::vector<COMDLG_FILTERSPEC>{
				{ Filters::JSON,	L"*.json"	},
				{ Filters::Any,		L"*.*"		}
			};
		}
		case Luden::Platform::EExtensionFilter::Model:
		{
			return std::vector<COMDLG_FILTERSPEC>{
				{ L"3D",			L"*.gltf;*.glb;*.fbx;*.obj"	},
				{ Filters::GLTF,	L"*.gltf;*.glb"				},
				{ Filters::FBX,		L"*.fbx"					},
				{ Filters::OBJ,		L"*.obj"					},
				{ Filters::Any,		L"*.*"						}
			};
		}
		case Luden::Platform::EExtensionFilter::Image:
		{
			return std::vector<COMDLG_FILTERSPEC>{
				{ L"Image",				L"*.jpg;*.jpeg;*.png;*.dds" },
				{ Filters::JPG,			L"*.jpg;*.jpeg"				},
				{ Filters::PNG,			L"*.png"					},
				{ Filters::DDS,			L"*.dds"					},
				{ Filters::Any,			L"*.*"						}
			};
		}
		case Luden::Platform::EExtensionFilter::Any: [[fallthrough]];
		default:
		{
			return std::vector<COMDLG_FILTERSPEC>{
				{ Filters::Any,		L"*.*" }
			};
		}
		}
	}
	
	std::string FileDialog::Open(FOpenDialogOptions Options)
	{
		IFileOpenDialog* pFileOpen{};
		std::string output = "";

		// Create the FileOpenDialog object.
		HRESULT hResult = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));

		if (FAILED(hResult))
		{
			return output;
		}

		const auto extensions = ExtensionFilterToTypes(Options.FilterExtensions);

		CheckHRESULT(pFileOpen->SetFileTypes(static_cast<uint32_t>(extensions.size()), extensions.data()));
		CheckHRESULT(pFileOpen->SetOptions(FOS_DONTADDTORECENT | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM | FOS_STRICTFILETYPES | FOS_PATHMUSTEXIST));
		CheckHRESULT(pFileOpen->SetTitle(std::wstring(Options.Title.begin(), Options.Title.end()).c_str()));

		std::wstring defaultFolderLocation = std::wstring(Options.OpenLocation.begin(), Options.OpenLocation.end());
		IShellItem2* pDefaultFolder = nullptr;
		CheckHRESULT(::SHCreateItemFromParsingName(defaultFolderLocation.c_str(), nullptr, IID_PPV_ARGS(&pDefaultFolder)));

		CheckHRESULT(pFileOpen->SetFolder(pDefaultFolder));

		if (SUCCEEDED(hResult))
		{
			// Show the Open dialog box.
			hResult = pFileOpen->Show(nullptr);

			// TODO:
			if (hResult == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No items were selected.
			{
				pFileOpen->Release();
				return output;
			}

			// Get the file name from the dialog box.
			if (SUCCEEDED(hResult))
			{
				IShellItem* pItem = nullptr;
				hResult = pFileOpen->GetResult(&pItem);

				if (SUCCEEDED(hResult))
				{
					PWSTR pszFilePath{};
					hResult = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					if (SUCCEEDED(hResult))
					{
						output = NarrowStringDown(pszFilePath);

						::CoTaskMemFree(pszFilePath);
						pItem->Release();
						pFileOpen->Release();

						return output;
					}
				}
			}

			pFileOpen->Release();
		}

		return output;
	}

} // namespace Luden::Platform
