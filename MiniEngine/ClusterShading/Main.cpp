#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "GameInput.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "BufferManager.h"

#include"FbxLoader/FbxLoader.h"

using namespace GameCore;
using namespace Graphics;

#pragma comment(lib, "FbxLoader.lib")

class ClusterShading : public GameCore::IGameApp
{
public:

	ClusterShading()
	{
	}

	virtual void Startup(void) override;
	virtual void Cleanup(void) override;

	virtual void Update(float deltaT) override;
	virtual void RenderScene(void) override;

private:
	struct ConstantBufferData0
	{
		XMFLOAT4X4 World;
		XMFLOAT4X4 ViewProjection;
	};

	GpuResource g_SponzaVb;
	size_t g_SponzaVb_Size;
	size_t m_sponza_vertex_count;
	GpuResource g_SponzaIb;
	size_t g_SponzaIb_Size;
	size_t m_sponza_index_count;

	GraphicsPSO g_general_pso;

	ID3DBlob* g_default_vs;
	ID3DBlob* g_default_ps;

	D3D12_SHADER_BYTECODE g_default_shader_vs;
	D3D12_SHADER_BYTECODE g_default_shader_ps;

	RootSignature g_default_shader_rs;
	RootParameter g_default_shader_cb0;

	GpuResource m_constant_buffer_0;
	ConstantBufferData0 m_constant_data_0;
	
	DepthBuffer m_depth_0;

	ID3D12InfoQueue* m_Info_Queue;
	ID3D12Debug* m_Debug_Layer;
};

CREATE_APPLICATION(ClusterShading)

