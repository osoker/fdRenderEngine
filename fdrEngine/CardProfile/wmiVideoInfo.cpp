// http://www.spectranaut.net/sourcecode/WMI.cpp

#include "stdafx.h"
# include "../interface/osInterface.h"
#define _WIN32_DCOM

#include <comdef.h>
#include <wbemidl.h>
//#include <atlconv.h>
#pragma comment(lib, "comsuppw.lib") 
#pragma comment(lib, "wbemuuid.lib")

#include "wmiVideoInfo.h"




// Add constructor to GUID.
struct MYGUID : public GUID
{
   MYGUID( DWORD a, SHORT b, SHORT c, BYTE d, BYTE e, BYTE f, BYTE g, BYTE h, BYTE i, BYTE j, BYTE k )
   {
      Data1       = a;
      Data2       = b;
      Data3       = c;
      Data4[ 0 ]  = d;
      Data4[ 1 ]  = e;
      Data4[ 2 ]  = f;
      Data4[ 3 ]  = g;
      Data4[ 4 ]  = h;
      Data4[ 5 ]  = i;
      Data4[ 6 ]  = j;
      Data4[ 7 ]  = k;
   }
};

//------------------------------------------------------------------------------
// DXGI decls for retrieving device info on Vista.  We manually declare that
// stuff here, so we don't depend on headers and compile on any setup.  At
// run-time, it depends on whether we can successfully load the DXGI DLL; if
// not, nothing of this here will be used.

struct IDXGIObject;
struct IDXGIFactory;
struct IDXGIAdapter;
struct IDXGIOutput;

struct DXGI_SWAP_CHAIN_DESC;
struct DXGI_ADAPTER_DESC;

struct IDXGIObject : public IUnknown
{
   virtual HRESULT STDMETHODCALLTYPE SetPrivateData( REFGUID, UINT, const void* ) = 0; 
   virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( REFGUID, const IUnknown* ) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetPrivateData( REFGUID, UINT*, void* ) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetParent( REFIID, void** ) = 0;
};

struct IDXGIFactory : public IDXGIObject
{
   virtual HRESULT STDMETHODCALLTYPE EnumAdapters( UINT, IDXGIAdapter** ) = 0;
   virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation( HWND, UINT ) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation( HWND ) = 0;
   virtual HRESULT STDMETHODCALLTYPE CreateSwapChain( IUnknown*, DXGI_SWAP_CHAIN_DESC* ) = 0;
   virtual HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter( HMODULE, IDXGIAdapter** ) = 0;
};
                 
struct IDXGIAdapter : public IDXGIObject
{
   virtual HRESULT STDMETHODCALLTYPE EnumOutputs( UINT, IDXGIOutput** ) = 0;
   virtual HRESULT STDMETHODCALLTYPE GetDesc( DXGI_ADAPTER_DESC* ) = 0;
   virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport( REFGUID, LARGE_INTEGER* ) = 0;
};

struct DXGI_ADAPTER_DESC
{
   WCHAR Description[ 128 ];
   UINT VendorId;
   UINT DeviceId;
   UINT SubSysId;
   UINT Revision;
   SIZE_T DedicatedVideoMemory;
   SIZE_T DedicatedSystemMemory;
   SIZE_T SharedSystemMemory;
   LUID AdapterLuid;
};

static MYGUID IID_IDXGIFactory( 0x7b7166ec, 0x21c7, 0x44ae, 0xb2, 0x1a, 0xc9, 0xae, 0x32, 0x1a, 0xe3, 0x69 );

//------------------------------------------------------------------------------
// DXDIAG declarations.

struct DXDIAG_INIT_PARAMS
{
   DWORD    dwSize;
   DWORD    dwDxDiagHeaderVersion;
   BOOL     bAllowWHQLChecks;
   LPVOID   pReserved;
};

