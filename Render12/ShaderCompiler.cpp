#include "stdafx.h"
#include "ShaderCompiler.h"
#include "Helpers.h"

ShaderCompiler::ShaderCompiler(const wchar_t* const pszFileName)
:m_pszFileName(pszFileName)
{

}

ShaderCompiler::CompiledShader ShaderCompiler::CompileShader(const char* const pszEntryPoint, const char* const pszTarget)
{
    LOGMESSAGEF(L"Compiling \"%s\" for target %S.", m_pszFileName, pszTarget);

    UINT iFlags = 0;
#ifdef _DEBUG
    iFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> pMessages;
    ComPtr<ID3DBlob> pBlob;
    const HRESULT hResult = D3DCompileFromFile(m_pszFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszEntryPoint, pszTarget, iFlags, 0, &pBlob, &pMessages);

    if (pMessages)
    {
        LOGWARNINGF(L"Shader compilation:\r\n%S", static_cast<char*>(pMessages->GetBufferPointer()));
        OutputDebugStringA(static_cast<char*>(pMessages->GetBufferPointer()));
        DebugBreak();
    }

    ThrowIfFail(hResult, L"Failed to compile shader '%s'.", m_pszFileName);

    return CompiledShader(pBlob.Detach());

}

ShaderCompiler::CompiledShader ShaderCompiler::CompileVertexShader()
{
    return CompileShader("VSMain", "vs_5_1");
}

ShaderCompiler::CompiledShader ShaderCompiler::CompileGeometryShader()
{
    return CompileShader("GSMain", "gs_5_1");
}

ShaderCompiler::CompiledShader ShaderCompiler::CompilePixelShader()
{
    return CompileShader("PSMain", "ps_5_1");
}
