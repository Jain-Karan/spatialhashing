#include "my_h3_extension.h"

static void h3ToStringFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 1);
	H3Index cell = sqlite3_value_int64(argv[0]);
	char result[17];
	H3Error error = h3ToString(cell, result, 17); if (error) { sqlite3_result_error(context, "Invalid H3 index", -1); return; }
	sqlite3_result_text(context, result, -1, SQLITE_TRANSIENT);
}

static void stringToH3Func(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 1);
	// Return type is unsigned
	const char* indexStr = (const char*)sqlite3_value_text(argv[0]);
	H3Index cell;
	H3Error error = stringToH3(indexStr, &cell); if (error) { sqlite3_result_error(context, "Invalid H3 index", -1); return; }
	sqlite3_result_int64(context, (int64_t)cell);
}

static void h3ToGeoFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 1);
	H3Index cell;
	H3Error error;

	if (sqlite3_value_type(argv[0]) == SQLITE_TEXT) {
		// Input is a string (H3 index in hexadecimal form)
		const char* h3IndexStr = (const char*)sqlite3_value_text(argv[0]);
		if (h3IndexStr == NULL) {
			sqlite3_result_error(context, "Invalid H3 Index string", -1);
			return;
		}

		// Convert the string to a H3 index
		error = stringToH3(h3IndexStr, &cell);
	}
	else if (sqlite3_value_type(argv[0]) == SQLITE_INTEGER) {
		// Input is an integer (raw H3 index)
		cell = (uint64_t)sqlite3_value_int64(argv[0]);
		error = E_SUCCESS;
	}
	else {
		sqlite3_result_error(context, "Expected string or int64 for H3 index", -1);
		return;
	}

	if (error) {
		sqlite3_result_error(context, "Invalid H3 index", -1);
		return;
	}

	//Convert H3 Index to Lat/Lng
	LatLng geo;
	error = cellToLatLng(cell, &geo); if (error) { sqlite3_result_error(context, "Invalid H3 index", -1); return; }
	double lat_deg = radsToDegs(geo.lat);
	double lng_deg = radsToDegs(geo.lng);

	// Return result as string
	char result[50];
	snprintf(result, sizeof(result), "%f,%f", lat_deg, lng_deg);
	sqlite3_result_text(context, result, -1, SQLITE_TRANSIENT);	
}

static void geoToH3Func(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 3);
	double lat = sqlite3_value_double(argv[0]);
	double lng = sqlite3_value_double(argv[1]);
	int res = sqlite3_value_int(argv[2]);

	LatLng geo = { .lat = degsToRads(lat), .lng = degsToRads(lng)};
	H3Index cell;
	H3Error error = latLngToCell(&geo, res, &cell); if (error) { sqlite3_result_error(context, "Invalid Coordinates or Resolution", -1); return; }
		
	sqlite3_result_int64(context, (int64_t)cell);
}

static void h3ParentFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 2 || argc == 1);
	int res;
	H3Index cell = sqlite3_value_int64(argv[0]);
	if (argc == 1) {
		res = getResolution(cell) - 1;
	}
	else {
		res = sqlite3_value_int(argv[1]);
	}
	H3Index parent;
	H3Error error = cellToParent(cell, res, &parent); if (error) { sqlite3_result_error(context, "Function call failed", -1); return; }
	sqlite3_result_int64(context, (int64_t)parent);	
}

static void h3DistanceFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 2);
	H3Index cell1 = sqlite3_value_int64(argv[0]);
	H3Index cell2 = sqlite3_value_int64(argv[1]);
	int64_t dist;
	H3Error error = gridDistance(cell1, cell2, &dist); if (error) { sqlite3_result_error(context, "Function call failed", -1); return; }
	sqlite3_result_double(context, dist);
}

static void geoDistanceMFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 4);
	double lat1 = sqlite3_value_double(argv[0]);
	double lng1 = sqlite3_value_double(argv[1]);
	double lat2 = sqlite3_value_double(argv[2]);
	double lng2 = sqlite3_value_double(argv[3]);

	LatLng geo1 = { .lat = degsToRads(lat1), .lng = degsToRads(lng1) };
	LatLng geo2 = { .lat = degsToRads(lat2), .lng = degsToRads(lng2) };
	
	double dist = greatCircleDistanceM(&geo1, &geo2);
	sqlite3_result_double(context, dist);
}