struct IDxDiagContainer : public IUnknown
{
   virtual HRESULT   STDMETHODCALLTYPE GetNumberOfChildContaiiners( DWORD* pdwCount ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE EnumChildContainerNames( DWORD dwIndex, LPWSTR pwszContainer, DWORD cchContainer ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetChildContainer( LPCWSTR pwszContainer, IDxDiagContainer** ppInstance ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetNumberOfProps( DWORD* pdwCount ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE EnumPropNames( DWORD dwIndex, LPWSTR pwszPropName, DWORD cchPropName ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetProp( LPCWSTR pwszPropName, VARIANT* pvarProp ) = 0;
};

struct IDxDiagProvider : public IUnknown
{
   virtual HRESULT   STDMETHODCALLTYPE Initialize( DXDIAG_INIT_PARAMS* pParams ) = 0;
   virtual HRESULT   STDMETHODCALLTYPE GetRootContainer( IDxDiagContainer** ppInstance ) = 0;
};

static MYGUID CLSID_DxDiagProvider( 0xA65B8071, 0x3BFE, 0x4213, 0x9A, 0x5B, 0x49, 0x1D, 0xA4, 0x46, 0x1C, 0xA7 );
static MYGUID IID_IDxDiagProvider( 0x9C6B4CB0, 0x23F8, 0x49CC, 0xA3, 0xED, 0x45, 0xA5, 0x50, 0x00, 0xA6, 0xD2 );
static MYGUID IID_IDxDiagContainer( 0x7D0F462F, 0x4064, 0x4862, 0xBC, 0x7F, 0x93, 0x3E, 0x50, 0x58, 0xC1, 0x0F );

//------------------------------------------------------------------------------

WCHAR *WMIVideoInfo::smPVIQueryTypeToWMIString [] =
{
   L"MaxNumberControlled",    //PVI_NumDevices
   L"Description",            //PVI_Description
   L"Name",                   //PVI_Name
   L"VideoProcessor",         //PVI_ChipSet
   L"DriverVersion",          //PVI_DriverVersion
   L"AdapterRAM",             //PVI_VRAM
};

//------------------------------------------------------------------------------

WMIVideoInfo::WMIVideoInfo()
   : PlatformVideoInfo(),
     mLocator( NULL ),
     mServices( NULL ),
     mComInitialized( false ),
     mDXGIModule( NULL ),
     mDXGIFactory( NULL ),
     mDxDiagProvider( NULL )
{

}

//------------------------------------------------------------------------------

WMIVideoInfo::~WMIVideoInfo()
{
   SAFE_RELEASE( mLocator );
   SAFE_RELEASE( mServices );

   if( mDxDiagProvider )
      SAFE_RELEASE( mDxDiagProvider );

   if( mDXGIFactory )
      SAFE_RELEASE( mDXGIFactory );
   if( mDXGIModule )
      FreeLibrary( ( HMODULE ) mDXGIModule );

   if( mComInitialized )
      CoUninitialize();
}

//------------------------------------------------------------------------------

bool WMIVideoInfo::_initialize()
{
   // Init COM
   HRESULT hr = CoInitialize( NULL );
   mComInitialized = SUCCEEDED( hr );

   if( !mComInitialized )
      return false;

   bool success = false;

   success |= _initializeDXGI();
   success |= _initializeDxDiag();
   success |= _initializeWMI();

   return success;
}

bool WMIVideoInfo::_initializeWMI()
{
   //// Set security levels 
   //hr = CoInitializeSecurity(
   //   NULL, 
   //   -1,                          // COM authentication
   //   NULL,                        // Authentication services
   //   NULL,                        // Reserved
   //   RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
   //   RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
   //   NULL,                        // Authentication info
   //   EOAC_NONE,                   // Additional capabilities 
   //   NULL                         // Reserved
   //   );

   //if( FAILED( hr ) )
   //{
   //   osDebugOut( "WMIVideoInfo: Failed to initialize com security." );
   //   return false;
   //}

   // Obtain the locator to WMI 
   HRESULT hr = CoCreateInstance(
      CLSID_WbemLocator,             
      0, 
      CLSCTX_INPROC_SERVER, 
      IID_IWbemLocator, 
      (void**)&mLocator
      );

   if( FAILED( hr ) )
   {
      osDebugOut( "WMIVideoInfo: Failed to create instance of IID_IWbemLocator." );
      return false;
   }

   // Connect to the root\cimv2 namespace with
   // the current user and obtain pointer pSvc
   // to make IWbemServices calls.
   hr = mLocator->ConnectServer(
      BSTR(L"ROOT\\CIMV2"), // Object path of WMI namespace
      NULL,                    // User name. NULL = current user
      NULL,                    // User password. NULL = current
      0,                       // Locale. NULL indicates current
      NULL,                    // Security flags.
      0,                       // Authority (e.g. Kerberos)
      0,                       // Context object 
      &mServices               // pointer to IWbemServices proxy
      );

   if( FAILED( hr ) )
   {
      osDebugOut( "WMIVideoInfo: Connect server failed." );
      return false;
   }


   // Set security levels on the proxy 
   hr = CoSetProxyBlanket(
      mServices,                   // Indicates the proxy to set
      RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
      RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
      NULL,                        // Server principal name 
      RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
      RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
      NULL,                        // client identity
      EOAC_NONE                    // proxy capabilities 
      );

   if( FAILED( hr ) )
   {
      osDebugOut( "WMIVideoInfo: CoSetProxyBlanket failed" );
      return false;
   }

   return true;
}

bool WMIVideoInfo::_initializeDXGI()
{
   // DXGI stuff works, but loading dxgi.dll seems to cause crashes in the DX rendering code.
   // Deactivated for now.

#if 0
   // Try going for DXGI.  Will only succeed on Vista.

   mDXGIModule = ( HMODULE ) LoadLibrary( L"dxgi.dll" );
   if( mDXGIModule != 0 )
   {
      typedef HRESULT (* CreateDXGIFactoryFuncType )( REFIID, void** );
      CreateDXGIFactoryFuncType factoryFunction =
         ( CreateDXGIFactoryFuncType ) GetProcAddress( ( HMODULE ) mDXGIModule, "CreateDXGIFactory" );

      if( factoryFunction && factoryFunction( IID_IDXGIFactory, ( void** ) &mDXGIFactory ) == S_OK )
         return true;
      else
      {
         FreeLibrary( ( HMODULE ) mDXGIModule );
         mDXGIModule = 0;
      }
   }
#endif
   return false;
}

bool WMIVideoInfo::_initializeDxDiag()
{
   if( CoCreateInstance( CLSID_DxDiagProvider, NULL, CLSCTX_INPROC_SERVER, IID_IDxDiagProvider, ( void** ) &mDxDiagProvider ) == S_OK )
   {
      DXDIAG_INIT_PARAMS params;
      memset( &params, 0, sizeof( DXDIAG_INIT_PARAMS ) );

      params.dwSize                 = sizeof( DXDIAG_INIT_PARAMS );
      params.dwDxDiagHeaderVersion  = 111;
      params.bAllowWHQLChecks       = false;

      HRESULT result = mDxDiagProvider->Initialize( &params );
      if( result != S_OK )
      {
         osDebugOut( "WMIVideoInfo: DxDiag initialization failed (%i)", result );
         SAFE_RELEASE( mDxDiagProvider );
         return false;
      }
      else
      {
         osDebugOut( "WMIVideoInfo: DxDiag initialized" );
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
// http://msdn2.microsoft.com/en-us/library/aa394512.aspx
// 
// The Win32_VideoController WMI class represents the capabilities and management capacity of the 
// video controller on a computer system running Windows.
//
// Starting with Windows Vista, hardware that is not compatible with Windows Display Driver Model (WDDM) 
// returns inaccurate property values for instances of this class.
//
// Windows Server 2003, Windows XP, Windows 2000, and Windows NT 4.0:  This class is supported.
//------------------------------------------------------------------------------

bool WMIVideoInfo::_queryProperty( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue )
{
   if( _queryPropertyDXGI( queryType, adapterId, outValue ) )
      return true;
   else if( _queryPropertyDxDiag( queryType, adapterId, outValue ) )
      return true;
   else
      return _queryPropertyWMI( queryType, adapterId, outValue );
}

bool WMIVideoInfo::_queryPropertyDxDiag( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue )
{
   if( mDxDiagProvider != 0 )
   {
      IDxDiagContainer* rootContainer = 0;
      IDxDiagContainer* displayDevicesContainer = 0;
      IDxDiagContainer* deviceContainer = 0;

      WCHAR adapterIdString[ 2 ];
      adapterIdString[ 0 ] = L'0' + adapterId;
      adapterIdString[ 1 ] = L'\0';

      std::string value;
      if( mDxDiagProvider->GetRootContainer( &rootContainer ) == S_OK
         && rootContainer->GetChildContainer( L"DxDiag_DisplayDevices", &displayDevicesContainer ) == S_OK
         && displayDevicesContainer->GetChildContainer( adapterIdString, &deviceContainer ) == S_OK )
      {
         const WCHAR* propertyName = 0;

         switch( queryType )
         {
         case PVI_Description:
            propertyName = L"szDescription";
            break;

         case PVI_Name:
            propertyName = L"szDeviceName";
            break;

         case PVI_ChipSet:
            propertyName = L"szManufacturer";
            break;

         case PVI_DriverVersion:
            propertyName = L"szDriverVersion";
            break;

         // Don't get VRAM via DxDiag as that won't tell us about the actual amount of dedicated
         // video memory but rather some dedicated+shared RAM value.
         }

		 //TEST CODE
		/* DWORD iCnt;
		deviceContainer->GetNumberOfProps(&iCnt);
		 WCHAR temp[512];
		 for (int i = 0; i < iCnt;i++)
		 {
			 VARIANT val;
			 deviceContainer->EnumPropNames(i,temp,512);
			 deviceContainer->GetProp( temp, &val ) ;
			 if (val.vt== VT_BSTR)
			 {
				 osDebugOut(va("%s->%s",temp,_bstr_t( val.bstrVal )));
			 }
			 
		 }*/
		 //
         if( propertyName )
         {
            VARIANT val;
            if( deviceContainer->GetProp( propertyName, &val ) == S_OK )
               switch( val.vt )
               {
               case VT_BSTR:
                  value = _bstr_t( val.bstrVal );
                  break;

               default:
                  osassertex( false, va( "WMIVideoInfo: property type '%i' not implemented", val.vt ) );
               }
         }
      }

      if( rootContainer )
         SAFE_RELEASE( rootContainer );
      if( displayDevicesContainer )
         SAFE_RELEASE( displayDevicesContainer );
      if( deviceContainer )
         SAFE_RELEASE( deviceContainer );

      if( !value.empty() )
      {
         // Try to get the DxDiag data into some canonical form.  Otherwise, we
         // won't be giving the card profiler much opportunity for matching up
         // its data with profile scripts.

         switch( queryType )
         {
         case PVI_ChipSet:
            if( strnicmp(value.c_str(),"ATI",3) == 0 )
               value = "ATI Technologies Inc.";
            else if( strnicmp(value.c_str(),"NVIDIA",6) == 0)
               value = "NVIDIA";
            else if( strnicmp(value.c_str(),"INTEL",5) == 0  )
               value = "INTEL";
            else if( strnicmp(value.c_str(),"MATROX",6) == 0  )
               value = "MATROX";
            break;

         case PVI_Description:
            if(strnicmp(value.c_str(),"ATI",3) == 0  )
            {
               value = value.substr( 4, value.length() - 4 );
               if( strnicmp(value.c_str(),"Series",6) == 0   )
                  value = value.substr( 0, value.length() - 7 );
            }
            else if(strnicmp(value.c_str(),"NVIDIA",6) == 0  )
               value = value.substr( 7, value.length() - 7 );
            else if(strnicmp(value.c_str(),"INTEL",5) == 0 )
               value = value.substr( 6, value.length() - 6 );
            else if(strnicmp(value.c_str(),"MATROX",6) == 0 )
               value = value.substr( 7, value.length() - 7 );
            break;
         }

         *outValue = value;
         return true;
      }
   }
   return false;
}

bool WMIVideoInfo::_queryPropertyDXGI( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue )
{
#if 0
   if( mDXGIFactory )
   {
      IDXGIAdapter* adapter;
      if( mDXGIFactory->EnumAdapters( adapterId, &adapter ) != S_OK )
         return false;

      DXGI_ADAPTER_DESC desc;
      if( adapter->GetDesc( &desc ) != S_OK )
      {
         adapter->Release();
         return false;
      }

      std::string value;
      switch( queryType )
      {
      case PVI_Description:
         value = ( desc.Description );
         break;

      case PVI_Name:
         value = ( va( "%i", desc.DeviceId ) );
         break;

      case PVI_VRAM:
         value = ( va( "%i", desc.DedicatedVideoMemory ) );
         break;

         //RDTODO
      }

      adapter->Release();
      *outValue = value;
      return true;
   }
#endif
   return false;
}

bool WMIVideoInfo::_queryPropertyWMI( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue )
{
   if( mServices == NULL )
      return false;

   // Use the IWbemServices pointer to make requests of WMI
   IEnumWbemClassObject *enumerator;
   HRESULT hr = mServices->ExecQuery(
         BSTR("WQL"), 
         BSTR("SELECT * FROM Win32_VideoController"),
         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
         NULL,
         &enumerator
      );

   if( FAILED( hr ) )
      return false;

   // Uh I think this is right...get the proper adapter
   IWbemClassObject *adapter = NULL;
   for( int i = 0; i < (int)adapterId + 1; i++ )
   {
      ULONG count = 0;
      HRESULT hr = enumerator->Next( WBEM_INFINITE, 1, &adapter, &count );

      if( FAILED( hr ) || count == 0 )
      {
         enumerator->Release();
         return false;
      }
   }

   // Now get the property
   VARIANT v;
   hr = adapter->Get( smPVIQueryTypeToWMIString[queryType], 0, &v, 0, 0 );

   if( SUCCEEDED( hr ) )
   {
      switch( v.vt )
      {
         case VT_I4:
            {
               LONG longVal = v.lVal;

               if( queryType == PVI_VRAM )
                  longVal = longVal >> 20; // Convert to megabytes

               *outValue = va("%d", (int)longVal );
               break;
            }

         case VT_UI4:
            {
               *outValue = va("%d", (unsigned int)v.ulVal );
               break;
            }

         case VT_BSTR:
            *outValue = _bstr_t( v.bstrVal );
            break;
         case VT_LPSTR:
         case VT_LPWSTR:
            break;
      }

      VariantClear( &v );
   }
   else
   {
      adapter->Release();
      enumerator->Release();
      return false;
   }

   adapter->Release();

   // Cleanup
   enumerator->Release();

   return true;
}