void ClusterShading::Startup(void)
{
	USES_CONVERSION;
	AllocConsole();

	std::cout << "Engine Initialize\n";

	std::string executablePath;
	std::string assetPath;
	std::string projectPath;
	std::string sponzaPath;
	std::string sponza_shader_path;

	char buffer[256];
	GetModuleFileNameA(nullptr, buffer, 256);

	executablePath = buffer;
	projectPath = executablePath.substr(0, executablePath.find_last_of('\\'));
	projectPath = projectPath.substr(0, projectPath.find_last_of('\\'));
	projectPath = projectPath.substr(0, projectPath.find_last_of('\\'));
	projectPath = projectPath.substr(0, projectPath.find_last_of('\\'));
	projectPath = projectPath.substr(0, projectPath.find_last_of('\\'));
	projectPath = projectPath.substr(0, projectPath.find_last_of('\\'));

	projectPath += "\\ClusterShading";
	assetPath = projectPath + "\\assets";

	sponzaPath = assetPath + "\\sponza.fbx";
	sponza_shader_path = assetPath + "\\Default.hlsl";

	ID3DBlob* errblob = nullptr;
	HRESULT compileResult = D3DCompileFromFile(A2W(sponza_shader_path.c_str()),
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vert", "vs_5_0", D3DCOMPILE_DEBUG, 0, &g_default_vs, &errblob);
	if (errblob != nullptr || compileResult != S_OK)
	{
		std::cout << (const char*)errblob->GetBufferPointer() << '\n';
		errblob->Release();
		errblob = nullptr;
	}

	compileResult = D3DCompileFromFile(A2W(sponza_shader_path.c_str()),
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "frag", "ps_5_0", D3DCOMPILE_DEBUG, 0, &g_default_ps, &errblob);
	if (errblob != nullptr)
	{
		std::cout << (const char*)errblob->GetBufferPointer() << '\n';
		errblob->Release();
		errblob = nullptr;
	}


	g_default_shader_vs = { g_default_vs->GetBufferPointer(), g_default_vs->GetBufferSize() };
	g_default_shader_ps = { g_default_ps->GetBufferPointer(), g_default_ps->GetBufferSize() };

	g_general_pso = GraphicsPSO(L"General PSO");
	g_general_pso.SetVertexShader(g_default_shader_vs);
	g_general_pso.SetPixelShader(g_default_shader_ps);

	g_default_shader_rs.Reset(1, 0);
	g_default_shader_cb0.InitAsConstantBuffer(0);
	g_default_shader_rs[0] = g_default_shader_cb0;
	g_default_shader_rs.Finalize(L"Default Shader RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	CD3DX12_RASTERIZER_DESC default_rasterizer = CD3DX12_RASTERIZER_DESC::CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
	
	CD3DX12_BLEND_DESC default_blend = CD3DX12_BLEND_DESC::CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
	CD3DX12_DEPTH_STENCIL_DESC default_depth_stencil = CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());// CD3DX12_DEPTH_STENCIL_DESC::CD3DX12_DEPTH_STENCIL_DESC(true, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_ALWAYS, false, 0, 0, D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO, D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO, D3D12_STENCIL_OP_ZERO, D3D12_COMPARISON_FUNC_ALWAYS);
	//default_depth_stencil.DepthEnable = true;
	//default_depth_stencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//default_depth_stencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//default_depth_stencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	//default_depth_stencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	//
	D3D12_INPUT_ELEMENT_DESC default_input_elements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0xffffffff, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0xffffffff, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0xffffffff, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0xffffffff, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 4, 0xffffffff, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		/*
			LPCSTR SemanticName;
	UINT SemanticIndex;
	DXGI_FORMAT Format;
	UINT InputSlot;
	UINT AlignedByteOffset;
	D3D12_INPUT_CLASSIFICATION InputSlotClass;
	UINT InstanceDataStepRate;*/
	};

	g_general_pso.SetRasterizerState(default_rasterizer);
	g_general_pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	g_general_pso.SetRenderTargetFormat(DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_D32_FLOAT);
	g_general_pso.SetRootSignature(g_default_shader_rs);
	g_general_pso.SetBlendState(default_blend);
	g_general_pso.SetDepthStencilState(default_depth_stencil);
	g_general_pso.SetInputLayout(ARRAYSIZE(default_input_elements), default_input_elements);
	g_general_pso.SetSampleMask(0xFFFFFFFF);
	g_general_pso.Finalize();

	CD3DX12_RESOURCE_DESC constant_buffer_desc0 = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBufferData0));
	CD3DX12_HEAP_PROPERTIES constant_buffer_heap_props{}; // = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE, D3D12_MEMORY_POOL_L0);
	constant_buffer_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	constant_buffer_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

	ID3D12Resource* constant_buffer_raw;
	SUCCEEDED(g_Device->CreateCommittedResource(&constant_buffer_heap_props, D3D12_HEAP_FLAG_NONE, &constant_buffer_desc0, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), (void**)&constant_buffer_raw));
	
	m_constant_buffer_0 = GpuResource(constant_buffer_raw, D3D12_RESOURCE_STATE_GENERIC_READ);

	XMMATRIX view, projection;

	view = XMMatrixLookAtLH(XMVectorSet(0.0f, 5.0f, -1.0f, 1.0f), XMVectorSet(0.0f, 5.0f, 0.0, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(75.0f), 1.777f, 0.001f, 10000.0f);

	D3D12_RANGE constant_buffer_map_range{};
	constant_buffer_map_range.End = sizeof(ConstantBufferData0);
	void* constant_buffer_map_ptr;
	m_constant_buffer_0->Map(0, &constant_buffer_map_range, (void**)&constant_buffer_map_ptr);

	XMStoreFloat4x4(&m_constant_data_0.World, XMMatrixIdentity());
	XMStoreFloat4x4(&m_constant_data_0.ViewProjection, XMMatrixTranspose(view * projection));

	memcpy(constant_buffer_map_ptr, &m_constant_data_0, sizeof(ConstantBufferData0));
	
	m_constant_buffer_0->Unmap(0, &constant_buffer_map_range);

	FbxLoader sponzaLoader = FbxLoader(sponzaPath.c_str());

	ID3D12Resource* sponzaVb;
	ID3D12Resource* sponzaIb;

	m_sponza_vertex_count = sponzaLoader.Vertices.size();
	m_sponza_index_count = sponzaLoader.Indices.size();

	CD3DX12_RESOURCE_DESC sponzaVBDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * m_sponza_vertex_count);
	CD3DX12_RESOURCE_DESC sponzaIBDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned int) * m_sponza_index_count);
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	SUCCEEDED(g_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &sponzaVBDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), (void**)&sponzaVb));
	SUCCEEDED(g_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &sponzaIBDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), (void**)&sponzaIb));

	g_SponzaVb_Size = sizeof(Vertex) * m_sponza_vertex_count;
	g_SponzaIb_Size = sizeof(unsigned int) * m_sponza_index_count;

	g_SponzaVb = GpuResource(sponzaVb, D3D12_RESOURCE_STATE_GENERIC_READ);
	g_SponzaIb = GpuResource(sponzaIb, D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_RANGE vb_map_range{};
	vb_map_range.End = g_SponzaVb_Size;
	void* sponza_vb_map_ptr;
	g_SponzaVb->Map(0, &vb_map_range, &sponza_vb_map_ptr);
	memcpy(sponza_vb_map_ptr, sponzaLoader.Vertices.data(), g_SponzaVb_Size);
	g_SponzaVb->Unmap(0, &vb_map_range);


	D3D12_RANGE ib_map_range{};
	ib_map_range.End = g_SponzaIb_Size;
	void* sponza_ib_map_ptr;
	g_SponzaIb->Map(0, &ib_map_range, &sponza_ib_map_ptr);
	memcpy(sponza_ib_map_ptr, sponzaLoader.Indices.data(), g_SponzaIb_Size);
	g_SponzaIb->Unmap(0, &ib_map_range);

	HRESULT result = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&m_Debug_Layer);
	if (result != S_OK)
	{
		std::cout << "Failed to get debug interface\n";
	}
	g_Device->QueryInterface<ID3D12InfoQueue>(&m_Info_Queue);
	if (m_Info_Queue == nullptr)
	{
		std::cout << "Failed to Query ID3D12InfoQueue\n";
	}

	Graphics::ResizeDisplayDependentBuffers(1280, 720);

	g_SceneColorBuffer->SetName(L"RenderBuffer");
	g_SceneDepthBuffer->SetName(L"DepthBuffer");
}

