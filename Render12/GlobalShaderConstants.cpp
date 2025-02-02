#include "stdafx.h"
#include "GlobalShaderConstants.h"

GlobalShaderConstants::GlobalShaderConstants(ID3D12Device& Device, ID3D12GraphicsCommandList& CommandList)
:m_CommandList(CommandList)
{
    std::array<CD3DX12_DESCRIPTOR_RANGE, 1> Ranges;
    Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0);

    std::array<CD3DX12_ROOT_PARAMETER, 2> Parameters;
    Parameters[0].InitAsConstants(sizeof(m_CBufPerFrame) / sizeof(int32_t), 0, 0, D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL); //TODO: only for pixel/vertex shader -> just OR-ing doesn't work
    Parameters[1].InitAsDescriptorTable(Ranges.size(), Ranges.data(), D3D12_SHADER_VISIBILITY_PIXEL);

    std::array<D3D12_STATIC_SAMPLER_DESC, 1> Samplers;
    Samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    Samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    Samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    Samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    Samplers[0].MipLODBias = 0;
    Samplers[0].MaxAnisotropy = 0;
    Samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    Samplers[0].MinLOD = 0.0f;
    Samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    Samplers[0].ShaderRegister = 0;
    Samplers[0].RegisterSpace = 0;
    Samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    const D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

    const CD3DX12_ROOT_SIGNATURE_DESC RSDesc(Parameters.size(), Parameters.data(), Samplers.size(), Samplers.data(), RootSignatureFlags);

    ComPtr<ID3DBlob> pSignature;
    ComPtr<ID3DBlob> pError;
    ThrowIfFail(D3D12SerializeRootSignature(&RSDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError), L"Failed to serialize root signature.");
    ThrowIfFail(Device.CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), __uuidof(m_pRootSignature), &m_pRootSignature), L"Failed to create root signature.");
    m_pRootSignature->SetName(L"The root signature");

    m_Viewport.MaxDepth = 1.0;
}

void GlobalShaderConstants::SetSceneNode(const FSceneNode& SceneNode)
{
    assert(SceneNode.Viewport);
    assert(SceneNode.Viewport->Actor);
    assert(reinterpret_cast<uintptr_t>(&m_CBufPerFrame.ProjectionMatrix) % 16 == 0);

    //TODO: we don't need any of the SceneNode precalculated values, so remove calculation from render.dll
    if (SceneNode.Viewport->Actor->FovAngle == m_fFov && SceneNode.FX == m_Viewport.Height && SceneNode.FY == m_Viewport.Width)
    {
        return;
    }

    //Create projection matrix with swapped near/far for better accuracy
    static const float fZNear = 32760.0f;
    static const float fZFar = 1.0f;

    const float fAspect = SceneNode.FX / SceneNode.FY;
    const float fFovVert = SceneNode.Viewport->Actor->FovAngle / fAspect * static_cast<float>(PI) / 180.0f;

    m_CBufPerFrame.fRes[0] = SceneNode.FX;
    m_CBufPerFrame.fRes[1] = SceneNode.FY;
    m_CBufPerFrame.fRes[2] = 1.0f/SceneNode.FX;
    m_CBufPerFrame.fRes[3] = 1.0f/SceneNode.FY;
    m_CBufPerFrame.ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fFovVert, fAspect, fZNear, fZFar);
    m_CBufPerFrame.ProjectionMatrix.r[1].m128_f32[1] *= -1.0f; //Flip Y

    m_fFov = SceneNode.Viewport->Actor->FovAngle;
    m_Viewport.Height = SceneNode.FY;
    m_Viewport.Width = SceneNode.FX;
    m_ScissorRect.right = SceneNode.X;
    m_ScissorRect.bottom = SceneNode.Y;
}

void GlobalShaderConstants::Bind()
{
    m_CommandList.SetGraphicsRootSignature(m_pRootSignature.Get());
    m_CommandList.SetGraphicsRoot32BitConstants(0, sizeof(m_CBufPerFrame) / sizeof(int32_t), &m_CBufPerFrame, 0);
    m_CommandList.RSSetViewports(1, &m_Viewport);
    m_CommandList.RSSetScissorRects(1, &m_ScissorRect);
}


