# Spatial Hashing
Spatial Hashing Index using SQLite and H3

<hr>

## Responsibilites : 
[Aditya Patel](aditya.s.patel@sjsu.edu) :  H3 references, SQLite extension implementation  
[Karan Jain](karan.jain@sjsu.edu):  Architecture Design, Virtual Table Module, GUI Application 

<hr>

## Installation :

### SQLite and Spatialite :

*_WINDOWS_*  
Download and install [OSGeo4W](https://trac.osgeo.org/osgeo4w/).  
This will install all necessary libraries for Spatialite and SQLite.  
Add `bin` folder to the SYSTEM PATH.  

### Python libraries :
requirments.txt has most of the libraries.
Run `pip install -r requirements.txt`

<hr>

## Running : 

### Sample Queries :
Run the available queries in _Sample Queries.txt_ from within `sqlite3` shell.
### Application :
Run `python -m streamlit run map.py` and follow the url to the application in the default browser.
