#pragma once

#include "DynamicBuffer12.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

class GouraudRenderer
{
public:
    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Color;
        DirectX::XMFLOAT2 TexCoords;
        unsigned int PolyFlags;
        unsigned int TextureIndex;
    };

    explicit GouraudRenderer(ID3D12Device& Device, ID3D12RootSignature& RootSignature, ID3D12GraphicsCommandList& CommandList);
    GouraudRenderer(const GouraudRenderer&) = delete;
    GouraudRenderer& operator=(const GouraudRenderer&) = delete;
    
    void NewFrame(const size_t iFrameIndex);
    void StartBatch() { assert(!m_bInBatch);  m_InstanceBuffer.StartBatch(); m_bInBatch = true; }
    void StopBatch() { m_bInBatch = false; }
    bool InBatch() const { return m_bInBatch; }

    void Bind();
    void Draw();

    Vertex* GetTriangleFan(const size_t iSize) { return m_InstanceBuffer.PushBack(iSize); }

    Vertex& GetTriangle() { return m_InstanceBuffer.PushBack(); }

    //Diagnostics
    size_t GetNumIndices() const { return m_InstanceBuffer.GetSize(); }
    size_t GetNumDraws() const { return m_iNumDraws; }
    size_t GetMaxIndices() const { return m_InstanceBuffer.GetReserved(); }

protected:
    ID3D12Device& m_Device;
    ID3D12RootSignature& m_RootSignature;
    ID3D12GraphicsCommandList& m_CommandList;

    ShaderCompiler::CompiledShader m_pVertexShader;
    ShaderCompiler::CompiledShader m_pPixelShader;

    ComPtr<ID3D12PipelineState> m_PipelineState;

    DynamicBuffer12<Vertex> m_InstanceBuffer;  //We only create a per-instance-data buffer, we don't use a vertex buffer as vertex positions are irrelevant
    //DynamicBuffer12<unsigned short> m_IndexBuffer;

    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    size_t m_iNumDraws = 0; //Number of draw calls this frame, for stats
    bool m_bInBatch = false;
};