
# Spatial Hashing Index Using SQLite and H3

## Contributors
- **Aditya Patel**(aditya.s.patel@sjsu.edu): H3 API binding, Virtual Table Module, SQLite extension implementation  
- **Karan Jain**(karan.jain@sjsu.edu): Architecture Design and Implementation, Virtual Table Module, GUI Application

## Overview
This project implements a spatial hashing index using SQLite and H3 to optimize spatial queries. It involves integrating the H3 API with SQLite through a custom virtual table extension, allowing users to efficiently query geospatial data. A Python-based application is provided to interact with the database and visualize results.

## Installation Instructions

### Prerequisites

#### _SQLite and SpatiaLite Installation on Windows_
1. **Download and Install OSGeo4W**:  
   - Visit the [OSGeo4W download page](https://trac.osgeo.org/osgeo4w/) and install the package.
   - OSGeo4W will install all necessary libraries for both Spatialite and SQLite.

2. **Add `bin` folder to System PATH**:  
   After installation, make sure to add the `bin` folder from OSGeo4W to your system PATH. This allows you to access the command line tools (e.g., `sqlite3` and `spatialite`).

#### _Python Libraries_
1. Install the required Python libraries by running the following command in your terminal:
   ```bash
   pip install -r requirements.txt
   ```

---

## Database Setup

1. **Download the Database**:  
   Download the geospatial database from [this link](https://drive.google.com/file/d/1UWzVFWZ601qVjbo0fVf8DHuyV8yLgrac/view?usp=sharing).
   OR paste the following link in browser :
	https://drive.google.com/file/d/1UWzVFWZ601qVjbo0fVf8DHuyV8yLgrac/view?usp=sharing

2. **Place Database and Extension in the Same Folder**:  
   Ensure the following files are in the same directory for seamless operation:
   - `ile-de-france.db` (Database file)
   - `my_h3_extension.dll` (SQLite extension for H3 spatial indexing)

---

## Running the Application

### Sample Queries
1. Launch the SQLite shell and run the sample queries provided in the `_Sample Queries.txt` file.  
   These queries demonstrate how to interact with the spatial hash indexing extension and run spatial queries in SQLite.

2. You can run the queries directly in the `sqlite3` shell or use any SQLite-compatible client.

### Running the Streamlit Application
1. Start the Streamlit application by running the following command in your terminal:
   ```bash
   python -m streamlit run map.py
   ```

2. After running the above command, Streamlit will start a local web server. Open the provided URL in your default browser to access the application.

---

## Application Features
- **Geospatial Querying**: Use the spatial hash indexing feature to run efficient spatial queries on geospatial data.
- **Map Visualization**: The application provides a simple GUI to interact with the map and spatial data, making it easier to visualize the queried results.

---

## Additional Notes
- Ensure that the `ile-de-france.db` file and `my_h3_extension.dll` are in the same directory as the Python script to avoid errors.
- The sample queries provided in `_Sample Queries.txt` are ready to run in the SQLite shell, demonstrating how to utilize the spatial hash index.
- This project uses H3 for spatial indexing, which is a hexagonal grid system that facilitates efficient geospatial queries.

---

## Troubleshooting
- **Error: SQLite extension not found**:  
  Ensure that the `my_h3_extension.dll` file is in the same folder as the SQLite database (`ile-de-france.db`).
  Ensure SYSTEM PATH contains path to OSGeo4W bin (default location: C:\OSGeo4W\bin)

- **Error: Missing Python libraries**:  
  Double-check the `requirements.txt` and ensure all dependencies are installed.
