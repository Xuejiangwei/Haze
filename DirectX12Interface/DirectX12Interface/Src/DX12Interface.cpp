#include "DX12Interface.h"

#include <iostream>
#include <Windows.h>
#include <unordered_map>


#include "Common/d3dUtil.h"
#include "Common/GameTimer.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

struct GRS_VERTEX
{
	DirectX::XMFLOAT4 m_vtPos;
	DirectX::XMFLOAT4 m_vtColor;
};

int mClientWidth = 720;
int mClientHeight = 720;
const int SwapChainBufferCount = 2;
int mCurrBackBuffer = 0;
HWND mhMainWnd = nullptr;

const float	faClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

//DX 变量
Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

//命令队列
Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

//交换链
Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
Microsoft::WRL::ComPtr<IDXGISwapChain3>	pISwapChain3;

//交换链Buffer
Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
UINT nFrameIndex = 0;

//描述符元素的大小
UINT mRtvDescriptorSize = 0;
UINT mDsvDescriptorSize = 0;
UINT mCbvSrvUavDescriptorSize = 0;

//RTV描述符堆
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;

//DSV描述符堆
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

//DSV Buffer
Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

//根描述符
Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;

//渲染管线对象
Microsoft::WRL::ComPtr<ID3D12PipelineState>	mPipelineState;

//顶点 Buffer
Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW mVertexBufferView = {};

//围栏
Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
UINT64 n64FenceValue = 0;
HANDLE hEventFence = nullptr;

//Viewport
D3D12_VIEWPORT stViewPort = { 0.0f, 0.0f, static_cast<float>(mClientWidth), static_cast<float>(mClientHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };

//裁剪矩形
D3D12_RECT stScissorRect = { 0, 0, static_cast<LONG>(mClientWidth), static_cast<LONG>(mClientHeight) };

LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.
		// We pause the game when the window is deactivated and unpause it
		// when it becomes active.
	case WM_ACTIVATE:
		/*if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}*/
		return 0;

		// WM_SIZE is sent when the user resizes the window.
	case WM_SIZE:
		// Save the new client area dimensions.
		//mClientWidth = LOWORD(lParam);
		//mClientHeight = HIWORD(lParam);
		//if (md3dDevice)
		//{
		//	if (wParam == SIZE_MINIMIZED)
		//	{
		//		mAppPaused = true;
		//		mMinimized = true;
		//		mMaximized = false;
		//	}
		//	else if (wParam == SIZE_MAXIMIZED)
		//	{
		//		mAppPaused = false;
		//		mMinimized = false;
		//		mMaximized = true;
		//		OnResize();
		//	}
		//	else if (wParam == SIZE_RESTORED)
		//	{
		//		// Restoring from minimized state?
		//		if (mMinimized)
		//		{
		//			mAppPaused = false;
		//			mMinimized = false;
		//			OnResize();
		//		}

		//		// Restoring from maximized state?
		//		else if (mMaximized)
		//		{
		//			mAppPaused = false;
		//			mMaximized = false;
		//			OnResize();
		//		}
		//		else if (mResizing)
		//		{
		//			// If user is dragging the resize bars, we do not resize
		//			// the buffers here because as the user continuously
		//			// drags the resize bars, a stream of WM_SIZE messages are
		//			// sent to the window, and it would be pointless (and slow)
		//			// to resize for each WM_SIZE message received from dragging
		//			// the resize bars.  So instead, we reset after the user is
		//			// done resizing the window and releases the resize bars, which
		//			// sends a WM_EXITSIZEMOVE message.
		//		}
		//		else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
		//		{
		//			OnResize();
		//		}
		//	}
		//}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		/*mAppPaused = true;
		mResizing = true;
		mTimer.Stop();*/
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		/*mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();*/
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses
		// a key that does not correspond to any mnemonic or accelerator key.
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		/*((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;*/
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		/*if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			Set4xMsaaState(!m4xMsaaState);*/
		return 0;
	case WM_KEYDOWN:
		//OnKeyDown(wParam);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return MsgProc(hwnd, msg, wParam, lParam);
}

void CreateWindowsWindow(HAZE_CALL_FUNC_PARAM)
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", L"Haze窗口", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return;
}

