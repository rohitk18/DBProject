# Database System Implementation
UFID: 90971158

This is a database project of the course “Database System Implementation” (COP6726) taught in University of Florida.

### About:
- A simple command line database system supporting a subset of SQL. Basic operations include create tables and display output based on queries.
- Heap and sorted file implementation to manage database records.
- Basic relational operations include select, project, join, sum, group by, duplicate removal.
- Query optimization to decide and execute optimized query plan using histograms.

### Initial Setup:
- Sample data: tpch-dbgen(https://github.com/electrum/tpch-dbgen.git)
- To get this sample data, run the following commands in shell:
```
cd tpch-dbgen
make
// to generate 10MB data
./dbgen -s 0.01
// to generate 1GB data
./dbgen -s 1
```
- Clone this database project and go to the project directory. Create new directories named 'DATA','DB' and 'schema-catalog'. Then copy the .tbl files generated in tpch-dbgen directory to this project directory. Follow the following commands in shell for this step:
```
// in the DBProject
mkdir DATA DB schema-catalog
cp <tpch-dbgen directory location>/*.tbl ./DATA/
```
- Build the project: `make`
- Run the project: `./main.out`
- List of tables: nation, region, customer, part, partsupp, supplier, orders, lineitem
- Create the tables by running the following queries:
```
CREATE TABLE region (r_regionkey INTEGER, r_name STRING, r_comment STRING) AS HEAP
CREATE TABLE nation (n_nationkey INTEGER, n_name STRING, n_regionkey INTEGER, n_comment STRING) AS HEAP
CREATE TABLE part (p_partkey INTEGER, p_name STRING, p_mfgr STRING, p_brand STRING, p_type STRING, p_size INTEGER, p_container STRING, p_retailprice DOUBLE, p_comment STRING) AS HEAP
CREATE TABLE supplier (s_suppkey INTEGER, s_name STRING, s_address STRING, s_nationkey INTEGER, s_phone STRING, s_acctbal DOUBLE, s_comment STRING) AS HEAP
CREATE TABLE partsupp (ps_partkey INTEGER, ps_suppkey INTEGER, ps_availqty INTEGER, ps_supplycost DOUBLE, ps_comment STRING) AS HEAP
CREATE TABLE customer (c_custkey INTEGER, c_name STRING, c_address STRING, c_nationkey INTEGER, c_phone STRING, c_acctbal DOUBLE, c_mktsegment STRING, c_comment STRING) AS HEAP
CREATE TABLE orders (o_orderkey INTEGER, o_custkey INTEGER, o_orderstatus STRING, o_totalprice DOUBLE, o_orderdate STRING, o_orderpriority STRING, o_clerk STRING, o_shippriority INTEGER, o_comment STRING) AS HEAP
CREATE TABLE lineitem (l_orderkey INTEGER, l_partkey INTEGER, l_suppkey INTEGER, l_linenumber INTEGER, l_quantity DOUBLE, l_extendedprice DOUBLE, l_discount DOUBLE, l_tax DOUBLE, l_returnflag STRING, l_linestatus STRING, l_shipdate STRING, l_commitdate STRING, l_receiptdate STRING, l_shipinstruct STRING, l_shipmode STRING) AS HEAP
```
![Create Query](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Create.png?raw=true)
- To fill the tables with data, run this query for each table:
```
INSERT './DATA/<table>.tbl' INTO table
```
![Insert Query](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Insert.png?raw=true)
- The database is ready with data now.

### Outputs:
![](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Query%201.png?raw=true)  
![](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Query%202.png?raw=true)  
![](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Query%203.png?raw=true)  
![](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Query%204.png?raw=true)  
![](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Query%205.png?raw=true)  
![](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Query%206.png?raw=true)  

### Concepts:
About different classes and the system structure.
- Record - Actual objects stored in database.
- Page - A collection of database records for both in memory and disk. Basic operations include append records, binary conversion.
- File - Disk based container holding array of pages. Operations involve open/close file, add/get page.
- DBFile - Handles File and page objects. Operations include create, open, close database files, load records from generated data, adds and gets records.
- GenericDBFile - Parent class to handle DBFile with sorted and heap based strategies. Child classes include heap and sorted.
- Comparison - Holds the comparison rule.
- ComparisonEngine - Uses the comparison object to compare records.
- Schema - Encodes a few functions that load up a relation schema from the database catalog.
- Pipe - A queue buffer for records based on FIFO.
- BigQ - Handles sorting of records based on disk based priority queue using TPMMS (Two Pass Multiway Merge Sort). It consists of input pipe which pushes records to this algorithm, sorts and then sends to output pipe.
- RelationalOp - This base class involves getting data from input pipes, run the operation it is defined and put the result in the output pipe. Its child classes are:
  - SelectPipe - Applies CNF from input pipe to output pipe.
  - SelectFile - Applies CNF from input DBFile to output pipe.
  - Project -  Applies attributes to input records and their order and send to output pipe.
  - Join - Takes two input pipes, merges and pushes records to outpipe. Uses BigQ.
  - DuplicateRemoval - Uses BigQ to filter duplicated records from input pipe and pushes records to output pipe.
  - Sum - Computes sum aggregate function on records from input pipe and pushes single tuple to output pipe.
  - GroupBy - Does grouping, puts one sum into the output pipe for each group.
  - WriteOut - Accepts input pipe, uses schema to write text version of output records to the file.
- Statistics - Uses statistical information during optimization process.
- Query Runner - Based on the input SQL query, the query runner gets query plan and executes the output.
- Query Plan - Plan built on statistics to be used in query runner to return output.
- RelOpPlanNode - Query runner uses instances from this class to work with query plan and relop.
- Parser - Parses input SQL statement.

### Project Structure
![Database Structure](https://github.com/rohitk18/DBProject/blob/C++-implementation/screenshots/Architecture.png?raw=true)
