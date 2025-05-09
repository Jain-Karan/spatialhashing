import sqlite3
import streamlit as st
from streamlit_folium import st_folium
import folium
import pandas as pd
import time
database_path = 'ile-de-france.db'

# Initialize database
@st.cache_resource
def init_db():
    conn = sqlite3.connect(database_path, check_same_thread=False)
    conn.enable_load_extension(True)
    conn.load_extension('my_h3_extension')
    conn.load_extension('mod_spatialite')

    c = conn.cursor()
    c.execute('''
              CREATE VIRTUAL TABLE IF NOT EXISTS my_points_h3_index USING hash_index(points, geometry, 7);
    ''')
    c.execute('''
              CREATE VIRTUAL TABLE IF NOT EXISTS my_points_h3_index9 USING hash_index(points, geometry, 9);
    ''')    
    conn.commit()
    return conn

conn = init_db()

# Map setup
st.title("Spatial Hashing Demo 🌍")
m = folium.Map(location=[48.8584, 2.2945], zoom_start=14)
map_data = st_folium(m, width=700, height=500)

# Track map viewport changes
current_bounds = map_data.get('bounds', [[48.85, 2.29], [48.87, 2.30]])

if 'prev_bounds' not in st.session_state:
    st.session_state.prev_bounds = current_bounds

# Re-query when viewport changes
if current_bounds != st.session_state.prev_bounds:
    st.session_state.prev_bounds = current_bounds
    st.rerun()

# Get visible area bounds
for key in current_bounds.keys():
    print(f"Key: {key}, Value: {current_bounds[key]}")
    if "southWest" in key:
        sw_lat, sw_lon = current_bounds[key]["lat"], current_bounds[key]["lng"]
    elif "northEast" in key:
        ne_lat, ne_lon = current_bounds[key]["lat"], current_bounds[key]["lng"]
print(f"Southwest: {sw_lat}, {sw_lon}")
print(f"Northeast: {ne_lat}, {ne_lon}")
center_lat, center_lon = sw_lat + (ne_lat - sw_lat) / 2, sw_lon + (ne_lon - sw_lon) / 2

# Radius slider
starting_radius = st.slider("Starting Radius (in km)", min_value=200, max_value=2000, value=200, step=200) // 200

# Spatialite query to get points in the current viewport
sql_query = '''
select
    rowid as rowid, name, X(GEOMETRY) AS lon,Y(GEOMETRY) as lat,other_tags as type
from
    points
where
   rowid in (
        SELECT pkid FROM idx_points_geometry
        WHERE
        xmin > {}
        and xmax < {}
        and ymin > {}
        and ymax < {}
    )
AND ST_Distance(
  ST_Transform(geometry, 3857),
  ST_Transform(MakePoint({},{}, 4326), 3857)
) <= {};
'''.format(sw_lon, ne_lon,sw_lat , ne_lat,center_lon, center_lat,starting_radius*200)
print(sql_query)
start_time = time.perf_counter()
df = pd.read_sql(sql_query, conn)
df = df[df['name'].notna()]
print(df)
end_time = time.perf_counter()
st.write(f"SpatialIndex Query execution time: {end_time - start_time:.4f} seconds")


# Hashing query to get points in the current viewport
subquery= "SELECT rowid as rowid, name, X(GEOMETRY) AS lon,Y(GEOMETRY) as lat,other_tags as type FROM points WHERE rowid IN (SELECT rowid FROM my_points_h3_index9 WHERE h3 = h3Neighbors(geoToH3({}, {}, 9), {}));".format(center_lat, center_lon,starting_radius)
start_time = time.perf_counter()
df = pd.read_sql(subquery, conn)
end_time = time.perf_counter()
st.write(f"Hashing Index Query execution time: {end_time - start_time:.4f} seconds")
print(df)

st.header("Areas of Interest 🔍")
    
# Type filter
selected_type = st.selectbox(
    "Filter by type:",
    options=['all', 'cafe', 'heritage', 'landmark']
)

if selected_type != 'all':
    df = df[df['type'].str.contains(selected_type,na=False)]
# filter out empty
df = df[df['name'].notna()]

# Display filtered locations
st.subheader(f"Visible Locations ({len(df)})")
st.map(df[['lat', 'lon']],size=10)
print(df)

# Display raw data
st.subheader("Location Data")
# st.dataframe(df[['name', 'type', 'lat', 'lon']])
st.dataframe(df[['name', 'rowid','type']])