static void h3NeighborsFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
	assert(argc == 2 || argc == 1);
	int k;
	H3Index cell = sqlite3_value_int64(argv[0]);
	if (argc == 1) {
		k = 1;
	}
	else {
		k = sqlite3_value_int(argv[1]);
	}
	H3Index neighbors[400];
	H3Error error = gridDisk(cell, k, neighbors); if (error) { sqlite3_result_error(context, "Function call failed", -1); return; }

	//Write neigbours to a string
	char result[8000] = "";
	//hexagons in k-ring
	int neighbor_count = 1 + 6 * (k * (k + 1)) / 2;
	// Iterate through the neighbors and construct the string
	for (size_t i = 0; i < neighbor_count; ++i) {
		char temp[20];
		sprintf(temp, "%lld", neighbors[i]); 
		strcat(result, temp); 

		if (i != neighbor_count - 1) {
			strcat(result, ","); // Add a comma after each element, except the last one
		}
	}
	sqlite3_result_text(context, result, -1, SQLITE_TRANSIENT);
}

// cellToBoundary
// polygonToCells
// cellAreaM2
// edgeLengthM
// getResolution
// cellToChildren
// cellToCenterChild
// cellToCenterChild
// areNeighborCells



// Function to extract latitude and longitude from WKT point string
int extract_lat_lng(const char* wkt_point, double* lat, double* lng) {
	// Check if the string starts with "POINT("
	if (strncmp(wkt_point, "POINT(", 6) == 0) {
		const char* coords_start = wkt_point + 6;
		const char* coords_end = strchr(coords_start, ')');
		if (coords_end != NULL) {
			// Temporarily terminate the string at the closing parenthesis
			char temp = *coords_end;  // Save the original character
			*((char*)coords_end) = '\0';  // Temporarily null-terminate the coordinates string

			// Use sscanf to extract latitude and longitude
			if (sscanf(coords_start, "%lf %lf", lat, lng) == 2) {
				// Restore the original character
				*((char*)coords_end) = temp;
				return 1;  // Success
			}
		}
	}
	return 0;  // Error: Invalid WKT format or failed to parse
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------------

static int h3_from_spatialite_blob(const void* blob, int len, int res, H3Index* out_h3) {
	const unsigned char* b = (const unsigned char*)blob;

	if (len < 59) return SQLITE_ERROR;  // Minimum size for full SpatiaLite POINT blob

	uint8_t endian = b[1];  // Byte 1: endian marker
	uint32_t geom_type;
	memcpy(&geom_type, b + 39, 4);  // Byte 39–42: geometry type

	if (endian == 0) geom_type = __builtin_bswap32(geom_type);  // Convert if big endian

	//fprintf(stderr, "Raw BLOB bytes: ");
	//for (int i = 0; i < len && i < 64; i++) {
	//	fprintf(stderr, "%02X ", b[i]);
	//}
	//fprintf(stderr, "\n");

	//fprintf(stderr, "Endian: %d, Geom Type: %u (0x%X)\n", endian, geom_type, geom_type);

	if (geom_type != 1) {  // 1 = Point
		fprintf(stderr, "Invalid or unexpected geometry type: %u (0x%X)\n", geom_type, geom_type);
		return SQLITE_ERROR;
	}

	// Coordinates are at byte 43–58 (8 bytes each)
	double x, y;
	memcpy(&x, b + 43, 8);  // X = Longitude
	memcpy(&y, b + 51, 8);  // Y = Latitude

	// Optional: handle endianness of doubles if needed
	// Assume little endian for now (common case)

	LatLng geo = {
		.lat = degsToRads(y),
		.lng = degsToRads(x)
	};

	H3Index cell;
	H3Error error = latLngToCell(&geo, res, &cell);
	if (error) {
		fprintf(stderr, "H3 conversion failed.\n");
		return SQLITE_ERROR;
	}
	fprintf(stderr, "Generated H3 Index: %llu\n", (unsigned long long)cell);

	*out_h3 = (sqlite3_int64)cell;
	return SQLITE_OK;
}
// Virtual table and cursor structure definitions
typedef struct {
	sqlite3_vtab base;
	sqlite3* db;
	char* storage_table;  // Store the storage table name here
} HashIndexVTab;


typedef struct {
	sqlite3_vtab_cursor base;
	sqlite3_int64 h3;
	sqlite3_int64* rowids;
	int row_index;
	int rowid_count;
} HashIndexCursor;

// Virtual Table: Create and Connect
static int hashIndexCreate(sqlite3* db, void* pAux, int argc, const char* const* argv, sqlite3_vtab** ppVtab, char** pzErr) {
	HashIndexVTab* pVTab = sqlite3_malloc(sizeof(HashIndexVTab));
	memset(pVTab, 0, sizeof(HashIndexVTab));
	pVTab->db = db;

	const char* table_name = argv[3];
	const char* column_name = argv[4];
	int h3_res = atoi(argv[5]);


	// Derive the real table name (e.g., "h3_index_my_points_geometry")
	char* storage_table = sqlite3_mprintf("h3_index_%s_%s", table_name, column_name);
	pVTab->storage_table = storage_table;


	// Declare schema for the virtual table: columns 'h3' for H3Index, 'rowids' for the list of rowids
	sqlite3_declare_vtab(db, "CREATE TABLE x(h3 BIGINT, rowids TEXT)");
	*ppVtab = (sqlite3_vtab*)pVTab;

	// Create the storage table if it doesn't exist
	char* create_sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (h3 BIGINT PRIMARY KEY, rowids TEXT);", storage_table);
	sqlite3_exec(db, create_sql, 0, 0, 0);
	sqlite3_free(create_sql);


	// Query the base table to extract geometry and calculate the H3 index
	char* sql = sqlite3_mprintf("SELECT rowid, \"%w\" FROM \"%w\";", column_name, table_name);
	sqlite3_stmt* stmt = NULL;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
		*pzErr = sqlite3_mprintf("Failed to query table: %s", sqlite3_errmsg(db));
		sqlite3_free(sql);
		return SQLITE_ERROR;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		sqlite3_int64 rowid = sqlite3_column_int64(stmt, 0);
		const void* blob = sqlite3_column_blob(stmt, 1);
		int len = sqlite3_column_bytes(stmt, 1);

		if (blob && len > 0) {
			H3Index h3;
			int err = h3_from_spatialite_blob(blob, len, h3_res, &h3);
			if (err != 0) {
				*pzErr = sqlite3_mprintf("Error extracting H3 index from blob");
				sqlite3_finalize(stmt);
				sqlite3_free(sql);
				return SQLITE_ERROR;
			}

			// Check if the rowid already exists in the storage table
			char* rowids = sqlite3_mprintf("%lld", rowid);
			char* select_sql = sqlite3_mprintf("SELECT rowids FROM %s WHERE h3 = %llu", storage_table, h3);
			sqlite3_stmt* select_stmt;

			if (sqlite3_prepare_v2(db, select_sql, -1, &select_stmt, NULL) != SQLITE_OK) {
				sqlite3_free(select_sql);
				sqlite3_free(rowids);
				return SQLITE_ERROR;
			}

			if (sqlite3_step(select_stmt) == SQLITE_ROW) {
				// If H3 index already exists, append the new rowid
				const char* existing_rowids = (const char*)sqlite3_column_text(select_stmt, 0);
				char* new_rowids = sqlite3_mprintf("%s,%s", existing_rowids, rowids);
				char* update_sql = sqlite3_mprintf("UPDATE %s SET rowids = '%s' WHERE h3 = %llu", storage_table, new_rowids, h3);
				sqlite3_exec(db, update_sql, 0, 0, 0);
				sqlite3_free(update_sql);
				sqlite3_free(new_rowids);
			}
			else {
				// If H3 index doesn't exist, insert a new row
				char* insert_sql = sqlite3_mprintf("INSERT INTO %s (h3, rowids) VALUES (%llu, '%s')", storage_table, h3, rowids);
				sqlite3_exec(db, insert_sql, 0, 0, 0);
				sqlite3_free(insert_sql);
			}

			sqlite3_finalize(select_stmt);
			sqlite3_free(select_sql);
			sqlite3_free(rowids);
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_free(sql);
	return SQLITE_OK;
}

static int hashIndexConnect(sqlite3* db, void* pAux, int argc, const char* const* argv, sqlite3_vtab** ppVtab, char** pzErr) {
	return hashIndexCreate(db, pAux, argc, argv, ppVtab, pzErr);
}

static int hashIndexDisconnect(sqlite3_vtab* pVtab) {
	sqlite3_free(((HashIndexVTab*)pVtab)->storage_table);
	sqlite3_free(pVtab);
	return SQLITE_OK;
}

static int hashIndexDestroy(sqlite3_vtab* pVtab) {
	return hashIndexDisconnect(pVtab);
}

static int hashIndexBestIndex(sqlite3_vtab* tab, sqlite3_index_info* pIdxInfo) {
	for (int i = 0; i < pIdxInfo->nConstraint; i++) {
		if (pIdxInfo->aConstraint[i].usable &&
			pIdxInfo->aConstraint[i].iColumn == 0 &&
			pIdxInfo->aConstraint[i].op == SQLITE_INDEX_CONSTRAINT_EQ) {
			pIdxInfo->aConstraintUsage[i].argvIndex = 1;
			pIdxInfo->aConstraintUsage[i].omit = 1;
			pIdxInfo->idxNum = 1;
			break;
		}
	}
	return SQLITE_OK;
}

static int hashIndexOpen(sqlite3_vtab* pVtab, sqlite3_vtab_cursor** ppCursor) {
	HashIndexCursor* pCursor = sqlite3_malloc(sizeof(HashIndexCursor));
	memset(pCursor, 0, sizeof(HashIndexCursor));
	*ppCursor = (sqlite3_vtab_cursor*)pCursor;
	return SQLITE_OK;
}

static int hashIndexClose(sqlite3_vtab_cursor* cur) {
	HashIndexCursor* pCursor = (HashIndexCursor*)cur;
	if (pCursor->rowids) {
		sqlite3_free(pCursor->rowids);
	}
	sqlite3_free(pCursor);
	return SQLITE_OK;
}

static int hashIndexEof(sqlite3_vtab_cursor* cur) {
	HashIndexCursor* pCursor = (HashIndexCursor*)cur;
	return pCursor->row_index >= pCursor->rowid_count;  // EOF condition
}

static int hashIndexNext(sqlite3_vtab_cursor* cur) {
	HashIndexCursor* pCursor = (HashIndexCursor*)cur;

	// Debugging: Print the current index and rowid count
	fprintf(stderr, "Next: Current row_index = %d, rowid_count = %d\n", pCursor->row_index, pCursor->rowid_count);

	if (pCursor->row_index < pCursor->rowid_count) {
		pCursor->row_index++;  // Increment row_index safely
	}
	else {
		// We've reached the end of the rowids, set EOF
		pCursor->row_index = pCursor->rowid_count;
	}

	return SQLITE_OK;
}



static int hashIndexColumn(sqlite3_vtab_cursor* cur, sqlite3_context* ctx, int i) {
	HashIndexCursor* pCursor = (HashIndexCursor*)cur;

	if (i == 0) {
		fprintf(stderr, "Returning h3 = %lld\n", pCursor->h3);
		sqlite3_result_int64(ctx, pCursor->h3);
	}
	else if (i == 1) {
		fprintf(stderr, "Returning rowid = %lld\n", pCursor->rowids[pCursor->row_index]);
		sqlite3_result_int64(ctx, pCursor->rowids[pCursor->row_index]);
	}
	return SQLITE_OK;
}


static int hashIndexRowid(sqlite3_vtab_cursor* cur, sqlite3_int64* pRowid) {
	HashIndexCursor* pCursor = (HashIndexCursor*)cur;

	// Debugging: Print the current row_index and rowid value
	fprintf(stderr, "Fetching rowid for row_index = %d\n", pCursor->row_index);

	if (pCursor->row_index < pCursor->rowid_count) {
		*pRowid = pCursor->rowids[pCursor->row_index];
	}
	else {
		*pRowid = -1;  // Invalid rowid when out of bounds
	}

	return SQLITE_OK;
}

static int hashIndexFilter(sqlite3_vtab_cursor* cur, int idxNum, const char* idxStr, int argc, sqlite3_value** argv) {
	HashIndexCursor* pCursor = (HashIndexCursor*)cur;

	// Check if the expected number of arguments is passed (we expect at least one argument)
	if (argc < 1) {
		pCursor->rowid_count = 0;
		return SQLITE_OK;
	}

	// Extract the H3 value from the first argument
	sqlite3_int64 query_h3 = sqlite3_value_int64(argv[0]);

	// Debug output: Print the H3 value being filtered
	fprintf(stderr, "Filtering on h3: %lld\n", query_h3);

	// Obtain a pointer to the virtual table
	HashIndexVTab* pVTab = (HashIndexVTab*)(pCursor->base.pVtab);
	sqlite3* db = pVTab->db;

	pCursor->rowids = NULL;
	pCursor->rowid_count = 0;

	// Prepare the SQL to fetch rowids based on the H3 index
	sqlite3_stmt* stmt;
	char* query_sql = sqlite3_mprintf("SELECT rowids FROM %s WHERE h3 = ?", pVTab->storage_table);

	// Debug output: Print the query that is being executed
	fprintf(stderr, "Executing SQL: %s\n", query_sql);

	int rc = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
	sqlite3_free(query_sql);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Error preparing query: %s\n", sqlite3_errmsg(db));
		return SQLITE_ERROR;
	}

	sqlite3_bind_int64(stmt, 1, query_h3);


	// Execute the statement and fetch results
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		// Get the comma-separated rowids
		const char* rowids_str = (const char*)sqlite3_column_text(stmt, 0);

		// Debug output: Print the rowids string retrieved from the database
		fprintf(stderr, "Retrieved rowids: %s\n", rowids_str);

		// If there are no rowids, set the rowid_count to 0
		if (rowids_str == NULL || *rowids_str == '\0') {
			pCursor->rowid_count = 0;
		}
		else {
			// Split the comma-separated rowids and count them
			char* rowids_copy = sqlite3_mprintf("%s", rowids_str);
			char* token = strtok(rowids_copy, ",");

			// Debug output: Print each token (rowid) during the split
			fprintf(stderr, "Parsing rowids:\n");
			pCursor->rowid_count = 0;
			while (token) {
				fprintf(stderr, "  - %s\n", token);  // Print the rowid token
				pCursor->rowid_count++;
				token = strtok(NULL, ",");
			}

			// Allocate memory to store the rowids
			pCursor->rowids = sqlite3_malloc(sizeof(sqlite3_int64) * pCursor->rowid_count);

			// Reset the rowids_copy for actual parsing
			strcpy(rowids_copy, rowids_str);
			token = strtok(rowids_copy, ",");
			int index = 0;
			while (token) {
				pCursor->rowids[index++] = atoll(token);
				token = strtok(NULL, ",");
			}

			// Free the temporary rowids_copy
			sqlite3_free(rowids_copy);
		}
	}

	// Finalize the statement
	sqlite3_finalize(stmt);

	// Debug output: Confirm that we finished the query execution and parsed the rowids
	fprintf(stderr, "Completed rowid parsing\n");

	// Set the cursor to point to the first row
	pCursor->h3 = query_h3;
	pCursor->row_index = 0;

	return SQLITE_OK;
}



