//-----------------------------------------------------------------------------
// Torque Shader Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#include "stdafx.h"
# include "../interface/osInterface.h"
# include "../../common/include/tinyxml.h"
#include "gfxCardProfile.h"

GFXCardProfiler* gCardProfiler = NULL;

void GFXCardProfiler::loadProfileScript(const char* aScriptName)
{
//   char scriptName[512];
   std::string   profilePath = "data\\profile\\";//Con::getVariable( "$Pref::Video::ProfilePath" );
   
   profilePath.append(aScriptName);
   //strcat(scriptName, aScriptName);

   if (::file_exist((char*)profilePath.c_str()))
   {
	   int		t_i = 0;
	   int   t_iGBufIdx = -1;
	   BYTE*	t_ptrStart = START_USEGBUF( t_iGBufIdx );
	   t_i = ::read_fileToBuf( (char*)profilePath.c_str(), t_ptrStart, TMP_BUFSIZE );
	   if( t_i <= 0 )
	   {
		   END_USEGBUF( t_iGBufIdx );
		   osDebugOut( va("调入profile文件出错<%s>..\n", profilePath.c_str() ) );
		   return;
	   }
	   TiXmlDocument xmlDoc;
	   xmlDoc.Parse( (const char*)t_ptrStart );
	   END_USEGBUF( t_iGBufIdx );


	   //osassert( !xmlDoc.Error() );

	   // std::string effName;
	   TiXmlElement* shaEle = xmlDoc.FirstChildElement("global");
	   osassert(shaEle);

	   int Count = atoi(shaEle->Attribute("Count"));
	   for (int i = 0; i < Count;i++)
	   {
		   TiXmlElement* variableEle = xmlDoc.FirstChildElement(va("ID_%d",i));
		   osassert(variableEle);
		   if (stricmp(variableEle->Attribute("Type"),"Double")==0)
		   {
			   mVariableDoubleDictionary[StringTable->insert(variableEle->Attribute("Variabe"))] = 
				   atof(variableEle->Attribute("Value"));
		   }
		   else if(stricmp(variableEle->Attribute("Type"),"Bool")==0){
			   const char* t_value = variableEle->Attribute("Value");
			   bool t_bool = strcmp(t_value,"True") == 0;
			   const char* t_key = StringTable->insert(variableEle->Attribute("Variabe"));
			   
			   mVariableBoolDictionary.insert(std::make_pair(t_key,t_bool));

		   }else
		   {
			   //! TODO ....
		   }

	   }
	   osDebugOut(va("加载显卡Profile %s\n",profilePath.c_str()));
   }
   //else
	//   osDebugOut(va("没有找到显卡Profile %s",profilePath.c_str()));
}

void GFXCardProfiler::loadProfileScripts(const char* render, const char* vendor, const char* card, const char* version)
{
	osDebugOut("   - 加载显卡 profiles...\n");

	char script[512] = "";

	_snprintf(script, 512, "%s.xml", render);
	loadProfileScript(script);

	_snprintf(script, 512, "%s.%s.xml", render, vendor);
	loadProfileScript(script);

	_snprintf(script, 512, "%s.%s.%s.xml", render, vendor, card);
	loadProfileScript(script);

	_snprintf(script, 512, "%s.%s.%s.%s.xml", render, vendor, card, version);
	loadProfileScript(script);
}

GFXCardProfiler::GFXCardProfiler() : mVideoMemory( 0 )
{
}

GFXCardProfiler::~GFXCardProfiler()
{
	mVariableDoubleDictionary.clear();
	mVariableIntDictionary.clear();
   mCapDictionary.clear();
}

char * GFXCardProfiler::strippedString(const char *string)
{
   // Allocate the new buffer.
   char *res = new char[strlen(string)+1];

   // And fill it with the stripped string...
   const char *a=string;
   char *b=res;
   while(*a)
   {
      if(isalnum(*a))
      {
         *b = *a;
         b++;
      }
      a++;
   }
   *b = '\0';

   return res;
}

void GFXCardProfiler::init()
{
	// Spew a bit...
	osDebugOut_tag("DisplayCard","\n   初始化显卡信息:%s\n   o 制造商		:%s\n   o 型号		:%s\n   o 版本		: %s\n",
					getRendererString(),getChipString(),getCardString(),getVersionString());

	setupCardCapabilities();

	// And finally, load stuff up...
	char *render  = strippedString(getRendererString());
	char *chipset  = strippedString(getChipString());
	char *card    = strippedString(getCardString());
	char *version = strippedString(getVersionString());

	
	loadProfileScripts(render, chipset, card, version);

	// Clean up.
	delete []render;
	delete []chipset;
	delete []card;
	delete []version;
}

unsigned int GFXCardProfiler::queryProfile(const char* cap)
{
   unsigned int res;
   if( _queryCardCap( cap, res ) )
      return res;

   //if(mCapDictionary.has(cap))
	return mCapDictionary[cap];

   //osDebugOut(va( "GFXCardProfiler (%s) - Unknown capability '%s'.", getRendererString(), cap ));
   return 0;
}

unsigned int GFXCardProfiler::queryProfile(const char* cap, unsigned int defaultValue)
{
   unsigned int res;
   if( _queryCardCap( cap, res ) )
      return res;

//   if( mCapDictionary.contains( cap ) )
      return mCapDictionary[cap];
//   else
 //     return defaultValue;
}

void GFXCardProfiler::setCapability(const char* cap, unsigned int value)
{
   // Check for dups.
   //if( mCapDictionary.contains( cap ) )
   //{
     // Con::warnf( "GFXCardProfiler (%s) - Setting capability '%s' multiple times.", getRendererString().c_str(), cap.c_str() );
      mCapDictionary[cap] = value;
      //return;
   //}

   // Insert value as necessary.
   //Con::warnf( "GFXCardProfiler (%s) - Setting capability '%s' to %d.", getRendererString().c_str(), cap.c_str(), value );
   //mCapDictionary.insert( cap, value );
}

//bool GFXCardProfiler::checkFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips )
//{
//   return _queryFormat( fmt, profile, inOutAutogenMips );
//}
double GFXCardProfiler::getDoubleValue(const char* _name)
{
	return mVariableDoubleDictionary[_name];

}
bool GFXCardProfiler::getBooleanValue(const char* _name)
{
	return mVariableBoolDictionary[StringTable->insert(_name)];
}