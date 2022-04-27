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
#define RADIUS 100.0f

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

float gClearColor[4];		//RGBA
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
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR La;
	XMVECTOR Ld;
	XMVECTOR Ls;
	XMVECTOR LightPosition;
	XMVECTOR Ka;
	XMVECTOR Kd;
	XMVECTOR Ks;
	float Material_Shininess;
	unsigned int LKeyPressed;
};

XMMATRIX gPerspectiveProjectionMatrix;
BOOL gbLight = FALSE;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements;
unsigned int gNumVertices;

float LightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 

float MaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess =  128.0f ; 

int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

struct material_array
{
	float MaterialAmbient[4];
	float MaterialDiffuse[4];
	float MaterialSpecular[4];
	float MaterialShininess;
};

material_array mArray[24];
int iCount = 0;
float angle_sphere = 0.0f;

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//Variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("Direct3D11");
	bool bDone = false;

	//function declaration
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);

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
		TEXT("Direct3D11 24 Spheres"),
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

		case 'X':		//rotate x
		case 'x':
			iCount = 1;
			break;

		case 'Y':		//rotate y
		case 'y':
			iCount = 2;
			break;

		case 'Z':		//rotate z
		case 'z':
			iCount = 3;
			break;
		}
		break;


	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			winWidth = LOWORD(lParam);
			winHeight = HIWORD(lParam);
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
	void DrawMaterial(void);

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
			"float4x4 worldMatrix;" \
			"float4x4 viewMatrix;" \
			"float4x4 projectionMatrix;" \
			"float4 la;" \
			"float4 ld;" \
			"float4 ls;" \
			"float4 lightPosition;" \
			"float4 ka;" \
			"float4 kd;" \
			"float4 ks;" \
			"float material_shininess;" \
			"uint keyPressed;" \
		"}" \

		"struct vertex_output" \
		"{" \
			"float4 position : SV_POSITION;" \
			"float3 transformed_normal : NORMAL_0;" \
			"float3 viewer_vector : NORMAL_1;" \
			"float3 light_direction : NORMAL_2;" \
		"};" \

		"vertex_output main(float4 pos:POSITION,float4 normal:NORMAL)" \
		"{" \
		"vertex_output output;" \
		"if(keyPressed == 1)" \
		"{" \
			"float4 eye_coordinates = mul(worldMatrix, pos);" \
			"eye_coordinates = mul(viewMatrix, eye_coordinates);" \
			"output.transformed_normal = mul((float3x3)worldMatrix, (float3)normal);" \
			"output.light_direction = (float3)(lightPosition - eye_coordinates);" \
			"output.viewer_vector = -eye_coordinates.xyz;" \
		"}" \
		"float4 position = mul(worldMatrix, pos);" \
		"position = mul(viewMatrix, position);" \
		"position = mul(projectionMatrix, position);" \
		"output.position = position;" \
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

	// Pixel Shader 
	const char* pixelShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
			"float4x4 worldMatrix;" \
			"float4x4 viewMatrix;" \
			"float4x4 projectionMatrix;" \
			"float4 la;" \
			"float4 ld;" \
			"float4 ls;" \
			"float4 lightPosition;" \
			"float4 ka;" \
			"float4 kd;" \
			"float4 ks;" \
			"float material_shininess;" \
			"uint keyPressed;" \
		"}" \
		"struct vertex_output" \
		"{" \
			"float4 position : SV_POSITION;" \

			"float3 transformed_normal : NORMAL_0;" \
			"float3 viewer_vector : NORMAL_1;" \
			"float3 light_direction : NORMAL_2;" \
		"};" \

		"float4 main(float4 pos:SV_POSITION, vertex_output input) : SV_TARGET" \
		"{" \
			"float4 phong_ads_light;" \
			"if(keyPressed == 1)" \
			"{" \
				"float3 normalized_transformed_normal = normalize(input.transformed_normal);" \
				"float3 normalized_light_direction = normalize(input.light_direction);" \
				"float3 normalized_viewer_vector = normalize(input.viewer_vector);" \
				"float3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);" \
				"float tn_dot_ld = max(dot(normalized_transformed_normal, normalized_light_direction), 0.0);" \

				"float4 ambient = la * ka;" \
				"float4 diffuse = ld * kd * tn_dot_ld;" \
				"float4 specular = ls * ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), material_shininess);" \
				"phong_ads_light = ambient + diffuse + specular;" \
				
			"}" \
			"else"\
			"{" \
				"phong_ads_light = float4(1.0, 1.0, 1.0, 1.0);" \
			"}" \
			"return(phong_ads_light);" \
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
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer, NULL); 

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
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

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


	//init materials
	DrawMaterial();

	//d3d clear color
	gClearColor[0] = 0.3f;
	gClearColor[1] = 0.3f;
	gClearColor[2] = 0.3f;
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
	textureDesc.SampleDesc.Count = 1;	//can be upto 4.
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
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;		
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
	//code
	angle_sphere = angle_sphere - 0.002f;
	if (angle_sphere >= 360)
	{
		angle_sphere = 0.0f;
	}
}

