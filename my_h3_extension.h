#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

#include "./h3lib/include/h3api.h"
#include "./sqlite3/sqlite3.h"
#include "./spatialite/include/spatialite_ext.h"
#include "./spatialite/include/spatialite.h"
#include "./spatialite/include/spatialite_private.h"

/*
#include "./geos/include/geos.h"
#include "./geos/include/geos_c.h"
#include "./proj/include/proj.h"
*/
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <assert.h>
#include <string.h>


/*


#ifndef SPATIAL_EXTENSION_H
#define SPATIAL_EXTENSION_H
#ifdef __cplusplus
extern "C" {
#endif

	static void spatial_func(sqlite3_context* context, int argc, sqlite3_value** argv);
	static int insert_point_geometry(sqlite3* db, double x, double y);
	static int create_spatial_virtual_table(sqlite3* db);

#ifdef __cplusplus
}
#endif
#endif // SPATIAL_EXTENSION_H


*/