// DirectX12Interface.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <unordered_map>


//#include "Common/d3dUtil.h"
//#include "Common/GameTimer.h"
//
//#pragma comment(lib,"d3dcompiler.lib")
//#pragma comment(lib, "D3D12.lib")
//#pragma comment(lib, "dxgi.lib")
//
#include "HazeDefine/HazeLibraryDefine.h"
//
//GameTimer mTimer;
//
//int mClientWidth = 720;
//int mClientHeight = 720;
//const int SwapChainBufferCount = 2;
//int mCurrBackBuffer = 0;
//HWND mhMainWnd = nullptr;
//
//D3D12_VIEWPORT mScreenViewport;
//D3D12_RECT mScissorRect;
//
//
//Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
//UINT64 mCurrentFence = 0;
//
//
//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
//
//Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
//Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
//
//UINT mRtvDescriptorSize = 0;
//UINT mDsvDescriptorSize = 0;
//UINT mCbvSrvUavDescriptorSize = 0;
//
//bool m4xMsaaState = false;
//UINT m4xMsaaQuality = 0;
//
//DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
//DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//
//LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	switch (msg)
//	{
//		// WM_ACTIVATE is sent when the window is activated or deactivated.
//		// We pause the game when the window is deactivated and unpause it
//		// when it becomes active.
//	case WM_ACTIVATE:
//		/*if (LOWORD(wParam) == WA_INACTIVE)
//		{
//			mAppPaused = true;
//			mTimer.Stop();
//		}
//		else
//		{
//			mAppPaused = false;
//			mTimer.Start();
//		}*/
//		return 0;
//
//		// WM_SIZE is sent when the user resizes the window.
//	case WM_SIZE:
//		// Save the new client area dimensions.
//		//mClientWidth = LOWORD(lParam);
//		//mClientHeight = HIWORD(lParam);
//		//if (md3dDevice)
//		//{
//		//	if (wParam == SIZE_MINIMIZED)
//		//	{
//		//		mAppPaused = true;
//		//		mMinimized = true;
//		//		mMaximized = false;
//		//	}
//		//	else if (wParam == SIZE_MAXIMIZED)
//		//	{
//		//		mAppPaused = false;
//		//		mMinimized = false;
//		//		mMaximized = true;
//		//		OnResize();
//		//	}
//		//	else if (wParam == SIZE_RESTORED)
//		//	{
//		//		// Restoring from minimized state?
//		//		if (mMinimized)
//		//		{
//		//			mAppPaused = false;
//		//			mMinimized = false;
//		//			OnResize();
//		//		}
//
//		//		// Restoring from maximized state?
//		//		else if (mMaximized)
//		//		{
//		//			mAppPaused = false;
//		//			mMaximized = false;
//		//			OnResize();
//		//		}
//		//		else if (mResizing)
//		//		{
//		//			// If user is dragging the resize bars, we do not resize
//		//			// the buffers here because as the user continuously
//		//			// drags the resize bars, a stream of WM_SIZE messages are
//		//			// sent to the window, and it would be pointless (and slow)
//		//			// to resize for each WM_SIZE message received from dragging
//		//			// the resize bars.  So instead, we reset after the user is
//		//			// done resizing the window and releases the resize bars, which
//		//			// sends a WM_EXITSIZEMOVE message.
//		//		}
//		//		else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
//		//		{
//		//			OnResize();
//		//		}
//		//	}
//		//}
//		return 0;
//
//		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
//	case WM_ENTERSIZEMOVE:
//		/*mAppPaused = true;
//		mResizing = true;
//		mTimer.Stop();*/
//		return 0;
//
//		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
//		// Here we reset everything based on the new window dimensions.
//	case WM_EXITSIZEMOVE:
//		/*mAppPaused = false;
//		mResizing = false;
//		mTimer.Start();
//		OnResize();*/
//		return 0;
//
//		// WM_DESTROY is sent when the window is being destroyed.
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		return 0;
//
//		// The WM_MENUCHAR message is sent when a menu is active and the user presses
//		// a key that does not correspond to any mnemonic or accelerator key.
//	case WM_MENUCHAR:
//		// Don't beep when we alt-enter.
//		return MAKELRESULT(0, MNC_CLOSE);
//
//		// Catch this message so to prevent the window from becoming too small.
//	case WM_GETMINMAXINFO:
//		/*((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
//		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;*/
//		return 0;
//
//	case WM_LBUTTONDOWN:
//	case WM_MBUTTONDOWN:
//	case WM_RBUTTONDOWN:
//		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//		return 0;
//	case WM_LBUTTONUP:
//	case WM_MBUTTONUP:
//	case WM_RBUTTONUP:
//		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//		return 0;
//	case WM_MOUSEMOVE:
//		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//		return 0;
//	case WM_KEYUP:
//		/*if (wParam == VK_ESCAPE)
//		{
//			PostQuitMessage(0);
//		}
//		else if ((int)wParam == VK_F2)
//			Set4xMsaaState(!m4xMsaaState);*/
//		return 0;
//	case WM_KEYDOWN:
//		//OnKeyDown(wParam);
//		return 0;
//	}
//
//	return DefWindowProc(hwnd, msg, wParam, lParam);
//}
//
//LRESULT CALLBACK
//MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
//	// before CreateWindow returns, and thus before mhMainWnd is valid.
//	return MsgProc(hwnd, msg, wParam, lParam);
//}
//
//void CreateWindowsWindow(HAZE_CALL_FUNC_PARAM)
//{
//	HINSTANCE hInstance = ::GetModuleHandle(NULL);
//
//	WNDCLASS wc;
//	wc.style = CS_HREDRAW | CS_VREDRAW;
//	wc.lpfnWndProc = MainWndProc;
//	wc.cbClsExtra = 0;
//	wc.cbWndExtra = 0;
//	wc.hInstance = hInstance;
//	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
//	wc.hCursor = LoadCursor(0, IDC_ARROW);
//	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
//	wc.lpszMenuName = 0;
//	wc.lpszClassName = L"MainWnd";
//
//	if (!RegisterClass(&wc))
//	{
//		MessageBox(0, L"RegisterClass Failed.", 0, 0);
//		return;
//	}
//
//	// Compute window rectangle dimensions based on requested client area dimensions.
//	RECT R = { 0, 0, mClientWidth, mClientHeight };
//	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
//	int width = R.right - R.left;
//	int height = R.bottom - R.top;
//
//	mhMainWnd = CreateWindow(L"MainWnd", L"Haze窗口", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
//	if (!mhMainWnd)
//	{
//		MessageBox(0, L"CreateWindow Failed.", 0, 0);
//		return;
//	}
//
//	ShowWindow(mhMainWnd, SW_SHOW);
//	UpdateWindow(mhMainWnd);
//
//	return;
//}
//
//void CreateCommandObjects()
//{
//	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
//	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//
//	ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
//
//	ThrowIfFailed(md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));
//
//	ThrowIfFailed(md3dDevice->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT, mDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())));
//
//	mCommandList->Close();
//}
//
//void CreateSwapChain()
//{
//	// Release the previous swapchain we will be recreating.
//	mSwapChain.Reset();
//
//	DXGI_SWAP_CHAIN_DESC sd;
//	sd.BufferDesc.Width = mClientWidth;
//	sd.BufferDesc.Height = mClientHeight;
//	sd.BufferDesc.RefreshRate.Numerator = 60;
//	sd.BufferDesc.RefreshRate.Denominator = 1;
//	sd.BufferDesc.Format = mBackBufferFormat;
//	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
//	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
//	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
//	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	sd.BufferCount = SwapChainBufferCount;
//	sd.OutputWindow = mhMainWnd;
//	sd.Windowed = true;
//	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
//
//	// Note: Swap chain uses queue to perform flush.
//	ThrowIfFailed(mdxgiFactory->CreateSwapChain(mCommandQueue.Get(), &sd, mSwapChain.GetAddressOf()));
//}
//
//void CreateRtvAndDsvDescriptorHeaps()
//{
//	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
//	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
//	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvHeapDesc.NodeMask = 0;
//	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));
//
//	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
//	dsvHeapDesc.NumDescriptors = 1;
//	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
//	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	dsvHeapDesc.NodeMask = 0;
//	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
//}
//
//void FlushCommandQueue()
//{
//	// Advance the fence value to mark commands up to this fence point.
//	mCurrentFence++;
//
//	// Add an instruction to the command queue to set a new fence point.  Because we
//	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
//	// processing all the commands prior to this Signal().
//	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));
//
//	// Wait until the GPU has completed commands up to this fence point.
//	if (mFence->GetCompletedValue() < mCurrentFence)
//	{
//		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
//
//		// Fire event when GPU hits current fence.
//		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));
//
//		// Wait until the GPU hits current fence event is fired.
//		WaitForSingleObject(eventHandle, INFINITE);
//		CloseHandle(eventHandle);
//	}
//}
//
//void OnResize()
//{
//	assert(md3dDevice);
//	assert(mSwapChain);
//	assert(mDirectCmdListAlloc);
//
//	// Flush before changing any resources.
//	FlushCommandQueue();
//
//	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
//
//	// Release the previous resources we will be recreating.
//	for (int i = 0; i < SwapChainBufferCount; ++i)
//		mSwapChainBuffer[i].Reset();
//	mDepthStencilBuffer.Reset();
//
//	// Resize the swap chain.
//	ThrowIfFailed(mSwapChain->ResizeBuffers(
//		SwapChainBufferCount,
//		mClientWidth, mClientHeight,
//		mBackBufferFormat,
//		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
//
//	mCurrBackBuffer = 0;
//
//	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
//	for (UINT i = 0; i < SwapChainBufferCount; i++)
//	{
//		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
//		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
//		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
//	}
//
//	// Create the depth/stencil buffer and view.
//	D3D12_RESOURCE_DESC depthStencilDesc;
//	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	depthStencilDesc.Alignment = 0;
//	depthStencilDesc.Width = mClientWidth;
//	depthStencilDesc.Height = mClientHeight;
//	depthStencilDesc.DepthOrArraySize = 1;
//	depthStencilDesc.MipLevels = 1;
//
//	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from
//	// the depth buffer.  Therefore, because we need to create two views to the same resource:
//	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
//	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
//	// we need to create the depth buffer resource with a typeless format.
//	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
//
//	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
//	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
//	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//
//	D3D12_CLEAR_VALUE optClear;
//	optClear.Format = mDepthStencilFormat;
//	optClear.DepthStencil.Depth = 1.0f;
//	optClear.DepthStencil.Stencil = 0;
//	ThrowIfFailed(md3dDevice->CreateCommittedResource(
//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//		D3D12_HEAP_FLAG_NONE,
//		&depthStencilDesc,
//		D3D12_RESOURCE_STATE_COMMON,
//		&optClear,
//		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));
//
//	// Create descriptor to mip level 0 of entire resource using the format of the resource.
//	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
//	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
//	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
//	dsvDesc.Format = mDepthStencilFormat;
//	dsvDesc.Texture2D.MipSlice = 0;
//	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, mDsvHeap->GetCPUDescriptorHandleForHeapStart());
//
//	// Transition the resource from its initial state to be used as a depth buffer.
//	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
//		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
//
//	// Execute the resize commands.
//	ThrowIfFailed(mCommandList->Close());
//	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
//	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//
//	// Wait until resize is complete.
//	FlushCommandQueue();
//
//	// Update the viewport transform to cover the client area.
//	mScreenViewport.TopLeftX = 0;
//	mScreenViewport.TopLeftY = 0;
//	mScreenViewport.Width = static_cast<float>(mClientWidth);
//	mScreenViewport.Height = static_cast<float>(mClientHeight);
//	mScreenViewport.MinDepth = 0.0f;
//	mScreenViewport.MaxDepth = 1.0f;
//
//	mScissorRect = { 0, 0, mClientWidth, mClientHeight };
//}
//
//void InitDirect3D(HAZE_CALL_FUNC_PARAM)
//{
//	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));
//
//	// Try to create hardware device.
//	HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice));
//	if (FAILED(hardwareResult))
//	{
//		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
//		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
//		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&md3dDevice)));
//	}
//
//	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
//
//	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
//	msQualityLevels.Format = mBackBufferFormat;
//	msQualityLevels.SampleCount = 4;
//	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
//	msQualityLevels.NumQualityLevels = 0;
//
//	ThrowIfFailed(md3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
//
//	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
//	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
//
//	CreateCommandObjects();
//	CreateSwapChain();
//	CreateRtvAndDsvDescriptorHeaps();
//
//	OnResize();
//	mTimer.Reset();
//}
//
//bool mAppPaused = false;
//
//void CalculateFrameStats()
//{
//	// Code computes the average frames per second, and also the
//	// average time it takes to render one frame.  These stats
//	// are appended to the window caption bar.
//
//	static int frameCnt = 0;
//	static float timeElapsed = 0.0f;
//
//	frameCnt++;
//
//	// Compute averages over one second period.
//	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f)
//	{
//		float fps = (float)frameCnt; // fps = frameCnt / 1
//		float mspf = 1000.0f / fps;
//
//		std::wstring fpsStr = std::to_wstring(fps);
//		std::wstring mspfStr = std::to_wstring(mspf);
//
//		std::wstring windowText = std::wstring(L"Haze窗口") +
//			L"    fps: " + fpsStr +
//			L"   mspf: " + mspfStr;
//
//		SetWindowText(mhMainWnd, windowText.c_str());
//
//		// Reset for next average.
//		frameCnt = 0;
//		timeElapsed += 1.0f;
//	}
//}
//
//void OnKeyboardInput(const GameTimer& gt)
//{
//	const float dt = gt.DeltaTime();
//}
//
//void Update(const GameTimer& gt)
//{
//	//OnKeyboardInput(gt);
//	//UpdateCamera(gt);
//
//	//// Cycle through the circular frame resource array.
//	//mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
//	//mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
//
//	//// Has the GPU finished processing the commands of the current frame resource?
//	//// If not, wait until the GPU has completed commands up to this fence point.
//	//if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
//	//{
//	//	HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
//	//	ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
//	//	WaitForSingleObject(eventHandle, INFINITE);
//	//	CloseHandle(eventHandle);
//	//}
//
//	//AnimateMaterials(gt);
//	//UpdateObjectCBs(gt);
//	//UpdateMaterialCBs(gt);
//	//UpdateMainPassCB(gt);
//}
//
//void Draw(const GameTimer& gt)
//{
//	//auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
//
//	//// Reuse the memory associated with command recording.
//	//// We can only reset when the associated command lists have finished execution on the GPU.
//	//ThrowIfFailed(cmdListAlloc->Reset());
//
//	//// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
//	//// Reusing the command list reuses memory.
//	//ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
//
//	//mCommandList->RSSetViewports(1, &mScreenViewport);
//	//mCommandList->RSSetScissorRects(1, &mScissorRect);
//
//	//// Indicate a state transition on the resource usage.
//	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//	//	D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//	//// Clear the back buffer and depth buffer.
//	//mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
//	//mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
//
//	//// Specify the buffers we are going to render to.
//	//mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
//
//	//ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
//	//mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
//
//	//mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
//
//	//auto passCB = mCurrFrameResource->PassCB->Resource();
//	//mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
//
//	//DrawGameObjects(mCommandList.Get(), 2, mAllGameObjects.size());
//	//mCommandList->SetPipelineState(mPSOs["transparent"].Get());
//	//DrawGameObjects(mCommandList.Get(), 0, 2/*, mOpaqueRitems*/);
//
//	//// Indicate a state transition on the resource usage.
//	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
//	//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
//
//	//// Done recording commands.
//	//ThrowIfFailed(mCommandList->Close());
//
//	//// Add the command list to the queue for execution.
//	//ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
//	//mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
//
//	//// Swap the back and front buffers
//	//ThrowIfFailed(mSwapChain->Present(0, 0));
//	//mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
//
//	//// Advance the fence value to mark commands up to this fence point.
//	//mCurrFrameResource->Fence = ++mCurrentFence;
//
//	//// Add an instruction to the command queue to set a new fence point.
//	//// Because we are on the GPU timeline, the new fence point won't be
//	//// set until the GPU finishes processing all the commands prior to this Signal().
//	//mCommandQueue->Signal(mFence.Get(), mCurrentFence);
//}
//
//MSG g_Msg = { 0 };
//void Run()
//{
//	while (g_Msg.message != WM_QUIT)
//	{
//		// If there are Window messages then process them.
//		if (PeekMessage(&g_Msg, 0, 0, 0, PM_REMOVE))
//		{
//			TranslateMessage(&g_Msg);
//			DispatchMessage(&g_Msg);
//		}
//		// Otherwise, do animation/game stuff.
//		else
//		{
//			mTimer.Tick();
//
//			if (!mAppPaused)
//			{
//				CalculateFrameStats();
//				Update(mTimer);
//				Draw(mTimer);
//			}
//			else
//			{
//				Sleep(100);
//			}
//		}
//	}
//}
//
//void CallHaze(HAZE_CALL_FUNC_PARAM)
//{
//	void* Call;
//	void* StackPointer = nullptr;
//	int a;
//	int b;
//	//int c;
//	
//	GET_PARAM_START();
//	GET_PARAM(Call, ParamStartAddress);
//	GET_PARAM(a, ParamStartAddress);
//	GET_PARAM(b, ParamStartAddress);
//
//	ExeHazeFunction(Stack, Call, SET_HAZE_CALL_PARAM(a, b));
//
//	SET_HAZE_RET_TO_RET();
//}

extern void CreateWindowsWindow(HAZE_CALL_FUNC_PARAM);
extern void InitDX12(HAZE_CALL_FUNC_PARAM);

int main()
{
	CreateWindowsWindow(HAZE_CALL_NULL_PARAM);
	InitDX12(HAZE_CALL_NULL_PARAM);
	/*CreateWindowsWindow(HAZE_CALL_NULL_PARAM);
	InitDirect3D(HAZE_CALL_NULL_PARAM);
	Run();*/
	system("pause");
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