void display(void)
{
	//function declaration
	void Draw24Spheres(void);

	//code
	//clear render target view to a chosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	//clear depth stencil vies
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView,D3D11_CLEAR_DEPTH,1.0f,0);

	//select which vertex buffer to display render target
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Position, &stride, &offset);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Normal, &stride, &offset);
	Draw24Spheres();

	//switch between front and back
	gpIDXGISwapChain->Present(0, 0);
}

void Draw24Spheres(void)
{
	// OpenGL Drawing
	//declaration of matrices
	//variable declaration
	int i = 0;

	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	//viewport variable
	D3D11_VIEWPORT d3dViewPort;

	//setting the viewport of the window
	for (i = 0; i < 24; i++)
	{
		x = (i % 6) * winWidth / 6;
		y = winHeight - (i / 6 + 1) * winHeight / 4;
		w = winWidth / 6;
		h = winHeight / 4;

		//set viewport
		d3dViewPort.TopLeftX = (float)x;
		d3dViewPort.TopLeftY = (float)y;
		d3dViewPort.Width = (float)w;
		d3dViewPort.Height = (float)h;
		d3dViewPort.MinDepth = 0.0f;
		d3dViewPort.MaxDepth = 1.0f;
		gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

		//set perspective matrix
		gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), ((float)(winWidth / 6) / (float)(winHeight / 4)), 0.1f, 100.0f);

		// set index buffer
		gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, DXGI_FORMAT_R16_UINT, 0); 

		//select primitive
		gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//translation corncerned with world co-ordinates
		XMMATRIX worldMatrix = XMMatrixIdentity();
		XMMATRIX viewMatrix = XMMatrixIdentity();
		XMMATRIX projectionMatrix = XMMatrixIdentity();
		XMMATRIX translationMatrix = XMMatrixIdentity();

		//transformation
		translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);

		worldMatrix = translationMatrix;
		projectionMatrix = gPerspectiveProjectionMatrix;

		//load the data into constant buffer
		CBUFFER constantBuffer;
		constantBuffer.WorldMatrix = worldMatrix;
		constantBuffer.ViewMatrix = viewMatrix;
		constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
		if (gbLight == TRUE)
		{
			constantBuffer.LKeyPressed = 1;
			constantBuffer.La = XMVectorSet(LightAmbient[0],LightAmbient[1],LightAmbient[2],LightAmbient[3]);
			constantBuffer.Ld = XMVectorSet(LightDiffuse[0],LightDiffuse[1],LightDiffuse[2],LightDiffuse[3]);
			constantBuffer.Ls = XMVectorSet(LightSpecular[0],LightSpecular[1],LightSpecular[2],LightSpecular[3]);
			//send light position
			if (iCount == 1)
			{
				LightPosition[0] = 0.0f;
				LightPosition[1] = cosf(angle_sphere) * RADIUS;
				LightPosition[2] = sinf(angle_sphere) * RADIUS;
				LightPosition[3] = 1.0f;
			}

			if (iCount == 2)
			{
				LightPosition[0] = cosf(angle_sphere) * RADIUS;
				LightPosition[1] = 0.0f;
				LightPosition[2] = sinf(angle_sphere) * RADIUS;
				LightPosition[3] = 1.0f;
			}

			if (iCount == 3)
			{
				LightPosition[0] = cosf(angle_sphere) * RADIUS;
				LightPosition[1] = sinf(angle_sphere) * RADIUS;
				LightPosition[2] = 0.0f;
				LightPosition[3] = 1.0f;
			}

			constantBuffer.LightPosition = XMVectorSet(	LightPosition[0],LightPosition[1],LightPosition[2],LightPosition[3]);
			constantBuffer.Ka = XMVectorSet(mArray[i].MaterialAmbient[0],mArray[i].MaterialAmbient[1],mArray[i].MaterialAmbient[2],mArray[i].MaterialAmbient[3]);
			constantBuffer.Kd = XMVectorSet(mArray[i].MaterialDiffuse[0],mArray[i].MaterialDiffuse[1],mArray[i].MaterialDiffuse[2],mArray[i].MaterialDiffuse[3]);
			constantBuffer.Ks = XMVectorSet(mArray[i].MaterialSpecular[0],mArray[i].MaterialSpecular[1],mArray[i].MaterialSpecular[2],mArray[i].MaterialSpecular[3]);
			constantBuffer.Material_Shininess = mArray[i].MaterialShininess;
		}
		else
		{
			constantBuffer.LKeyPressed = 0;
		}
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);
	}
}