void InitDX12(HAZE_CALL_FUNC_PARAM)
{
	//创建factory
	CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory));

	//创建DX设备对象
	HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));
	if (FAILED(hardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));
	}

	//创建命令队列
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));

	//创建命令分配器
	md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf()));

	//创建命令列表
	md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf()));
	mCommandList->Close();

	//创建交换链
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	mdxgiFactory->CreateSwapChain(mCommandQueue.Get(), &sd, mSwapChain.GetAddressOf());
	mSwapChain.As(&pISwapChain3);
	nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

	//获得描述符堆大小
	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//创建Render Target View描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	//创建 SwapChainBufferCount 个Render Target View描述符
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i]));
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	//创建深度模板描述符堆
	//D3D12_RESOURCE_DESC depthStencilDesc;
	//depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//depthStencilDesc.Alignment = 0;
	//depthStencilDesc.Width = mClientWidth;
	//depthStencilDesc.Height = mClientHeight;
	//depthStencilDesc.DepthOrArraySize = 1;
	//depthStencilDesc.MipLevels = 1;
	//depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	//depthStencilDesc.SampleDesc.Count = 1;
	//depthStencilDesc.SampleDesc.Quality = 0;
	//depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//D3D12_CLEAR_VALUE optClear;
	//optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//optClear.DepthStencil.Depth = 1.0f;
	//optClear.DepthStencil.Stencil = 0;
	//md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
	//	D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));

	//// Create descriptor to mip level 0 of entire resource using the format of the resource.
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	//dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//dsvDesc.Texture2D.MipSlice = 0;
	//md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, mDsvHeap->GetCPUDescriptorHandleForHeapStart());

	//// Transition the resource from its initial state to be used as a depth buffer.
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
	//	D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	//// Execute the resize commands.
	//ThrowIfFailed(mCommandList->Close());
	//ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	//mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	//// Wait until resize is complete.
	//FlushCommandQueue();

	//// Update the viewport transform to cover the client area.
	//mScreenViewport.TopLeftX = 0;
	//mScreenViewport.TopLeftY = 0;
	//mScreenViewport.Width = static_cast<float>(mClientWidth);
	//mScreenViewport.Height = static_cast<float>(mClientHeight);
	//mScreenViewport.MinDepth = 0.0f;
	//mScreenViewport.MaxDepth = 1.0f;

	//mScissorRect = { 0, 0, mClientWidth, mClientHeight };


	//创建根描述符
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc =
	{
		0
		, nullptr
		, 0
		, nullptr
		, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));

	//编译Shader
	Microsoft::WRL::ComPtr<ID3D10Blob> pIBlobVertexShader;
	Microsoft::WRL::ComPtr<ID3D10Blob> pIBlobPixelShader;

	D3DCompileFromFile(L"F:\\GitHub\\Haze\\DirectX12Interface\\DirectX12Interface\\Shader\\shaderTest.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &pIBlobVertexShader, nullptr);
	D3DCompileFromFile(L"F:\\GitHub\\Haze\\DirectX12Interface\\DirectX12Interface\\Shader\\shaderTest.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pIBlobPixelShader, nullptr);

	//定义Vertex layout
	D3D12_INPUT_ELEMENT_DESC stInputElementDescs[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	//定义渲染管线状态描述结构，创建渲染管线对象
	D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
	stPSODesc.InputLayout = { stInputElementDescs, _countof(stInputElementDescs) };
	stPSODesc.pRootSignature = mRootSignature.Get();
	stPSODesc.VS.pShaderBytecode = pIBlobVertexShader->GetBufferPointer();
	stPSODesc.VS.BytecodeLength = pIBlobVertexShader->GetBufferSize();
	stPSODesc.PS.pShaderBytecode = pIBlobPixelShader->GetBufferPointer();
	stPSODesc.PS.BytecodeLength = pIBlobPixelShader->GetBufferSize();

	stPSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	stPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	stPSODesc.BlendState.AlphaToCoverageEnable = FALSE;
	stPSODesc.BlendState.IndependentBlendEnable = FALSE;
	stPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	stPSODesc.DepthStencilState.DepthEnable = FALSE;
	stPSODesc.DepthStencilState.StencilEnable = FALSE;

	stPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	stPSODesc.NumRenderTargets = 1;
	stPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	stPSODesc.SampleMask = UINT_MAX;
	stPSODesc.SampleDesc.Count = 1;

	md3dDevice->CreateGraphicsPipelineState(&stPSODesc, IID_PPV_ARGS(&mPipelineState));

	//创建顶点Buffer
	float fTrangleSize = 3.f;
	GRS_VERTEX stTriangleVertices[] =
	{
		{ { 0.0f, 0.25f * fTrangleSize, 0.0f ,1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f * fTrangleSize, -0.25f * fTrangleSize, 0.0f ,1.0f  }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f * fTrangleSize, -0.25f * fTrangleSize, 0.0f  ,1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT nVertexBufferSize = sizeof(stTriangleVertices);
	D3D12_HEAP_PROPERTIES stHeapProp = { D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC stResSesc = {};
	stResSesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	stResSesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	stResSesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	stResSesc.Format = DXGI_FORMAT_UNKNOWN;
	stResSesc.Width = nVertexBufferSize;
	stResSesc.Height = 1;
	stResSesc.DepthOrArraySize = 1;
	stResSesc.MipLevels = 1;
	stResSesc.SampleDesc.Count = 1;
	stResSesc.SampleDesc.Quality = 0;

	md3dDevice->CreateCommittedResource(&stHeapProp, D3D12_HEAP_FLAG_NONE, &stResSesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mVertexBuffer));

	UINT8* pVertexDataBegin = nullptr;
	D3D12_RANGE stReadRange = { 0, 0 };
	mVertexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, stTriangleVertices, sizeof(stTriangleVertices));
	mVertexBuffer->Unmap(0, nullptr);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(GRS_VERTEX);
	mVertexBufferView.SizeInBytes = nVertexBufferSize;

	//创建围栏
	md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	n64FenceValue = 1;

	// 创建一个Event同步对象，用于等待围栏事件通知
	hEventFence = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	// 填充资源屏障结构
	D3D12_RESOURCE_BARRIER stBeginResBarrier = {};
	stBeginResBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	stBeginResBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	stBeginResBarrier.Transition.pResource = mSwapChainBuffer[nFrameIndex].Get();
	stBeginResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	stBeginResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	stBeginResBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	D3D12_RESOURCE_BARRIER stEndResBarrier = {};
	stEndResBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	stEndResBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	stEndResBarrier.Transition.pResource = mSwapChainBuffer[nFrameIndex].Get();
	stEndResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	stEndResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	stEndResBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	D3D12_CPU_DESCRIPTOR_HANDLE stRTVHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	DWORD dwRet = 0;
	BOOL bExit = FALSE;

	SetEvent(hEventFence);
	
	MSG msg = { 0 };
	while (!bExit)
	{
		dwRet = ::MsgWaitForMultipleObjects(1, &hEventFence, FALSE, INFINITE, QS_ALLINPUT);
		switch (dwRet - WAIT_OBJECT_0)
		{
			case 0:
			{
				//获取新的后缓冲序号，因为Present真正完成时后缓冲的序号就更新了
				nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

				//命令分配器先Reset一下
				mDirectCmdListAlloc->Reset();
				
				//Reset命令列表，并重新指定命令分配器和PSO对象
				mCommandList->Reset(mDirectCmdListAlloc.Get(), mPipelineState.Get());

				//开始记录命令
				mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
				mCommandList->SetPipelineState(mPipelineState.Get());
				mCommandList->RSSetViewports(1, &stViewPort);
				mCommandList->RSSetScissorRects(1, &stScissorRect);

				// 通过资源屏障判定后缓冲已经切换完毕可以开始渲染了
				stBeginResBarrier.Transition.pResource = mSwapChainBuffer[nFrameIndex].Get();
				mCommandList->ResourceBarrier(1, &stBeginResBarrier);

				stRTVHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
				stRTVHandle.ptr += nFrameIndex * mRtvDescriptorSize;
				//设置渲染目标
				mCommandList->OMSetRenderTargets(1, &stRTVHandle, FALSE, nullptr);

				// 继续记录命令，并真正开始新一帧的渲染

				mCommandList->ClearRenderTargetView(stRTVHandle, faClearColor, 0, nullptr);
				mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);

				//Draw Call！！！
				mCommandList->DrawInstanced(3, 1, 0, 0);

				//又一个资源屏障，用于确定渲染已经结束可以提交画面去显示了
				stEndResBarrier.Transition.pResource = mSwapChainBuffer[nFrameIndex].Get();
				mCommandList->ResourceBarrier(1, &stEndResBarrier);
				//关闭命令列表，可以去执行了
				mCommandList->Close();

				//执行命令列表
				ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
				mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

				//提交画面
				pISwapChain3->Present(1, 0);

				//开始同步GPU与CPU的执行，先记录围栏标记值
				const UINT64 n64CurrentFenceValue = n64FenceValue;
				mCommandQueue->Signal(mFence.Get(), n64CurrentFenceValue);
				n64FenceValue++;
				mFence->SetEventOnCompletion(n64CurrentFenceValue, hEventFence);
			}

			break;
			case 1:
			{//处理消息
				while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (WM_QUIT != msg.message)
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
					else
					{
						bExit = TRUE;
					}
				}
			}
			break;
			case WAIT_TIMEOUT:
			{

			}
			break;
			default:
				break;
		}
	}

}
