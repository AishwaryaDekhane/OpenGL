#include <Windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning(disable : 4838)
#include "XNAMath/xnamath.h"
#include "Sphere.h"

#pragma comment (lib, "user32.lib")
#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "kernel32.lib")
#pragma comment (lib, "Sphere.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3dcompiler.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600
#define X (GetSystemMetrics(SM_CXSCREEN) - WIN_WIDTH)/2
#define Y (GetSystemMetrics(SM_CYSCREEN) - WIN_HEIGHT)/2

//global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variable declaration
FILE* gpFile = NULL;
char gszLogFileName[] = "Log.txt";

HWND ghwnd;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullScreen = false;

float gClearColor[4];	//RGBA
IDXGISwapChain* gpIDXGISwapChain = NULL;
ID3D11Device* gpID3D11Device = NULL;
ID3D11DeviceContext* gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView* gpID3D11RenderTargetView = NULL;
ID3D11DepthStencilView* gpID3D11DepthStencilView = NULL;
ID3D11VertexShader* gpID3D11VertexShader = NULL;
ID3D11PixelShader* gpID3D11PixelShader = NULL;

ID3D11Buffer* gpID3D11Buffer_VertexBuffer_Position = NULL;
ID3D11Buffer* gpID3D11Buffer_VertexBuffer_Color = NULL;
ID3D11Buffer* gpID3D11Buffer_VertexBuffer_Normal = NULL;
ID3D11Buffer* gpID3D11Buffer_VertexBuffer_Texture = NULL;
ID3D11Buffer* gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11InputLayout* gpID3D11InputLayout = NULL;
ID3D11RasterizerState* gpID3D11RasterizerState = NULL;
ID3D11Buffer* gpID3D11Buffer_IndexBuffer = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR Ld;
	XMVECTOR Kd;
	XMVECTOR LightPosition;
	unsigned int LKeyPressed;
};

XMMATRIX gPerspectiveProjectionMatrix;
float rotation_angle = 0.0f;
BOOL gbLight = FALSE;

//sphere variables
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements;
unsigned int gNumVertices;

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//Variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("Direct3D11");
	bool bDone = false;
	int iRet = 0;

	//function declaration
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);
	//code
	//create log file
	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Cannot Be Created\nExitting..."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "Log File is Created Successfully\n");
		fclose(gpFile);
	}
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;

	//Register Class
	RegisterClassEx(&wndclass);

	//CreateWindow
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Direct3D11 Diffuse Light On Sphere"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		X,
		Y,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//initialise D3D
	HRESULT hr;
	hr = initialize();
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "d3dInitialize() is failed. Exitting Now...\n");
		fclose(gpFile);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "d3dInitialize() is Succeeded.\n");
		fclose(gpFile);
	}

	//Game Loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		else
		{
			display();
			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
				{
					bDone = true;
				}
				update();
			}

		}
	}

	//clean-up
	uninitialize();

	return((int)msg.wParam);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function declaration
	void ToggleFullScreen(void);
	HRESULT resize(int, int);
	void uninitialize(void);

	//varible declaration
	HRESULT hr;

	//code
	switch (iMsg)
	{

	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (gbEscapeKeyIsPressed == false)
				gbEscapeKeyIsPressed = true;

			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;

		case 'L':
		case 'l':
			if (gbLight == FALSE)
			{
				gbLight = TRUE;
			}
			else
			{
				gbLight = FALSE;
			}
			break;
		}
		break;


	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "resize() is Succeeded.\n");
				fclose(gpFile);
				return(hr);
			}
			else
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "resize() is Succeeded.\n");
				fclose(gpFile);
			}
		}
		break;

	case WM_ERASEBKGND:
		return(0);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}
void ToggleFullScreen(void)
{
	//Variable declaration
	MONITORINFO mi;

	//code
	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		gbFullScreen = TRUE;
	}

	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}