void ClusterShading::Cleanup(void)
{
	// Free up resources in an orderly fashion
}

void ClusterShading::Update(float /*deltaT*/)
{
	ScopedTimer _prof(L"Update State");
	
	
	// Update something
}

void ClusterShading::RenderScene(void)
{
	GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");
	Color clearColor;
	clearColor.SetRGB(0.0f, 1.0f, 0.0f);
	g_SceneColorBuffer.SetClearColor(clearColor);
	gfxContext.FlushResourceBarriers();

	gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
	gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

	gfxContext.ClearColor(g_SceneColorBuffer);
	gfxContext.ClearDepth(g_SceneDepthBuffer);


	D3D12_GPU_VIRTUAL_ADDRESS vb_virtual_address = g_SponzaVb->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS ib_virtual_address = g_SponzaIb->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_virtual_address0 = m_constant_buffer_0->GetGPUVirtualAddress();
		
	D3D12_VERTEX_BUFFER_VIEW sponza_vbv{ vb_virtual_address,  g_SponzaVb_Size, sizeof(Vertex) };
	D3D12_INDEX_BUFFER_VIEW sponza_ibv{ ib_virtual_address,  g_SponzaIb_Size, DXGI_FORMAT_R32_UINT };

	gfxContext.SetVertexBuffer(0, sponza_vbv);
	gfxContext.SetIndexBuffer(sponza_ibv);
	
	gfxContext.SetPipelineState(g_general_pso);
	gfxContext.SetRootSignature(g_default_shader_rs);
	gfxContext.SetConstantBuffer(0, constant_buffer_virtual_address0);
	gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	gfxContext.DrawIndexed(m_sponza_index_count, 0, 0);

	//size_t message_count = m_Info_Queue->GetNumStoredMessages();
	//D3D12_MESSAGE message;
	//size_t message_size;

	//HRESULT result = m_Info_Queue->GetMessageW(message_count - 1, &message, &message_size);
	//if (result != S_OK)
	//{
	//	std::cout << "Something went wrong.\n";
	//}
	// Rendering something
	
	gfxContext.Finish();
}
