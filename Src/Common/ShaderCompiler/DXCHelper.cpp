#include "Common/stdafx.h"
#include <d3dcompiler.h>
#include <dxcapi.h>
#include "DXCHelper.h"

#include "Common/Misc/Misc.h"
#include "Common/ShaderCompiler/ShaderCompiler.h"
#include "ShaderCompilerCache.h"
#include "Common/Misc/Error.h"


#define USE_DXC_SPIRV_FROM_DISK

DxcCreateInstanceProc s_dxc_create_func;

bool InitDirectXCompiler()
{
	std::string fullShaderCompilerPath = "dxcompiler.dll";
	std::string fullShaderDXILPath = "dxil.dll";

	HMODULE dxil_module = ::LoadLibrary(fullShaderDXILPath.c_str());
	HMODULE dxc_module = ::LoadLibrary(fullShaderCompilerPath.c_str());

	s_dxc_create_func = (DxcCreateInstanceProc)::GetProcAddress(dxc_module, "DxcCreateInstance");

	return s_dxc_create_func != NULL;
}

interface IncluderDxc : public IDxcIncludeHandler
{
	IDxcLibrary* m_pLibrary;

public:
	IncluderDxc(IDxcLibrary* pLibrary) : m_pLibrary(pLibrary) {}
	HRESULT QueryInterface(const IID&, void**) { return S_OK; }
	ULONG AddRef() { return 0; }
	ULONG Release() { return 0; }
	HRESULT LoadSource(LPCWSTR pFileName, IDxcBlob** ppIncludeSource)
	{
		char fullPath[1024];
		sprintf_s<1024>(fullPath, ("%s\\%S"), GetShaderCompilerLibDir().c_str(), pFileName);

		LPCVOID	pData;
		size_t	bytes;
		HRESULT hr = ReadFile(fullPath, (char**)&pData, (size_t*)&bytes, false) ? S_OK : E_FAIL;

		if (hr == E_FAIL)
		{
			// Return the faulre her instead of crashing on CreateBlobWithENcodingFromPinned
			// to be able to report the eerror to the output console.
			return hr;
		}

		IDxcBlobEncoding* pSource;
		ThrowIfFailed(m_pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)pData, (UINT32)bytes, CP_UTF8, &pSource));

		*ppIncludeSource = pSource;

		return S_OK;
	}
};