HRESULT initialize(void)
{
	//function declaration
	HRESULT resize(int, int);
	void uninitialize(void);

	//variable declaration
	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE
	};

	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;

	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;

	//code
	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,									//	ADAPTER
			d3dDriverType,							//	DRIVER TYPE
			NULL,									//	SOFTWARE
			createDeviceFlags,						//	FLAGS
			&d3dFeatureLevel_required,				//	FEATURE LEVELS
			numFeatureLevels,						//	NUM FEATURE LEVELS
			D3D11_SDK_VERSION,						//	SDK VERSION
			&dxgiSwapChainDesc,						//	SWAP CHAIN DESC
			&gpIDXGISwapChain,						//	SWAP CHAIN
			&gpID3D11Device,						//	DEVICE
			&d3dFeatureLevel_acquired,				//	FEATURE LEVEL
			&gpID3D11DeviceContext);				//	DEVICE CONTEXT
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() is Succeeded.\n");
		fprintf_s(gpFile, "The Choosen Driver Is Of");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, "Hardware Type.\n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, "Warp Type.\n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, "Reference Type.\n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown Type.\n");
		}

		fprintf_s(gpFile, "The Supported Highest Feature Level Is");
		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10.1\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10.0\n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown.\n");
		}
		fclose(gpFile);
	}

	//initialise shader, input layouts, constant buffers
	const char* vertexShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldViewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 ld;" \
		"float4 kd;" \
		"float4 lightPosition;" \
		"uint keyPressed;" \
		"}" \
		"struct vertex_output" \
		"{" \
		"float4 position : SV_POSITION;" \
		"float4 diffuse_light : COLOR;" \
		"};" \
		"vertex_output main(float4 pos:POSITION,float4 normal:NORMAL)" \
		"{" \
		"vertex_output output;" \
		"if(keyPressed == 1)" \
		"{" \
		"float4 eye_coordinates = mul(worldViewMatrix, pos);" \
		"float3 transformed_normal = normalize(mul((float3x3)worldViewMatrix, (float3)normal));" \
		"float3 light_direction = (float3)normalize(lightPosition - eye_coordinates);" \
		"output.diffuse_light = ld * kd * max(dot(light_direction, transformed_normal), 0.0);" \
		"}" \
		"else"\
		"{" \
		"output.diffuse_light = float4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"output.position = mul(projectionMatrix, mul(worldViewMatrix, pos));" \
		"return(output);" \
		"}";

	ID3DBlob* pID3DBlob_VertexShaderSourceCode = NULL;
	ID3DBlob* pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode),
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderSourceCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() is Failed For Vertex Shader :%s \n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() is Succeeded For Vertex Shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderSourceCode->GetBufferPointer(),		
		pID3DBlob_VertexShaderSourceCode->GetBufferSize(),													
		NULL,																								
		&gpID3D11VertexShader);																				

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateVertexShader() is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateVertexShader() is Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);

	/* Pixel Shader */
	const char* pixelShaderSourceCode =
		"float4 main(float4 pos:SV_POSITION, float4 diffuse_light_color:COLOR) : SV_TARGET" \
		"{" \
		"float4 color = diffuse_light_color;" \
		"return(color);" \
		"}";

	ID3DBlob* pID3DBlob_PixelShader = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode),
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShader,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() is Failed For Pixel Shader :%s \n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() is Succeeded For Pixel Shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShader->GetBufferPointer(),		
		pID3DBlob_PixelShader->GetBufferSize(),													
		NULL,																					
		&gpID3D11PixelShader);																	

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateVertexShader() is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateVertexShader() is Succeeded.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);

	//sphere
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];

	//Position

	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	// Normals 

	inputElementDesc[1].SemanticName = "NORMAL";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderSourceCode->GetBufferPointer(),
		pID3DBlob_VertexShaderSourceCode->GetBufferSize(),
		&gpID3D11InputLayout);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateInputLayout() is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateInputLayout() is Succeeded.\n");
		fclose(gpFile);
	}
	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);

	//release vertex shader
	pID3DBlob_VertexShaderSourceCode->Release();
	pID3DBlob_VertexShaderSourceCode = NULL;

	//release pixel shader
	pID3DBlob_PixelShader->Release();
	pID3DBlob_PixelShader = NULL;

	//position

	//create vertex buffer
	D3D11_BUFFER_DESC bufferDescPosition;
	ZeroMemory(&bufferDescPosition, sizeof(D3D11_BUFFER_DESC));
	bufferDescPosition.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescPosition.ByteWidth = sizeof(float) * _ARRAYSIZE(sphere_vertices);
	bufferDescPosition.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescPosition.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDescPosition, NULL,
		&gpID3D11Buffer_VertexBuffer_Position);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Succeeded.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresourcePosition;
	ZeroMemory(&mappedSubresourcePosition, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Position,
		NULL,
		D3D11_MAP_WRITE_DISCARD,
		NULL,
		&mappedSubresourcePosition);
	memcpy(mappedSubresourcePosition.pData, sphere_vertices, sizeof(sphere_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Position, NULL);

	//normal

	//create vertex buffer
	D3D11_BUFFER_DESC bufferDescNormal;
	ZeroMemory(&bufferDescNormal, sizeof(D3D11_BUFFER_DESC));
	bufferDescNormal.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescNormal.ByteWidth = sizeof(float) * _ARRAYSIZE(sphere_normals);
	bufferDescNormal.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescNormal.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDescNormal, NULL,
		&gpID3D11Buffer_VertexBuffer_Normal);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Succeeded.\n");
		fclose(gpFile);
	}

	//copy vertices into above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresourceNormal;
	ZeroMemory(&mappedSubresourceNormal, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Normal,
		NULL,
		D3D11_MAP_WRITE_DISCARD,
		NULL,
		&mappedSubresourceNormal);
	memcpy(mappedSubresourceNormal.pData, sphere_normals, sizeof(sphere_normals));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Normal, NULL);

	//index buffer

	// create index buffer
	D3D11_BUFFER_DESC bufferDescIndices;
	ZeroMemory(&bufferDescIndices, sizeof(D3D11_BUFFER_DESC));
	bufferDescIndices.Usage = D3D11_USAGE_DYNAMIC; // write access access by CPU and GPU
	bufferDescIndices.ByteWidth = gNumElements * sizeof(short);
	bufferDescIndices.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescIndices.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU to write into this buffer
	hr = gpID3D11Device->CreateBuffer(&bufferDescIndices, NULL, &gpID3D11Buffer_IndexBuffer);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Failed for index buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Succeeded for index buffer.\n");
		fclose(gpFile);
	}

	// copy indices into above index buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresourceIndices;
	ZeroMemory(&mappedSubresourceIndices, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresourceIndices); // map buffer
	memcpy(mappedSubresourceIndices.pData, sphere_elements, gNumElements * sizeof(short));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer, NULL); // unmap buffer

																	//define constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr,
		&gpID3D11Buffer_ConstantBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Failed for constant buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() is Succeeded for constant buffer.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void*)&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	hr = gpID3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RasterizerState);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateRasterizerState() is Failed for constant buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateRasterizerState() is Succeeded for constant buffer.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	//d3d clear color
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 1.0f;

	//identity projection matrix
	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	//call resize for first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() is Succeeded.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() is Succeeded.\n");
		fclose(gpFile);
	}

	return(S_OK);
}
HRESULT resize(int width, int height)
{
	//code
	HRESULT hr = S_OK;

	//free any size-dependant resource
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	//resize swap chain buffers accordingly
	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//again get back buffer from swap chain
	ID3D11Texture2D* pID3D11Texture2D_BackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2D_BackBuffer);

	//again get render target view from d3d11 device using above back buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateRenderTargetView is Failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateRenderTargetView is Succeeded.\n");
		fclose(gpFile);
	}
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	//create depth stencil buffer
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;	//in real world this can be upto 4.
	textureDesc.SampleDesc.Quality = 0;	//if above is 4 then it is 1.
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;	//openGL's 24 bit depth In D3D It Is 32 bit.
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* pID3D11Texture2D_DepthBuffer;
	gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	//create depth stencil buffer from above depth stencil buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;		//MS For MultiSample
	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer,
		&depthStencilViewDesc,
		&gpID3D11DepthStencilView);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateDepthStencilView() is Failed for depth buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "gpID3D11Device::CreateDepthStencilView() is Succeeded for depth buffer.\n");
		fclose(gpFile);
	}

	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	//set render target view as render target
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	//set viewport
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	//set perspective matrix
	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), ((float)width / (float)height), 0.1f, 100.0f);

	return(hr);
}
void update(void)
{
	rotation_angle = rotation_angle + 0.005f;
	if (rotation_angle > 360.0f)
	{
		rotation_angle = 0.0f;
	}
}
void display(void)
{
	//code
	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	//clear depth stencil vies
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView,
		D3D11_CLEAR_DEPTH,
		1.0f,
		0);

	// Cube 
	//select which vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Position, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Normal, &stride, &offset);


	// set index buffer
	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, DXGI_FORMAT_R16_UINT, 0); // R16 maps with 'short'

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//translation corncerned with world matrix transformations
	XMMATRIX worldViewMatrix = XMMatrixIdentity();
	XMMATRIX projectionMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX wvpMatrix = XMMatrixIdentity();

	//transformation
	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);

	worldViewMatrix = translationMatrix;
	projectionMatrix = gPerspectiveProjectionMatrix;

	//load the data into constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewMatrix = worldViewMatrix;
	constantBuffer.ProjectionMatrix = projectionMatrix;
	if (gbLight == TRUE)
	{
		constantBuffer.LKeyPressed = 1;
		constantBuffer.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		constantBuffer.Kd = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		constantBuffer.LightPosition = XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f);
	}
	else
	{
		constantBuffer.LKeyPressed = 0;
	}

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	// draw sphere to render target
	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

	//switch between front and back
	gpIDXGISwapChain->Present(0, 0);
}
void uninitialize(void)
{
	//code
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Position)
	{
		gpID3D11Buffer_VertexBuffer_Position->Release();
		gpID3D11Buffer_VertexBuffer_Position = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Normal)
	{
		gpID3D11Buffer_VertexBuffer_Normal->Release();
		gpID3D11Buffer_VertexBuffer_Normal = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Texture)
	{
		gpID3D11Buffer_VertexBuffer_Texture->Release();
		gpID3D11Buffer_VertexBuffer_Texture = NULL;
	}

	if (gpID3D11Buffer_IndexBuffer)
	{
		gpID3D11Buffer_IndexBuffer->Release();
		gpID3D11Buffer_IndexBuffer = NULL;
	}

	if (gpID3D11RasterizerState)
	{
		gpID3D11RasterizerState->Release();
		gpID3D11RasterizerState = NULL;
	}

	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}

	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}

	if (gpFile)
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "uninitialize() is Succeeded.\n");
		fprintf_s(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
	}
}
