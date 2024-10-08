//-----------------------------------------------------------------------------
// Torque Shader Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#include "stdafx.h"
# include "../interface/osInterface.h"
#include "platformVideoInfo.h"


//------------------------------------------------------------------------------

PlatformVideoInfo::PlatformVideoInfo() 
{
   
}

//------------------------------------------------------------------------------

PlatformVideoInfo::~PlatformVideoInfo()
{

}

//------------------------------------------------------------------------------


bool PlatformVideoInfo::profileAdapters()
{
   // Initialize the child class
   if( !_initialize() )
      return false;

   mAdapters.clear();

   // Query the number of adapters
   std::string tempString;

   
   mAdapters.push_back( PVIAdapter() );
   //if( !_queryProperty( PVI_NumAdapters, 0, &tempString ) )
   //   return false;

   //mAdapters.increment( dAtoi( tempString ) );

   unsigned int adapterNum = 0;
   for( std::vector<PVIAdapter>::iterator itr = mAdapters.begin(); itr != mAdapters.end(); itr++ )
   {
      PVIAdapter &adapter = *itr;

      
      unsigned int querySuccessFlags = 0xffffffff;
      osassertex( PVI_QueryCount < sizeof( querySuccessFlags ) * 8, "Not enough bits in query success mask." );
      querySuccessFlags -= ( ( 1 << PVI_QueryCount ) - 1 );
      
      // Fill in adapter information
#define _QUERY_MASK_HELPER( querytype, outstringaddr ) \
      querySuccessFlags |= ( _queryProperty( querytype, adapterNum, outstringaddr ) ? 1 << querytype : 0 )

      _QUERY_MASK_HELPER( PVI_NumDevices, &tempString );
      adapter.numDevices = atoi( tempString.c_str() );

      _QUERY_MASK_HELPER( PVI_VRAM, &tempString );
      adapter.vram = atoi( tempString.c_str() );

      _QUERY_MASK_HELPER( PVI_Description, &adapter.description );
      _QUERY_MASK_HELPER( PVI_Name, &adapter.name );
      _QUERY_MASK_HELPER( PVI_ChipSet, &adapter.chipSet );
      _QUERY_MASK_HELPER( PVI_DriverVersion, &adapter.driverVersion );

#undef _QUERY_MASK_HELPER

      // Test flags here for success
   }

   return true;
}

//------------------------------------------------------------------------------

const PlatformVideoInfo::PVIAdapter &PlatformVideoInfo::getAdapterInformation( const unsigned int adapterIndex ) const
{
   osassertex( adapterIndex < mAdapters.size(), "Not that many adapters" );
   return mAdapters[adapterIndex];
}