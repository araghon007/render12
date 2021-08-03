#include "stdafx.h"
#include "GouraudRenderer.h"
#include "Helpers.h"
#include "ShaderCompiler.h"

GouraudRenderer::GouraudRenderer(ID3D12Device& Device, ID3D12RootSignature& RootSignature, ID3D12GraphicsCommandList& CommandList)
:m_Device(Device)
, m_RootSignature(RootSignature)
, m_CommandList(CommandList)
, m_VertexBuffer(Device, CommandList, 4096)
, m_IndexBuffer(Device, CommandList, DynamicGPUBufferHelpers12::Fan2StripIndices(m_VertexBuffer.GetReserved()))
{
    ShaderCompiler Compiler(L"Render12\\Gouraud.hlsl");
    const auto VertexShader = Compiler.CompileVertexShader();
    const auto PixelShader = Compiler.CompilePixelShader();

    const D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
    {
        {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BlendIndices", 0, DXGI_FORMAT_R32G32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} //Todo make 8 bits, if necessary at all -> can't, hlsl doesn't support 8 bit data type
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
    psoDesc.pRootSignature = &m_RootSignature;
    psoDesc.VS = VertexShader.GetByteCode();
    psoDesc.PS = PixelShader.GetByteCode();
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = 0;
    psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    psoDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFail(Device.CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)), L"Failed to create pipeline state object.");

    //Translucent

    psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND::D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

    ThrowIfFail(Device.CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineStateTranslucent)), L"Failed to create pipeline state object.");

    //Modulated

    psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND::D3D12_BLEND_DEST_COLOR;
    psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND::D3D12_BLEND_SRC_COLOR;
    psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

    ThrowIfFail(Device.CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineStateModulated)), L"Failed to create pipeline state object.");

    //UpdateSubresources(&m_CommandList, iBuffer, m_IndexBuffer.Get(), 0, 0, 1, &indexData);

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    indexBufferView.BufferLocation = m_IndexBuffer.Get()->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    indexBufferView.SizeInBytes = m_IndexBuffer.GetSizeBytes();

    /*
    indexBufferView.BufferLocation = m_IndexBuffer.GetAddress();
    indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    indexBufferView.SizeInBytes = m_IndexBuffer.GetSizeInBytes();
    */
    //ShaderCompiler Compiler(m_Device, L"Render12\\Gouraud.hlsl");
    //m_pVertexShader = Compiler.CompileVertexShader();

    //const D3D11_INPUT_ELEMENT_DESC InputElementDescs[] =
    //{
    //    {"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"BlendIndices", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} //Todo make 8 bits, if necessary at all -> can't, hlsl doesn't support 8 bit data type
    //};

    //m_pInputLayout = Compiler.CreateInputLayout(InputElementDescs, _countof(InputElementDescs));

    //m_pPixelShader = Compiler.CompilePixelShader();
}


void GouraudRenderer::NewFrame(const size_t iFrameIndex)
{
    m_iNumDraws = 0;
    m_VertexBuffer.NewFrame(iFrameIndex);
    m_IndexBuffer.NewFrame(iFrameIndex);
    currFlags = 0;
}

void GouraudRenderer::Bind(DWORD PolyFlags)
{
    // Switch PSO depending on flags
    if (PolyFlags & PF_Translucent)
        m_CommandList.SetPipelineState(m_PipelineStateTranslucent.Get());
    else if (PolyFlags & PF_Modulated)
        m_CommandList.SetPipelineState(m_PipelineStateModulated.Get());
    else
        m_CommandList.SetPipelineState(m_PipelineState.Get());

    currFlags = PolyFlags;


    //m_CommandList.ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(iBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST));

    //UpdateSubresources(&m_CommandList, iBuffer, m_IndexBuffer.Get(), 0, 0, 1, &indexData);

    /*
    m_DeviceContext.IASetInputLayout(m_pInputLayout.Get());

    const UINT Strides[] = {sizeof(Vertex)};
    const UINT Offsets[] = {0};

    m_DeviceContext.IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), Strides, Offsets);
    m_DeviceContext.IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_DeviceContext.VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_DeviceContext.PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    */
}

void GouraudRenderer::Draw()
{
    m_CommandList.IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    m_CommandList.IASetVertexBuffers(0, 1, &m_VertexBuffer.GetView());

    m_CommandList.IASetIndexBuffer(&indexBufferView);

    auto a = m_IndexBuffer.GetNumNewElements();
    auto b = m_VertexBuffer.GetFirstElementForFrame();
    auto c = m_IndexBuffer.GetFirstNewElementIndex();
    //m_CommandList.DrawInstanced(a, 1, b, 0); //Just draw the entire thing as 1 big triangle strip, because 
    
    m_CommandList.DrawIndexedInstanced(a, 1, c, b, 0); //Better, but why
    m_iNumDraws++;
}

