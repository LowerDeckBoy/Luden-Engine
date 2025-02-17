#include "ShaderCompiler.hpp"
#include "D3D12/D3D12Utility.hpp"

#pragma comment(lib, "dxcompiler")

namespace Luden
{
	static std::string ShaderStageToString(ShaderStageFlag ShaderStage)
	{
		switch (ShaderStage)
		{
		
		case Luden::ShaderStageFlag::Vertex:			return "vs_6_6";
		case Luden::ShaderStageFlag::Pixel:				return "ps_6_6";
		case Luden::ShaderStageFlag::Amplification:		return "as_6_6";
		case Luden::ShaderStageFlag::Mesh:				return "ms_6_6";
		case Luden::ShaderStageFlag::RayGen:			return "lib_6_6";
		case Luden::ShaderStageFlag::ClosestHit:		return "lib_6_6";
		case Luden::ShaderStageFlag::Miss:				return "lib_6_6";
		case Luden::ShaderStageFlag::None:				FALLTHROUGH;
		default:										return "invalid";
		}
	}

	ShaderCompiler::ShaderCompiler()
	{
		VERIFY_D3D12_RESULT(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_DxcCompiler)));
		VERIFY_D3D12_RESULT(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_DxcUtils)));
		VERIFY_D3D12_RESULT(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_DxcLibrary)));
		VERIFY_D3D12_RESULT(m_DxcUtils->CreateDefaultIncludeHandler(&m_DxcIncludeHandler));

	}

	ShaderCompiler::~ShaderCompiler()
	{
		SAFE_RELEASE(m_DxcCompiler);
		SAFE_RELEASE(m_DxcUtils);
		SAFE_RELEASE(m_DxcLibrary);
		SAFE_RELEASE(m_DxcIncludeHandler);
	}

	D3D12Shader ShaderCompiler::CompileVS(Filepath Path, std::string_view EntryPoint)
	{
		return Compile(Path, ShaderStageFlag::Vertex, EntryPoint);
	}

	D3D12Shader ShaderCompiler::CompileAS(Filepath Path, std::string_view EntryPoint)
	{
		return Compile(Path, ShaderStageFlag::Amplification, EntryPoint);
	}

	D3D12Shader ShaderCompiler::CompileMS(Filepath Path, std::string_view EntryPoint)
	{
		return Compile(Path, ShaderStageFlag::Mesh, EntryPoint);
	}

	D3D12Shader ShaderCompiler::CompilePS(Filepath Path, std::string_view EntryPoint)
	{
		return Compile(Path, ShaderStageFlag::Pixel, EntryPoint);
	}

	D3D12Shader ShaderCompiler::Compile(Filepath Path, ShaderStageFlag ShaderStage, std::string_view EntryPoint)
	{
		const auto path = File::GetRelativePath(Path);

		uint32_t codePage = DXC_CP_ACP;
		IDxcBlobEncoding* sourceBlob{};
		VERIFY_D3D12_RESULT(m_DxcUtils->LoadFile(String::ToWide(path).c_str(), &codePage, &sourceBlob));

		std::wstring shaderType = String::ToWide(ShaderStageToString(ShaderStage));
		std::wstring parentPath = String::ToWide(File::GetParentPath(Path));

		std::wstring entryPoint = String::ToWide(EntryPoint);

		std::vector<LPCWSTR> arguments = {
			// Entry point
			L"-E", entryPoint.c_str(),
			// Target (i.e. vs_6_0)
			L"-T", shaderType.c_str(),
			// Include paths: without them, it can cause issues when trying to do includes inside hlsl
			L"-I Shaders/",
			L"-I ", parentPath.c_str(),
			// HLSL version: 2021 is latest
			L"-HV 2021",
			DXC_ARG_ALL_RESOURCES_BOUND,
	#if defined (_DEBUG)
			DXC_ARG_DEBUG,
			DXC_ARG_DEBUG_NAME_FOR_SOURCE,
			DXC_ARG_SKIP_OPTIMIZATIONS,
	#else
			DXC_ARG_OPTIMIZATION_LEVEL3
	#endif
		};


		DxcBuffer buffer{ 
			sourceBlob->GetBufferPointer(), 
			sourceBlob->GetBufferSize(),
			DXC_CP_ACP };

		Ref<IDxcResult> dxcResult;
		VERIFY_D3D12_RESULT(m_DxcCompiler.Get()->Compile(&buffer, arguments.data(), static_cast<uint32>(arguments.size()), m_DxcIncludeHandler.Get(), IID_PPV_ARGS(&dxcResult)));

		IDxcBlobUtf8* errors = nullptr;
		IDxcBlobUtf16* outputName = nullptr;
		dxcResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &outputName);
		if (errors && errors->GetStringLength() > 0)
		{
			//std::string errorMessage = std::format("File: {}\n\n{}", Path.c_str(), (char*)errors->GetBufferPointer());
			std::string errorMessage = std::format("{}", (char*)errors->GetBufferPointer());
			::MessageBoxA(nullptr, errorMessage.c_str(), "HLSL Error", MB_OK);
			throw std::runtime_error((char*)errors->GetBufferPointer());
		}

		IDxcBlob* blob = nullptr;
		VERIFY_D3D12_RESULT(dxcResult->GetResult(&blob));

		D3D12Shader output;
		output.ShaderName = "test";
		output.Data = blob->GetBufferPointer();
		output.Size = blob->GetBufferSize();
		output.ShaderStage = ShaderStage;

		return output;
		//return D3D12Shader("test", blob->GetBufferPointer(), blob->GetBufferSize(), ShaderStage);
	}
} // namespace Luden
