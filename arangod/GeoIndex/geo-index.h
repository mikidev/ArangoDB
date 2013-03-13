////////////////////////////////////////////////////////////////////////////////
/// @brief geo index
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Copyright 2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_GEO_INDEX_GEO_INDEX_H
#define TRIAGENS_GEO_INDEX_GEO_INDEX_H 1

#include "BasicsC/common.h"

#include "VocBase/index.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                         GEO INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a geo-index for lists
////////////////////////////////////////////////////////////////////////////////

TRI_index_t* TRI_CreateGeo1Index (struct TRI_primary_collection_s* collection,
                                  char const* locationName,
                                  TRI_shape_pid_t location,
                                  bool geoJson,
                                  bool constraint,
                                  bool ignoreNull);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a geo-index for arrays
////////////////////////////////////////////////////////////////////////////////

TRI_index_t* TRI_CreateGeo2Index (struct TRI_primary_collection_s* collection,
                                  char const* latitudeName,
                                  TRI_shape_pid_t latitude,
                                  char const* longitudeName,
                                  TRI_shape_pid_t longitude,
                                  bool constraint,
                                  bool ignoreNull);

////////////////////////////////////////////////////////////////////////////////
/// @brief frees the memory allocated, but does not free the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroyGeoIndex (TRI_index_t* idx);

////////////////////////////////////////////////////////////////////////////////
/// @brief frees the memory allocated and frees the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeGeoIndex (TRI_index_t* idx);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up all points within a given radius
////////////////////////////////////////////////////////////////////////////////

GeoCoordinates* TRI_WithinGeoIndex (TRI_index_t* idx,
                                    double lat,
                                    double lon,
                                    double radius);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up the nearest points
////////////////////////////////////////////////////////////////////////////////

GeoCoordinates* TRI_NearestGeoIndex (TRI_index_t* idx,
                                     double lat,
                                     double lon,
                                     size_t count);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