// Register the module
static sqlite3_module HashIndexModule = {
	0,
	hashIndexCreate,   // xCreate
	hashIndexConnect,   // xConnect
	hashIndexBestIndex, // xBestIndex
	hashIndexDisconnect, // xDisconnect
	hashIndexDestroy,    // xDestroy
	hashIndexOpen,       // xOpen
	hashIndexClose,      // xClose
	hashIndexFilter,     // xFilter
	hashIndexNext,       // xNext
	hashIndexEof,        // xEof
	hashIndexColumn,     // xColumn
	hashIndexRowid,      // xRowid
	/* Optional methods are not implemented in this simple version: */
	0,      /* xUpdate */
	0,      /* xBegin */
	0,      /* xSync */
	0,      /* xCommit */
	0,      /* xRollback */
	0,      /* xFindFunction */
	0,      /* xRename */
	0,      /* xSavepoint */
	0,      /* xRelease */
	0,      /* xRollbackTo */
	0       /* xShadowName */
};




#ifdef _WIN32
__declspec(dllexport)
#endif

int sqlite3_myhextension_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi) {

	SQLITE_EXTENSION_INIT2(pApi);
	int error = SQLITE_OK;
	
	error = sqlite3_create_function(db, "h3ToString", 1, SQLITE_UTF8, NULL, h3ToStringFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "stringToH3", 1, SQLITE_UTF8, NULL, stringToH3Func, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "h3ToGeo", 1, SQLITE_UTF8, NULL, h3ToGeoFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "geoToH3", 3, SQLITE_UTF8, NULL, geoToH3Func, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "h3Parent", 1, SQLITE_UTF8, NULL, h3ParentFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "h3Parent", 2, SQLITE_UTF8, NULL, h3ParentFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "h3Distance", 2, SQLITE_UTF8, NULL, h3DistanceFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "geoDistanceM", 4, SQLITE_UTF8, NULL, geoDistanceMFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "h3Neighbors", 1, SQLITE_UTF8, NULL, h3NeighborsFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	error = sqlite3_create_function(db, "h3Neighbors", 2, SQLITE_UTF8, NULL, h3NeighborsFunc, NULL, NULL); if (error != SQLITE_OK) { return error; }
	
	sqlite3_create_module(db, "hash_index", &HashIndexModule, 0);
	return error;
}