void DrawMaterial(void)
{
	//------------------------------------------------Black Rubber
	mArray[0].MaterialAmbient[0] = 0.02f;
	mArray[0].MaterialAmbient[1] = 0.02f;
	mArray[0].MaterialAmbient[2] = 0.02f;
	mArray[0].MaterialAmbient[3] = 1.0f;
	mArray[0].MaterialDiffuse[0] = 0.01f;
	mArray[0].MaterialDiffuse[1] = 0.01f;
	mArray[0].MaterialDiffuse[2] = 0.01f;
	mArray[0].MaterialDiffuse[3] = 1.0f;
	mArray[0].MaterialSpecular[0] = 0.4f;
	mArray[0].MaterialSpecular[1] = 0.4f;
	mArray[0].MaterialSpecular[2] = 0.4f;
	mArray[0].MaterialSpecular[3] = 1.0f;
	mArray[0].MaterialShininess = 0.078125f * 128.0f;
	
	//--------------------------------------------------Cyan Rubber
	mArray[1].MaterialAmbient[0] = 0.0f;
	mArray[1].MaterialAmbient[1] = 0.05f;
	mArray[1].MaterialAmbient[2] = 0.05f;
	mArray[1].MaterialAmbient[3] = 1.0f;
	mArray[1].MaterialDiffuse[0] = 0.4f;
	mArray[1].MaterialDiffuse[1] = 0.5f;
	mArray[1].MaterialDiffuse[2] = 0.5f;
	mArray[1].MaterialDiffuse[3] = 1.0f;
	mArray[1].MaterialSpecular[0] = 0.04f;
	mArray[1].MaterialSpecular[1] = 0.7f;
	mArray[1].MaterialSpecular[2] = 0.7f;
	mArray[1].MaterialSpecular[3] = 1.0f;
	mArray[1].MaterialShininess = 0.078125f * 128.0f;
	
	//--------------------------------------------Green Rubber
	mArray[2].MaterialAmbient[0] = 0.0f;
	mArray[2].MaterialAmbient[1] = 0.05f;
	mArray[2].MaterialAmbient[2] = 0.0f;
	mArray[2].MaterialAmbient[3] = 1.0f;
	mArray[2].MaterialDiffuse[0] = 0.4f;
	mArray[2].MaterialDiffuse[1] = 0.5f;
	mArray[2].MaterialDiffuse[2] = 0.4f;
	mArray[2].MaterialDiffuse[3] = 1.0f;
	mArray[2].MaterialSpecular[0] = 0.04f;
	mArray[2].MaterialSpecular[1] = 0.7f;
	mArray[2].MaterialSpecular[2] = 0.04f;
	mArray[2].MaterialSpecular[3] = 1.0f;
	mArray[2].MaterialShininess = 0.078125f * 128.0f;
	
	//-------------------------------------Red Rubber
	mArray[3].MaterialAmbient[0] = 0.05f;
	mArray[3].MaterialAmbient[1] = 0.0f;
	mArray[3].MaterialAmbient[2] = 0.0f;
	mArray[3].MaterialAmbient[3] = 1.0f;
	mArray[3].MaterialDiffuse[0] = 0.5f;
	mArray[3].MaterialDiffuse[1] = 0.4f;
	mArray[3].MaterialDiffuse[2] = 0.4f;
	mArray[3].MaterialDiffuse[3] = 1.0f;
	mArray[3].MaterialSpecular[0] = 0.7f;
	mArray[3].MaterialSpecular[1] = 0.04f;
	mArray[3].MaterialSpecular[2] = 0.04f;
	mArray[3].MaterialSpecular[3] = 1.0f;
	mArray[3].MaterialShininess = 0.078125f * 128.0f;
	
	//------------------------------------------White Rubber
	mArray[4].MaterialAmbient[0] = 0.05f;
	mArray[4].MaterialAmbient[1] = 0.05f;
	mArray[4].MaterialAmbient[2] = 0.05f;
	mArray[4].MaterialAmbient[3] = 1.0f;
	mArray[4].MaterialDiffuse[0] = 0.5f;
	mArray[4].MaterialDiffuse[1] = 0.5f;
	mArray[4].MaterialDiffuse[2] = 0.5f;
	mArray[4].MaterialDiffuse[3] = 1.0f;
	mArray[4].MaterialSpecular[0] = 0.7f;
	mArray[4].MaterialSpecular[1] = 0.7f;
	mArray[4].MaterialSpecular[2] = 0.7f;
	mArray[4].MaterialSpecular[3] = 1.0f;
	mArray[4].MaterialShininess = 0.078125f * 128.0f;
	
	//--------------------------------------------------Yellow Rubber
	mArray[5].MaterialAmbient[0] = 0.05f;
	mArray[5].MaterialAmbient[1] = 0.05f;
	mArray[5].MaterialAmbient[2] = 0.0f;
	mArray[5].MaterialAmbient[3] = 1.0f;
	mArray[5].MaterialDiffuse[0] = 0.5f;
	mArray[5].MaterialDiffuse[1] = 0.5f;
	mArray[5].MaterialDiffuse[2] = 0.4f;
	mArray[5].MaterialDiffuse[3] = 1.0f;
	mArray[5].MaterialSpecular[0] = 0.7f;
	mArray[5].MaterialSpecular[1] = 0.7f;
	mArray[5].MaterialSpecular[2] = 0.04f;
	mArray[5].MaterialSpecular[3] = 1.0f;
	mArray[5].MaterialShininess = 0.078125f * 128.0f;

	//--------------------------------------------------Black Plastic
	mArray[6].MaterialAmbient[0] = 0.0f;
	mArray[6].MaterialAmbient[1] = 0.0f;
	mArray[6].MaterialAmbient[2] = 0.0f;
	mArray[6].MaterialAmbient[3] = 1.0f;
	mArray[6].MaterialDiffuse[0] = 0.01f;
	mArray[6].MaterialDiffuse[1] = 0.01f;
	mArray[6].MaterialDiffuse[2] = 0.01f;
	mArray[6].MaterialDiffuse[3] = 1.0f;
	mArray[6].MaterialSpecular[0] = 0.50f;
	mArray[6].MaterialSpecular[1] = 0.50f;
	mArray[6].MaterialSpecular[2] = 0.50f;
	mArray[6].MaterialSpecular[3] = 1.0f;
	mArray[6].MaterialShininess = 0.25f * 128.0f;

	//---------------------------------------------------Cyan Plastic
	mArray[7].MaterialAmbient[0] = 0.0f;
	mArray[7].MaterialAmbient[1] = 0.1f;
	mArray[7].MaterialAmbient[2] = 0.06f;
	mArray[7].MaterialAmbient[3] = 1.0f;
	mArray[7].MaterialDiffuse[0] = 0.01f;
	mArray[7].MaterialDiffuse[1] = 0.50980392f;
	mArray[7].MaterialDiffuse[2] = 0.50980392f;
	mArray[7].MaterialDiffuse[3] = 1.0f;
	mArray[7].MaterialSpecular[0] = 0.50196078f;
	mArray[7].MaterialSpecular[1] = 0.50196078f;
	mArray[7].MaterialSpecular[2] = 0.50196078f;
	mArray[7].MaterialSpecular[3] = 1.0f;
	mArray[7].MaterialShininess = 0.25f * 128.0f;

	//------------------------------------------------Green Plastic
	mArray[8].MaterialAmbient[0] = 0.0f;
	mArray[8].MaterialAmbient[1] = 0.0f;
	mArray[8].MaterialAmbient[2] = 0.0f;
	mArray[8].MaterialAmbient[3] = 1.0f;
	mArray[8].MaterialDiffuse[0] = 0.1f;
	mArray[8].MaterialDiffuse[1] = 0.35f;
	mArray[8].MaterialDiffuse[2] = 0.1f;
	mArray[8].MaterialDiffuse[3] = 1.0f;
	mArray[8].MaterialSpecular[0] = 0.45f;
	mArray[8].MaterialSpecular[1] = 0.55f;
	mArray[8].MaterialSpecular[2] = 0.45f;
	mArray[8].MaterialSpecular[3] = 1.0f;
	mArray[8].MaterialShininess = 0.25f * 128.0f;	

	//-----------------------------------------------Red Plastic
	mArray[9].MaterialAmbient[0] = 0.0f;
	mArray[9].MaterialAmbient[1] = 0.0f;
	mArray[9].MaterialAmbient[2] = 0.0f;
	mArray[9].MaterialAmbient[3] = 1.0f;
	mArray[9].MaterialDiffuse[0] = 0.5f;
	mArray[9].MaterialDiffuse[1] = 0.0f;
	mArray[9].MaterialDiffuse[2] = 0.0f;
	mArray[9].MaterialDiffuse[3] = 1.0f;
	mArray[9].MaterialSpecular[0] = 0.7f;
	mArray[9].MaterialSpecular[1] = 0.6f;
	mArray[9].MaterialSpecular[2] = 0.6f;
	mArray[9].MaterialSpecular[3] = 1.0f;
	mArray[9].MaterialShininess = 0.25f * 128.0f;

	//------------------------------------------------White Plastic
	mArray[10].MaterialAmbient[0] = 0.0f;
	mArray[10].MaterialAmbient[1] = 0.0f;
	mArray[10].MaterialAmbient[2] = 0.0f;
	mArray[10].MaterialAmbient[3] = 1.0f;
	mArray[10].MaterialDiffuse[0] = 0.55f;
	mArray[10].MaterialDiffuse[1] = 0.55f;
	mArray[10].MaterialDiffuse[2] = 0.55f;
	mArray[10].MaterialDiffuse[3] = 1.0f;
	mArray[10].MaterialSpecular[0] = 0.70f;
	mArray[10].MaterialSpecular[1] = 0.70f;
	mArray[10].MaterialSpecular[2] = 0.70f;
	mArray[10].MaterialSpecular[3] = 1.0f;
	mArray[10].MaterialShininess = 0.25f * 128.0f;

	//------------------------------------------------yellow Plastic
	mArray[11].MaterialAmbient[0] = 0.0f;
	mArray[11].MaterialAmbient[1] = 0.0f;
	mArray[11].MaterialAmbient[2] = 0.0f;
	mArray[11].MaterialAmbient[3] = 1.0f;
	mArray[11].MaterialDiffuse[0] = 0.5f;
	mArray[11].MaterialDiffuse[1] = 0.5f;
	mArray[11].MaterialDiffuse[2] = 0.0f;
	mArray[11].MaterialDiffuse[3] = 1.0f;
	mArray[11].MaterialSpecular[0] = 0.60f;
	mArray[11].MaterialSpecular[1] = 0.60f;
	mArray[11].MaterialSpecular[2] = 0.50f;
	mArray[11].MaterialSpecular[3] = 1.0f;
	mArray[11].MaterialShininess = 0.25f * 128.0f;	

	//---------------------------------------------------------brass
	mArray[12].MaterialAmbient[0] = 0.329412f;
	mArray[12].MaterialAmbient[1] = 0.223529f;
	mArray[12].MaterialAmbient[2] = 0.027451f;
	mArray[12].MaterialAmbient[3] = 1.0f;
	mArray[12].MaterialDiffuse[0] = 0.782392f;
	mArray[12].MaterialDiffuse[1] = 0.568627f;
	mArray[12].MaterialDiffuse[2] = 0.113725f;
	mArray[12].MaterialDiffuse[3] = 1.0f;
	mArray[12].MaterialSpecular[0] = 0.992157f;
	mArray[12].MaterialSpecular[1] = 0.941176f;
	mArray[12].MaterialSpecular[2] = 0.807843f;
	mArray[12].MaterialSpecular[3] = 1.0f;
	mArray[12].MaterialShininess = 0.21794872f * 128.0f;

	//--------------------------------------------------bronze
	mArray[13].MaterialAmbient[0] = 0.2125f;
	mArray[13].MaterialAmbient[1] = 0.1275f;
	mArray[13].MaterialAmbient[2] = 0.054f;
	mArray[13].MaterialAmbient[3] = 1.0f;
	mArray[13].MaterialDiffuse[0] = 0.714f;
	mArray[13].MaterialDiffuse[1] = 0.4284f;
	mArray[13].MaterialDiffuse[2] = 0.18144f;
	mArray[13].MaterialDiffuse[3] = 1.0f;
	mArray[13].MaterialSpecular[0] = 0.393548f;
	mArray[13].MaterialSpecular[1] = 0.271906f;
	mArray[13].MaterialSpecular[2] = 0.166721f;
	mArray[13].MaterialSpecular[3] = 1.0f;
	mArray[13].MaterialShininess = 0.2f * 128.0f;

	//-------------------------------------------------chrome
	mArray[14].MaterialAmbient[0] = 0.25f;
	mArray[14].MaterialAmbient[1] = 0.25f;
	mArray[14].MaterialAmbient[2] = 0.25f;
	mArray[14].MaterialAmbient[3] = 1.0f;
	mArray[14].MaterialDiffuse[0] = 0.4f;
	mArray[14].MaterialDiffuse[1] = 0.4f;
	mArray[14].MaterialDiffuse[2] = 0.4f;
	mArray[14].MaterialDiffuse[3] = 1.0f;
	mArray[14].MaterialSpecular[0] = 0.774597f;
	mArray[14].MaterialSpecular[1] = 0.774597f;
	mArray[14].MaterialSpecular[2] = 0.774597f;
	mArray[14].MaterialSpecular[3] = 1.0f;
	mArray[14].MaterialShininess = 0.6f * 128.0f;

	//-----------------------------------------------------copper
	mArray[15].MaterialAmbient[0] = 0.19125f;
	mArray[15].MaterialAmbient[1] = 0.0735f;
	mArray[15].MaterialAmbient[2] = 0.0225f;
	mArray[15].MaterialAmbient[3] = 1.0f;
	mArray[15].MaterialDiffuse[0] = 0.7038f;
	mArray[15].MaterialDiffuse[1] = 0.27048f;
	mArray[15].MaterialDiffuse[2] = 0.0828f;
	mArray[15].MaterialDiffuse[3] = 1.0f;
	mArray[15].MaterialSpecular[0] = 0.256777f;
	mArray[15].MaterialSpecular[1] = 0.137622f;
	mArray[15].MaterialSpecular[2] = 0.086014f;
	mArray[15].MaterialSpecular[3] = 1.0f;
	mArray[15].MaterialShininess = 0.1f * 128.0f;

	//-------------------------------------------------------gold
	mArray[16].MaterialAmbient[0] = 0.24725f;
	mArray[16].MaterialAmbient[1] = 0.1995f;
	mArray[16].MaterialAmbient[2] = 0.0745f;
	mArray[16].MaterialAmbient[3] = 1.0f;
	mArray[16].MaterialDiffuse[0] = 0.75164f;
	mArray[16].MaterialDiffuse[1] = 0.60648f;
	mArray[16].MaterialDiffuse[2] = 0.22648f;
	mArray[16].MaterialDiffuse[3] = 1.0f;
	mArray[16].MaterialSpecular[0] = 0.628281f;
	mArray[16].MaterialSpecular[1] = 0.555802f;
	mArray[16].MaterialSpecular[2] = 0.366065f;
	mArray[16].MaterialSpecular[3] = 1.0f;
	mArray[16].MaterialShininess = 0.4f * 128.0f;

	//---------------------------------------------------silver
	mArray[17].MaterialAmbient[0] = 0.19225f;
	mArray[17].MaterialAmbient[1] = 0.19225f;
	mArray[17].MaterialAmbient[2] = 0.19225f;
	mArray[17].MaterialAmbient[3] = 1.0f;
	mArray[17].MaterialDiffuse[0] = 0.50754f;
	mArray[17].MaterialDiffuse[1] = 0.50754f;
	mArray[17].MaterialDiffuse[2] = 0.50754f;
	mArray[17].MaterialDiffuse[3] = 1.0f;
	mArray[17].MaterialSpecular[0] = 0.508273f;
	mArray[17].MaterialSpecular[1] = 0.508273f;
	mArray[17].MaterialSpecular[2] = 0.508273f;
	mArray[17].MaterialSpecular[3] = 1.0f;
	mArray[17].MaterialShininess = 0.4f * 128.0f;

	//---------------------------------------------emrald
	mArray[18].MaterialAmbient[0] = 0.0215f;
	mArray[18].MaterialAmbient[1] = 0.1745f;
	mArray[18].MaterialAmbient[2] = 0.0215f;
	mArray[18].MaterialAmbient[3] = 1.0f;
	mArray[18].MaterialDiffuse[0] = 0.07568f;
	mArray[18].MaterialDiffuse[1] = 0.61424f;
	mArray[18].MaterialDiffuse[2] = 0.07568f;
	mArray[18].MaterialDiffuse[3] = 1.0f;
	mArray[18].MaterialSpecular[0] = 0.633f;
	mArray[18].MaterialSpecular[1] = 0.727811f;
	mArray[18].MaterialSpecular[2] = 0.633f;
	mArray[18].MaterialSpecular[3] = 1.0f;
	mArray[18].MaterialShininess = 0.6f * 128.0f;

	//----------------------------------------------jade
	mArray[19].MaterialAmbient[0] = 0.135f;
	mArray[19].MaterialAmbient[1] = 0.2225f;
	mArray[19].MaterialAmbient[2] = 0.1575f;
	mArray[19].MaterialAmbient[3] = 1.0f;
	mArray[19].MaterialDiffuse[0] = 0.54f;
	mArray[19].MaterialDiffuse[1] = 0.89f;
	mArray[19].MaterialDiffuse[2] = 0.63f;
	mArray[19].MaterialDiffuse[3] = 1.0f;
	mArray[19].MaterialSpecular[0] = 0.316228f;
	mArray[19].MaterialSpecular[1] = 0.316228f;
	mArray[19].MaterialSpecular[2] = 0.316228f;
	mArray[19].MaterialSpecular[3] = 1.0f;
	mArray[19].MaterialShininess = 0.1f * 128.0f;

	//--------------------------------------------obsidian
	mArray[20].MaterialAmbient[0] = 0.05375f;
	mArray[20].MaterialAmbient[1] = 0.05f;
	mArray[20].MaterialAmbient[2] = 0.06625f;
	mArray[20].MaterialAmbient[3] = 1.0f;
	mArray[20].MaterialDiffuse[0] = 0.18275f;
	mArray[20].MaterialDiffuse[1] = 0.17f;
	mArray[20].MaterialDiffuse[2] = 0.22525f;
	mArray[20].MaterialDiffuse[3] = 1.0f;
	mArray[20].MaterialSpecular[0] = 0.332741f;
	mArray[20].MaterialSpecular[1] = 0.328634f;
	mArray[20].MaterialSpecular[2] = 0.346435f;
	mArray[20].MaterialSpecular[3] = 1.0f;
	mArray[20].MaterialShininess = 0.3f * 128.0f;

	//------------------------------------------------pearl
	mArray[21].MaterialAmbient[0] = 0.25f;
	mArray[21].MaterialAmbient[1] = 0.20725f;
	mArray[21].MaterialAmbient[2] = 0.20725f;
	mArray[21].MaterialAmbient[3] = 1.0f;
	mArray[21].MaterialDiffuse[0] = 1.0f;
	mArray[21].MaterialDiffuse[1] = 0.829f;
	mArray[21].MaterialDiffuse[2] = 0.829f;
	mArray[21].MaterialDiffuse[3] = 1.0f;
	mArray[21].MaterialSpecular[0] = 0.296648f;
	mArray[21].MaterialSpecular[1] = 0.296648f;
	mArray[21].MaterialSpecular[2] = 0.296648f;
	mArray[21].MaterialSpecular[3] = 1.0f;
	mArray[21].MaterialShininess = 0.088f * 128.0f;

	//-------------------------------------------------ruby
	mArray[22].MaterialAmbient[0] = 0.1745f;
	mArray[22].MaterialAmbient[1] = 0.01175f;
	mArray[22].MaterialAmbient[2] = 0.01175f;
	mArray[22].MaterialAmbient[3] = 1.0f;
	mArray[22].MaterialDiffuse[0] = 0.61424f;
	mArray[22].MaterialDiffuse[1] = 0.04136f;
	mArray[22].MaterialDiffuse[2] = 0.04136f;
	mArray[22].MaterialDiffuse[3] = 1.0f;
	mArray[22].MaterialSpecular[0] = 0.727811f;
	mArray[22].MaterialSpecular[1] = 0.626959f;
	mArray[22].MaterialSpecular[2] = 0.626959f;
	mArray[22].MaterialSpecular[3] = 1.0f;
	mArray[22].MaterialShininess = 0.6f * 128.0f;

	//----------------------------------------------Turquoise
	mArray[23].MaterialAmbient[0] = 0.1f;
	mArray[23].MaterialAmbient[1] = 0.18725f;
	mArray[23].MaterialAmbient[2] = 0.1745f;
	mArray[23].MaterialAmbient[3] = 1.0f;
	mArray[23].MaterialDiffuse[0] = 0.396f;
	mArray[23].MaterialDiffuse[1] = 0.74151f;
	mArray[23].MaterialDiffuse[2] = 0.69102f;
	mArray[23].MaterialDiffuse[3] = 1.0f;
	mArray[23].MaterialSpecular[0] = 0.297254f;
	mArray[23].MaterialSpecular[1] = 0.30829f;
	mArray[23].MaterialSpecular[2] = 0.306678f;
	mArray[23].MaterialSpecular[3] = 1.0f;
	mArray[23].MaterialShininess = 0.1f * 128.0f;
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