bool DXCompileToDXO(size_t hash, const char* pSrcCode,
	const DefineList* pDefines,
	const char* pEntryPoint,
	const char* pParams,
	char** outSpvData,
	size_t* outSpvSize)
{
	// Detect output bytecode type (DXBC/SPIR-V) nad use proper extension.
	std::string filenameOut;
	{
		auto found = std::string(pParams).find("-spirv ");
		if (found == std::string::npos)
			filenameOut = GetShaderCompilerCacheDir() + format("\\%p.dxo", hash);
		else
			filenameOut = GetShaderCompilerCacheDir() + format("\\%p.spv", hash);
	}

#ifdef USE_DXC_SPIRV_FROM_DISK
	if (ReadFile(filenameOut.c_str(), outSpvData, outSpvSize, true) && *outSpvSize > 0)
	{
		return true;
	}
#endif

	// Create hlsl file for shader compile to compile.
	//
	std::string filenameHlsl = GetShaderCompilerCacheDir() + format("\\%p.hlsl", hash);
	std::ofstream ofs( filenameHlsl, std::ofstream::out );
	ofs << pSrcCode;
	ofs.clear();

	std::string filenamePdb = GetShaderCompilerCacheDir() + format("\\%p.lld", hash);

	// get defines
	//
	wchar_t names[50][128];
	wchar_t values[50][128];
	DxcDefine defines[50];
	int defineCount = 0;
	if (pDefines != NULL)
	{
		for (auto it = pDefines->begin(); it != pDefines->end(); it++)
		{
			swprintf_s<128>(names[defineCount], L"%S", it->first.c_str());
			swprintf_s<128>(values[defineCount], L"%S", it->second.c_str());
			defines[defineCount].Name = names[defineCount];
			defines[defineCount].Value = values[defineCount];
			defineCount++;
		}
	}

	// check whether DXCompiler is initlialized
	if (s_dxc_create_func == nullptr)
	{
		Trace("Error: s_dxc_create_func() is null, have you called InitDirectXCompiler() ?");
		return false;
	}

	IDxcLibrary* pLibrary;
	ThrowIfFailed(s_dxc_create_func(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary)));

	IDxcBlobEncoding* pSource;
	ThrowIfFailed(pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)pSrcCode, (UINT32)strlen(pSrcCode), CP_UTF8, &pSource));

	IDxcCompiler2 *pCompiler;
	ThrowIfFailed(s_dxc_create_func(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)));

	IncluderDxc Includer(pLibrary);

	std::vector<LPCWSTR> ppArgs;
	// splits params string into an array of strings.
	{
		char params[1024];
		strcpy_s<1024>(params, pParams);

		char* next_token;
		char* token = strtok_s(params, " ", &next_token);
		while (token != NULL)
		{
			wchar_t wide_str[1024];
			swprintf_s<1024>(wide_str, L"%S", token);

			const size_t wide_str2_len = wcslen(wide_str) + 1;
			wchar_t* wide_str2 = (wchar_t*) malloc(wide_str2_len * sizeof(wchar_t));
			wcscpy_s(wide_str2, wide_str2_len, wide_str);
			ppArgs.push_back(wide_str2);

			token = strtok_s(NULL, " ", &next_token);
		}
	}

	wchar_t pEntryPointW[256];
	swprintf_s<256>(pEntryPointW, L"%S", pEntryPoint);

	IDxcOperationResult* pResultPre;
	HRESULT res1 = pCompiler->Preprocess(pSource, L"", NULL, 0, defines, defineCount, &Includer, &pResultPre);
	if (res1 == S_OK)
	{
		Microsoft::WRL::ComPtr<IDxcBlob> pCode1;
		pResultPre->GetResult(pCode1.GetAddressOf());
		std::string preprocessedCode = "";
		preprocessedCode = "// dxc -E" + std::string(pEntryPoint) + " " + std::string(pParams) + " " + filenameHlsl + "\n\n";
	
		if (pDefines)
		{
			for (auto it = pDefines->begin(); it != pDefines->end(); ++it)
			{
				preprocessedCode += "#define " + it->first + " " + it->second + "\n";
			}
		}
			preprocessedCode += std::string((char*)pCode1->GetBufferPointer());
		preprocessedCode += "\n";
		SaveFile(filenameHlsl.c_str(), preprocessedCode.c_str(), preprocessedCode.size(), false);

		IDxcOperationResult* pOpRes;
		HRESULT res;

		if (false)
		{
			Microsoft::WRL::ComPtr<IDxcBlob> pPDB;
			LPWSTR pDebugBlobName[1024];
			res = pCompiler->CompileWithDebug(pSource, NULL, pEntryPointW, L"", ppArgs.data(), (UINT32)ppArgs.size(),
				defines, defineCount, &Includer, &pOpRes, pDebugBlobName, pPDB.GetAddressOf()); res = pCompiler->CompileWithDebug(pSource, NULL, pEntryPointW, L"", ppArgs.data(), (UINT32)ppArgs.size(), defines, defineCount, &Includer, &pOpRes, pDebugBlobName, pPDB.GetAddressOf());

			// Setup the correct name for the PDB.
			if (pPDB)
			{
				char pPDBName[1024];
				sprintf_s(pPDBName, "%s\\%ls", GetShaderCompilerCacheDir().c_str(), *pDebugBlobName);
				SaveFile(pPDBName, pPDB->GetBufferPointer(), pPDB->GetBufferSize(), true);
			}
		}
		else
		{
			res = pCompiler->Compile(pSource, NULL, pEntryPointW, L"", ppArgs.data(), (UINT32)ppArgs.size(), defines, defineCount, &Includer, &pOpRes);
		}

		// Clean up allocated memory.
		while (!ppArgs.empty())
		{
			LPCWSTR pWString = ppArgs.back();
			ppArgs.pop_back();
			free((void*)pWString);
		}

		pSource->Release();
		pLibrary->Release();
		pCompiler->Release();

		IDxcBlob* pResult = NULL;
		IDxcBlobEncoding* pError = NULL;
		if (pOpRes != NULL)
		{
			pOpRes->GetResult(&pResult);
			pOpRes->GetErrorBuffer(&pError);
			pOpRes->Release();
		}

		if (pResult != NULL && pResult->GetBufferSize() > 0)
		{
			*outSpvSize = pResult->GetBufferSize();
			*outSpvData = (char*)malloc(*outSpvSize);

			memcpy(*outSpvData, pResult->GetBufferPointer(), *outSpvSize);

			pResult->Release();

			// Making sure that pError doesn't leak if it was allocated.
			if (pError)
			{
				pError->Release();
			}

#ifdef USE_DXC_SPIRV_FROM_DISK
			SaveFile(filenameOut.c_str(), *outSpvData, *outSpvSize, true);
#endif // USE_DXC_SPIRV_FROM_DISK
			return true;
		}
		else
		{
			IDxcBlobEncoding* pErrorUTF8;
			pLibrary->GetBlobAsUtf8(pError, &pErrorUTF8);

			Trace("*** Error compiling %p.hlsl ***\n", hash );

			std::string filenameErr = GetShaderCompilerCacheDir() + format("\\%p.err", hash);
			SaveFile(filenameErr.c_str(), pErrorUTF8->GetBufferPointer(), pErrorUTF8->GetBufferSize(), false);

			std::string errMsg = std::string((char*)pErrorUTF8->GetBufferPointer(), pErrorUTF8->GetBufferSize());
			Trace(errMsg);

			// Making sure that pError doesn't leak if it was allocated.
			if (pResult)
			{
				pResult->Release();
			}

			pErrorUTF8->Release();
		}
	}

	return false;
}