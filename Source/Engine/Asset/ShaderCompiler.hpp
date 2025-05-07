#pragma once

#include "D3D12/D3D12Shader.hpp"
#include <Core/File.hpp>
#include <Core/RefPtr.hpp>
#include <DXC/d3d12shader.h>
#include <DXC/dxcapi.h>

namespace Luden
{
	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		~ShaderCompiler();
	
		D3D12Shader Compile(Filepath Path, ShaderStageFlag ShaderStage, std::string_view EntryPoint, bool bHasRootSignature = false);

		D3D12Shader CompileVS(Filepath Path, bool bHasRootSignature = false, std::string_view EntryPoint = "VSMain");
		D3D12Shader CompileAS(Filepath Path, bool bHasRootSignature = false, std::string_view EntryPoint = "ASMain");
		D3D12Shader CompileMS(Filepath Path, bool bHasRootSignature = false, std::string_view EntryPoint = "MSMain");
		D3D12Shader CompilePS(Filepath Path, bool bHasRootSignature = false, std::string_view EntryPoint = "PSMain");
		D3D12Shader CompileCS(Filepath Path, bool bHasRootSignature = false, std::string_view EntryPoint = "CSMain");

	private:
		Ref<IDxcCompiler3>		m_DxcCompiler;
		Ref<IDxcIncludeHandler> m_DxcIncludeHandler;
		Ref<IDxcUtils>			m_DxcUtils;
		Ref<IDxcLibrary>		m_DxcLibrary;

	};
} // namespace Luden
