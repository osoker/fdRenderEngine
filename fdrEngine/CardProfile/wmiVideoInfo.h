#ifndef _WMI_CARDINFO_H_
#define _WMI_CARDINFO_H_

#include "platformVideoInfo.h"

struct IWbemLocator;
struct IWbemServices;

struct IDXGIFactory;
struct IDxDiagProvider;

class WMIVideoInfo : public PlatformVideoInfo
{
private:
   IWbemLocator *mLocator;
   IWbemServices *mServices;
   bool mComInitialized;

   void*             mDXGIModule;
   IDXGIFactory*     mDXGIFactory;
   IDxDiagProvider*  mDxDiagProvider;

   bool _initializeDXGI();
   bool _initializeDxDiag();
   bool _initializeWMI();

   bool _queryPropertyDXGI( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue );
   bool _queryPropertyDxDiag( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue );
   bool _queryPropertyWMI( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue );

protected:
   static WCHAR *smPVIQueryTypeToWMIString [];
   bool _queryProperty( const PVIQueryType queryType, const unsigned int adapterId, std::string *outValue );
   bool _initialize();

public:
   WMIVideoInfo();
   ~WMIVideoInfo();
};

#endif