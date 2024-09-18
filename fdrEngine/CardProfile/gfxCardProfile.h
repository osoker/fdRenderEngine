//-----------------------------------------------------------------------------
// Torque Shader Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _GFXCARDPROFILE_H_
#define _GFXCARDPROFILE_H_

# include "../interface/stringTable.h"

class GFXCardProfiler
{
   /// @name icpi Internal Card Profile Interface
   ///
   /// This is the interface implemented by subclasses of this class in order
   /// to provide implementation-specific information about the current
   /// card/drivers.
   ///
   /// Basically, the implementation needs to provide some unique strings:
   ///      - mVersionString indicating the current driver version of the
   ///        card in question. (For instance, "53.36")
   ///      - mCardDescription indicating the name of the card ("Radeon 8500")
   ///      - getRendererString() indicating the name of the renderer ("DX9", "GL1.2").
   ///        Each card profiler subclass must return a unique constant so we can keep
   ///        data separate. Bear in mind that punctuation is stripped from filenames.
   ///        
   /// The profiler also needs to implement setupCardCapabilities(), which is responsible
   /// for querying the active device and setting defaults based on the reported capabilities,
   /// and _queryCardCap, which is responsible for recognizing and responding to
   /// device-specific capability queries.
   ///
   /// @{

public:

   ///
   const char* getVersionString() const { return mVersionString.c_str(); }
   const char* getCardString() const { return mCardDescription.c_str(); }
   const char* getChipString() const { return mChipSet.c_str(); }
   unsigned int getVideoMemoryInMB() const { return mVideoMemory; }

   virtual const char *getRendererString() const = 0;

   double   getDoubleValue(const char* _name);
   bool		getBooleanValue(const char* _name);

protected:

   std::string mVersionString;
   std::string mCardDescription;
   std::string mChipSet;
   unsigned int mVideoMemory;

   virtual void setupCardCapabilities()=0;

   /// Implementation specific query code. 
   ///
   /// This function is meant to be overridden by the specific implementation class.
   ///
   /// Some query strings are handled by the external implementation while others must
   /// be done by the specific implementation. This is given first chance to return
   /// a result, then the generic rules are applied.
   ///
   /// @param  query       Capability being queried.
   /// @param  foundResult Result to return to the caller. If the function returns true
   ///                     then this value is returned as the result of the query.
   virtual bool _queryCardCap(const char* query, unsigned int &foundResult)=0;

   //virtual bool _queryFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips ) = 0;
   /// @}

   /// @name helpergroup Helper Functions
   ///
   /// Various helper functions.

   /// Load a specified script file from the profiles directory, if it exists.
   void loadProfileScript(const char* scriptName);
   
   /// Load the script files in order for the specified card profile tuple.
   void loadProfileScripts(const char* render, const char* vendor, const char* card, const char* version);

   char *strippedString(const char*string);

   /// @}
   
   /// Variable dictionary.
   std::map<const char*, double> mVariableDoubleDictionary;
   std::map<const char*, bool> mVariableBoolDictionary;
   std::map<const char*, int> mVariableIntDictionary;

   /// Capability dictionary.
   std::map<const char*, unsigned int> mCapDictionary;

public:
   
   /// @name ecpi External Card Profile Interface
   ///
   /// @{
   
   /// Called for a profile for a given device.
   GFXCardProfiler();
   virtual ~GFXCardProfiler();


   /// Set load script files and generally initialize things.
   virtual void init()=0;

   /// Called to query a capability. Given a query string it returns a 
   /// bool indicating whether or not the capability holds. If you call
   /// this and cap isn't recognized then it returns false and prints
   /// a console error.
   unsigned int queryProfile(const char* cap);

   /// Same as queryProfile(), but a default can be specified to indicate
   /// what value should be returned if the profiler doesn't know anything
   /// about it. If cap is not recognized, defaultValue is returned and
   /// no error is reported.
   unsigned int queryProfile(const char *cap, unsigned int defaultValue);

   /// Set the specified capability to the specified value.
   void setCapability(const char *cap, unsigned int value);

   /// Queries support for the specified texture format, and texture profile
   //bool checkFormat( const D3DFORMAT fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips );

   /// @}
};
extern GFXCardProfiler* gCardProfiler;
#endif

