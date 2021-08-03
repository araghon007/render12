#pragma once

#include "DynamicBuffer12.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

class ComplexSurfaceRenderer
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 TexCoords;
        DirectX::XMFLOAT2 TexCoords1;
        unsigned int PolyFlags;
        unsigned int TexFlags;
        unsigned int TextureIndex;
    };

    explicit ComplexSurfaceRenderer(ID3D12Device& Device, ID3D12RootSignature& RootSignature, ID3D12GraphicsCommandList& CommandList);
    ComplexSurfaceRenderer(const ComplexSurfaceRenderer&) = delete;
    ComplexSurfaceRenderer& operator=(const ComplexSurfaceRenderer&) = delete;

    void NewFrame(const size_t iFrameIndex);
    void StartBatch() { assert(!m_bInBatch);  m_VertexBuffer.StartBatch(); m_IndexBuffer.StartBatch(); m_bInBatch = true; }
    void StopBatch() { m_bInBatch = false; }
    bool InBatch() const { return m_bInBatch; }

    void Bind();
    void Draw();

    Vertex* GetTriangleFan(const size_t iSize) { return DynamicGPUBufferHelpers12::GetTriangleFan(m_VertexBuffer, m_IndexBuffer, iSize); }

    //Diagnostics
    size_t GetNumIndices() const { return m_IndexBuffer.GetSize(); }
    size_t GetNumDraws() const { return m_iNumDraws; }
    size_t GetMaxIndices() const { return m_IndexBuffer.GetReserved(); }

protected:
    ID3D12Device& m_Device;
    ID3D12RootSignature& m_RootSignature;
    ID3D12GraphicsCommandList& m_CommandList;

    ShaderCompiler::CompiledShader m_pVertexShader;
    ShaderCompiler::CompiledShader m_pPixelShader;

    ComPtr<ID3D12PipelineState> m_PipelineState;

    DynamicBuffer12<Vertex> m_VertexBuffer;  //We only create a per-instance-data buffer, we don't use a vertex buffer as vertex positions are irrelevant
    DynamicBuffer12<unsigned int> m_IndexBuffer;

    unsigned int iList[DynamicGPUBufferHelpers12::Fan2StripIndices(4096) * 2];

    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    size_t m_iNumDraws = 0; //Number of draw calls this frame, for stats
    bool m_bInBatch = false;
};
