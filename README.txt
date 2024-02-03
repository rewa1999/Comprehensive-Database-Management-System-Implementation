Assignment 3: Record Manager 

Group Number: 4

Name: Siddhi Shukla  	CWID: A20516414
Name: Anushka Chaubal 	CWID: A20511568
Name: Rewa Deshpande	CWID: A20492328


Code Development:
1. Connect to the server using Secure Shell (SSH) or any remote access tool.
2. Navigate to the directory containing Assignment3 using the `cd` command.
3. Verify the files in the directory by using the `ls` command to ensure you are in the correct location.
4. Delete any previously compiled object files by running the 'clean' script with the command `./clean.sh`.
5. After cleaning, compile all the necessary files by executing the 'build' script with the command `./build.sh`.
6. Run the test cases by using the command `./run_tests` for the 'testassign3' and 'test_expr' programs.


OVERVIEW OF FUNCTIONS

The Record Manager, Buffer Pool, Buffer Manager, and Storage Manager are components that collectively provide functionality for managing tables, data storage, and page access in an efficient manner. Here's a summarized overview of their roles and the methods/functions they offer:

Record Manager:
- Provides essential functionalities for managing tables.
- Offers functions for creating, opening, shutting down, and removing tables.
- Uses the Storage Manager internally to handle low-level disk operations.

Buffer Pool and Buffer Manager:
- Optimize page access using the LRU (Least Recently Used) page replacement policy.
- Manage the buffer pool, which is a portion of memory used to cache frequently accessed pages from the disk.
- Facilitate efficient data retrieval by reducing disk I/O operations.

Storage Manager:
- Handles low-level disk operations to facilitate data storage and retrieval efficiently.
- Provides functions for initializing the storage system, reading/writing blocks from/to disk, and managing files.

Initialization and Termination:
- Use `initRecordManager()` to launch the Record Manager. This function internally calls `initStorageManager()` from the Storage Manager to set up the storage system.
- Use `shutdownRecordManager()` to terminate the Record Manager. This function releases all memory and resources used by the Record Manager and the underlying components.

createTable() Method:
- Starts a new table with a specified name, configures its settings, and stores the initial block containing the table in the page file.
- Uses the LRU page replacement policy to manage the buffer pool and optimize page access.

openTable() Method:
- Creates a new table with the specified name and schema.
- Manages loading required pages into the buffer pool using the LRU page replacement policy to improve data access performance.

closeTable() Function:
- Closes the table using the `shutdownBufferPool()` method from the Buffer Manager.
- Ensures that any table alterations are written to the page file before closing the table.

deleteTable() Method:
- Deletes a table with the specified name using the `destroyPageFile()` function from the Storage Manager.
- Releases the memory space designated for the associated pages and removes them from the disk.

getNumTuples() Method:
- Returns the total number of tuples in the table as its final result.
- Retrieves this information from the table's metadata.



RECORD FUNCTIONS

Insert Record (insertRecord()): Using the supplied Record ID, a new record is added to a table. The method updates the table's existing record with the specified Record ID.

Delete Record (deleteRecord()): Using the Record ID "rId" and "tbData" parameters, the deleteRecord() method eliminates a record from a table.

Update Record (updateRecord()): Using the "tbData" and "record" arguments, the updateRecord() method modifies an existing record inside the specified table.

Get Record (getRecord()): Using the Record ID "rId" and the table indicated by the "tbData" parameter, the getRecord() method retrieves a specific record from the table and places it in the provided location "record".



SCAN FUNCTIONS

Input: RM ScanHandle data object and the scan condition (Expr).
Purpose: Initializes the scan process by setting up the necessary variables connected to the scan within the RM ScanHandle data structure.
Behavior: Produces an error code if the scan condition (Expr) is NULL.

next() Method:
Input: RM ScanHandle data object.
Output: Retrieves the next tuple that satisfies the scan condition (Expr).
Behavior: Provides a suitable error code if the scan condition is not met.

closeScan() Method:
Input: RM ScanHandle data object.
Purpose: Finalizes the scan process and performs necessary cleanup tasks.
Behavior: Checks if the scan is finished. If not, it unpins the page, resets all scan-related variables, and releases the memory used by the metadata.


SCHEMA FUNCTIONS 

1. getRecordSize() Method:
   - Input: Schema information (characteristics) for a record.
   - Output: Size of the record in bytes.
   - Purpose: Determines and returns the size of a record in the specified schema by adding up the space needs (in bytes) for each characteristic.
   - Behavior: Calculates the total size required to store a record with the given schema.

2. freeSchema() Function:
   - Input: Schema information (characteristics) to be freed from memory.
   - Purpose: Releases the memory consumed by the schema specified in the 'schema' argument.
   - Behavior: Uses the 'free()' method to deallocate the memory that was previously allocated to store the schema.

3. createSchema() Method:
   - Input: Properties of the new schema to be created.
   - Output: Newly created schema in the system's memory.
   - Purpose: Creates a brand-new schema with the requested properties in the system's memory.
   - Behavior: Allocates memory to store the schema and initializes its characteristics based on the provided properties.




ATTRIBUTE FUNCTIONS

Based on the information provided, there are four main methods related to handling records within a schema:

1. createRecord() Method:
   - Input: Supplied record and the specified schema.
   - Purpose: Inserts a new record into the specified schema using the supplied record as input.
   - Behavior: Creates a new record in the schema and populates it with the data from the supplied record.

2. freeRecord() Function:
   - Input: Record whose memory needs to be deallocated.
   - Purpose: Uses the C function `free()` to release the memory used by a particular record.
   - Behavior: Deallocates the memory occupied by the record, preventing memory leaks and freeing resources.

3. getAttr() Method:
   - Input: Record from the given schema and the specific attribute to extract.
   - Output: Value of the extracted attribute from the record.
   - Purpose: Extracts a specific attribute from a record in the given schema.
   - Behavior: Accesses the desired attribute from the record and returns its value.

4. setAttr() Function:
   - Input: Record inside the given schema and a specific attribute value to set.
   - Purpose: Assigns a specific attribute value to a record within the given schema.
   - Behavior: Updates the value of the specified attribute in the record.
