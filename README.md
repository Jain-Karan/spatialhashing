# 📍 Spatial Hashing Index using SQLite & H3

Efficient spatial indexing with **SQLite** and **H3** (hexagonal geospatial indexing) — enabling performant spatial queries directly within SQLite using a custom extension and an interactive Python app for visualization.

---

## 🚀 Features

* 🔹 **H3-based spatial hashing**: Leverages Uber’s H3 hex indexing system to partition spatial data efficiently. ([GitHub][1])
* 🛠️ **Custom SQLite extension**: Integrate H3 index functions directly into SQLite via a virtual table.
* 🧪 **Sample queries**: Ready-to-run SQL for validating and demonstrating functionality.
* 🗺️ **Streamlit map app**: Python interface to visualize spatial data and test queries.

---

## 📦 Prerequisites

Before you begin, ensure you have the following installed:

### 🧰 System Requirements

* **SQLite** (with extension support)
* Optionally **SpatiaLite** for additional spatial functions
* **OSGeo4W (Windows)** for easy SQLite & SpatiaLite setup

### 🐍 Python Dependencies

Install required Python packages:

```bash
pip install -r requirements.txt
```

---

## ⚙️ Installation

### ⭐ Windows (with OSGeo4W)

1. Download and install **OSGeo4W** (choose *Advanced Install*).
2. Add the `bin` folder from OSGeo4W to your `PATH`.
   This ensures `sqlite3`, `spatialite`, and extension loading work with your shell.

### 🧬 Project Files

Make sure you have:

* `ile-de-france.db` — The geospatial SQLite database
* `my_h3_extension.dll` — Compiled SQLite extension with H3 hashing
* Python scripts & notebooks (`map.py`, etc.)

---

## 📚 Database Setup

1. Place the database file and extension into the **same directory**.
2. Open a SQLite shell (`sqlite3 ile-de-france.db`).
3. Load the extension:

```sql
.load ./my_h3_extension
```

---

## 🧪 Sample Queries

Check out **_Sample Queries.txt** — includes ready-to-run SQL demonstrating spatial hash lookups such as:

* Nearest hex cells by H3 index
* Range queries by hex distance
* Lookup of spatial features via H3 grid

---

## 🖼️ Running the Streamlit App

This lightweight app lets you interactively explore the spatial index:

```bash
python -m streamlit run map.py
```

Open the URL shown in your browser to view an interactive map connected to your spatial database.

---

## 💡 Application Features

* **Efficient Spatial Search:** Use H3 hex indices for fast querying.
* **Map Visualization:** See indexed points and query results visualized.
* **Reusable Extensions:** Loadable SQLite extension lets you integrate spatial hashing into other workflows.

---

## 🧠 Background & Concepts

**Spatial Hashing** transforms a continuous geospatial domain into discrete indexed cells — greatly speeding up proximity queries compared to brute force methods. In this project, we use:

* **H3 hex indexing** — a hierarchical hex grid provided by Uber for global geospatial data. ([GitHub][1])

Hex grids often provide better uniformity than square grids for neighbor lookups and distance calculations on the earth’s surface.

---

## 🛠 Troubleshooting

### ⚠️ “Extension not found” error

* Double-check that `my_h3_extension.dll` is **in the same folder** as your `.db`.
* Confirm your `PATH` includes your SQLite installation with extension support.

### 🐍 Missing Python packages

* Ensure `pip install -r requirements.txt` completed successfully.
* Run inside a virtual environment if conflicts arise.

---

## 📄 License

MIT


[1]: https://github.com/uber/h3?utm_source=chatgpt.com "uber/h3: Hexagonal hierarchical geospatial indexing system"
