<?php
// $Id:$

/**
 * @file
 * 
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * @author  Mike Kot <mdk312@lehigh.edu>
 * 
 * @version 1.0
 *
 * \brief Provides access to Oracle and MySQL database functions and data structures in a transparent way.
 * 
 * The most important part of this module is the DAEDatabase class. This is the main API that allows a programmer
 * to take advantage of any DBMS without having to change his or her code. This class is further derived into
 * sub-classes that actually implement the core access functions to specific DBMSs like Oracle and MySQL.
 * 
 * @section LICENSE
 * 
 *  This file is part of the DAE Platform Project (DAE).
 *
 *  DAE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DAE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DAE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
  
require_once("./".drupal_get_path('module','dae_data')."/dae_data_person.inc");

/**
 * 
 */ 
define(DATA_ITEM_UNKNOWN, 600);
define(DATA_ITEM_PAGE_IMAGE, 601);
define(DATA_ITEM_DATASET, 602);
define(DATA_ITEM_PAGE_ELEMENT, 603);
define(DATA_ITEM_FILE, 604);

/**
 * \brief Get access to the current DAEDatabase instance.
 * 
 * @returns the DAEDatabase <a href="http://en.wikipedia.org/wiki/Singleton_pattern">singleton</a>.
 */
function getDAEDatabase()
{
	return DAEDatabase::getDAEDatabase();
}

/**
 * \brief Generic class, providing all database interaction functions.
 *  
 * This class is intended to be a single-point access to any kind of DBMS. 
 * It is currently extendended by inheritance to function with \link DAEOracleDatabase Oracle\endlink and \link DAEMySQLDatabase MySQL\endlink.
 * 
 * The class is implemented as a <a href="http://en.wikipedia.org/wiki/Singleton_pattern">Singleton Design Pattern</a>. It is therefore guaranteed to have only a single instance at one time.
 * 
 * @author Bart Lamiroy <lamiroy@cse.lehigh.edu>
 *
 */
class DAEDatabase
{
		protected $conn; //!< Connection identifier
		protected $user; //!< Database connection user name
		protected $pass; //!< Database connection user password
		
		protected $safe_conn; //!< Connection identifier for safe connexions
		protected $ro_user; //!< Database read-only connection user name
		protected $ro_pass; //!< Database read-only connection user password
		
		protected $host; //!< Database hostname
		
		var $dbmsType; //!< Database Management System used
		
		//! Singleton Pattern
		static protected $database = null;
		
		//! String representing the database system date variable or function
		var $defaultSysdateString = 'SYSDATE';
		//! String representing the database system date variable or function
		var $sysdateString = 'SYSDATE';
		//! Database function providing conversion from a 'Date' format to a 'String' format  (DBMS dependant)
		var $dateConversionString = 'to_char';
		//! Conversion parameters for 'Date' to 'String' (DBMS dependant)
		var $timestampFormat = 'd-M-Y h:i:s A';

		/**
		 * \brief Constructor
		 * 
		 * Retrieves the Drupal the following global variables:
		 *   - \a dae_db_user: affected to \a $this->user
		 *   - \a dae-db_pass: affected to \a $this->pass
		 *   - \a dae-db_server: affected to \a $this->host
		 *   
		 * \a $this->dbmsType is set to blank.
		 */
		function __construct()
		{
		   $this->user = variable_get('dae_db_user','');
		   $this->pass = variable_get('dae_db_pass','');
		   
		   $this->ro_user = variable_get('dae_db_readonly_user',$this->user);
		   $this->ro_pass = variable_get('dae_db_readonly_pass',$this->pass);
		   $this->host = variable_get('dae_db_server','localhost');	

		   $this->dbmsType = '';
		}

		/**
		 * \brief Retrieves the current active Database or creates one if it doesn't exist (Singleton Design Pattern)
		 * 
		 * This function violates classical inheritance encapsulation, in the sense that it creates derived class instances
		 * in function of the Drupal \a dae_db_application global variable.
		 * 
		 */
		static function getDAEDatabase()
		{
			/**
			 * Somewhere, somehow, when the Drupal daedatabase configuration is changed, this database gets reinitialized. 
			 * That's the way it should, but it's not quite clear how and where this is done.
			 */
			if(DAEDatabase::$database == null) 
			{
				if(variable_get('dae_db_application',0) == 1)
					DAEDatabase::$database = new DAEMySQLDatabase();
				else
					DAEDatabase::$database = new DAEOracleDatabase();
			}
	
			if(DAEDatabase::$database == null) 
			{
				databaseDebugMessage("Database not Created !");
			} else
			{
				//databaseDebugMessage("Database idenfifier = " . print_r($database,true));
			}
	
			return DAEDatabase::$database;
		}
		
		/**
		 * \brief Current connection identifier \a $this->conn
		 * 
		 * \returns the current connection identifier \a $this->conn if the calling Drupal User has the required access rights to do so. 
		 * Returns call to getSafeConn() otherwise. 
		 */
		function getConn()
		{
			if(user_access('raw DAE database access')) {
				return $this->conn;
			}
			else 
			    return $this->getSafeConn();
		}

		/**
		 * \brief Current safe connection identifier \a $this->conn
		 * 
		 * \returns the current safe connection identifier \a $this->safe_conn otherwise. 
		 */
		function getSafeConn()
		{
			if(user_access('raw read-only DAE database access')) {
				return $this->safe_conn;
			}
			else {
                return null;
			}
   		}

		/**
		 * \brief Transparent call to subclass function \a q( ) using the same parameters. Return value is discarded.
		 * 
		 * Please note that \a q( ) is not defined in current class and is supposed to be implemented within the inherited class
		 * 
		 * @param [in] $query a valid SQL query string
		 * @return nothing
		 */
		function i($query, $variables = -1)
		{	
		    if($variables == -1)
		        databaseDebugMessage("daedatabase i() function called without substitution variables !",true);
		        
		    $this->q($query, $variables);
		}
		
		/**
		 * \brief Transparent call to subclass function \a q( ) using the same parameters. Return single row from result.
		 * 
		 * Please note that \a q( ) is not defined in current class and is supposed to be implemented within the inherited class.
		 * 
		 * @param [in] $query a valid SQL query string
		 * @param [in] $variables array of substitution values to inject into \a $query
		 * 
		 * @return first row of the query result, discarding the rest
		 */
		function r($query, $variables = -1)//for getting a single unique value from a query
		{
		    if($variables == -1)
		        databaseDebugMessage("daedatabase r() function called without substitution variables !",true);
		        
			$result = $this->q($query,$variables);
			$row = $this->fetch_array($result);
			return $row[0];
		}		
		
		/**
		 * \brief Transparent call to a DMBS specific version returning the number of fields returned by a query
		 * 
		 * This function should be overloaded by deriving classes
		 * 
		 * @param [in] $query the query that was issued (not the SQL string, but the DBMS-specific structure. e.g. 'ressource')
		 * @return the number of fields returned by the query
		 */
		function num_fields($query)
		{
			databaseDebugMessage("Non Overloaded Function called !");
			return 0;	
		}
		
		/**
		 * \brief Transparent call to a DMBS specific version returning the name of a field returned by a query
		 * 
		 * This function should be overloaded by deriving classes		
		 * Fields are supposed to be numbered starting from 0.
		 * 
		 * @param [in] $query the query that was issued (not the SQL string, but the DBMS-specific structure. e.g. 'ressource')
		 * @param [in] $i index of the column the field name is to be returned
		 * @return the name of field number \a $i in query \a $query.
		 */
		function field_name($query, $i)
		{
			databaseDebugMessage("Non Overloaded Function called !");
			return "";
		}
		
		/**
		 * \brief Transparent call to a DMBS specific version returning the current row of a query result
		 * 
		 * This function should be overloaded by deriving classes		
		 * Each call is supposed to position the current row to the next row.

		 * @param [in] $query the query that was issued (not the SQL string, but the DBMS-specific structure. e.g. 'ressource')
		 * @return the current row of the query result
		 */
		function fetch_row($query)
		{
			databaseDebugMessage("Non Overloaded Function called !");
			return array();		
		}
		
		/**
		 * \brief Converts a date formatting string from the standard Oracle syntax to the effecively
		 * 		  used DBMS syntax. 
		 * 
		 * This function is supposed to be overloaded by inheriting classes.
		 * 
		 * @param [in] $format the Oracle syntax format string
		 * @return a format string in the appropriate syntax. This string contains embedded beginning and end quotes. 
		 */
		function convertDateFormat($format)
		{
			return "'" . $format . "'";
		}
		
		/**
		 * \brief Generic function that builds upon overloading by inheriting classes to generate a date conversion query that is complatible by the underlying DMBS. 
		 * 
		 * The query is supposed to be in the form of \a F($fields, $format) like \a to_char(SYSDATE,"MON DD").
		 * This function will replace:
		 *  - the standard conversion function name by the overridden function name
		 * 	- convert SYSDATE references 
		 *  - replace Oracle syntax with the correct format string syntax
		 * 
		 * @param [in] $fields
		 * @param [in] $format
		 * @return the DMBS compliant SQL query for date conversion 
		 */
		function dateFormat($fields,$format) 
		{
			$updatedfields = preg_replace('/'.$this->defaultSysdateString.'/', $this->sysdateString, $fields);
			$convertedformat = $this->convertDateFormat($format);
			
			return $this->dateConversionString . '(' . $updatedfields . ',' . $convertedformat . ')';
		}
		
		/**
		 * \brief SQL query returning type of a given dataitem id.
		 *  
		 * @param [in] $id Database id number from \a dataitem table.
		 * @return \a DATA_ITEM_PAGE_IMAGE if \a $id is a \a PAGE_IMAGE, \a DATA_ITEM_DATASET if it is a \a DATASET. Returns DATA_ITEM_UNKNOWN otherwise
		 * 
		 * \todo This function is DAE Data Model dependant and should be moved to a more specific file.
		 */
		function getDataItemType($id)
		{
			if(is_numeric($id))
			{
				if($this->r("SELECT ID FROM PAGE_IMAGE WHERE ID = $id",array()))
					return DATA_ITEM_PAGE_IMAGE;
				else if($this->r("SELECT ID FROM DATASET WHERE ID = $id",array()))
					return DATA_ITEM_DATASET;
				else if($this->r("SELECT ID FROM PAGE_ELEMENT where ID = $id",array()))
					return DATA_ITEM_PAGE_ELEMENT;
				else if($this->r("SELECT ID FROM FILES where ID = $id",array()))
					return DATA_ITEM_FILE;
			}
			return DATA_ITEM_UNKNOWN;
		}
		
		/**
		 * \brief SQL query returning the name of an algorithm with a given id.
		 *  
		 * @param [in] $id Database id number from \a algorithm table.
		 * @return name of \a $id in \a algorithm table or \a false if query did not succeed.
		 * 
		 * \todo This function is DAE Data Model dependant and should be moved to a more specific file.
		 */		
		function getAlgName($id)
		{
			if(is_numeric($id))
			{
				$r = $this->q("select NAME from ALGORITHM where ID = ". $id,array());
				$row = $this->fetch_array($r,DATABASE_NUM);
				return $row[0];
			}
			return false;

		}
		
		/**
		 * \brief SQL query checking whether a given algorithm id exists.
		 *  
		 * @param [in] $id Database id number supposedly from \a algorithm table.
		 * @return \a true if an algorithm with the given \a $id exists, \a false otherwise.
		 * 
		 * \todo This function is DAE Data Model dependant and should be moved to a more specific file.
		 */
		function checkAlgExists($id)
		{
		    if(is_numeric($id)) {
			    $r = $this->q("select count(ID) from ALGORITHM where ID = ". $id,array());
			    $row = $this->fetch_array($r,DATABASE_NUM);
			    return ($row[0] > 0);
		    }
		    else return false;
		}
		
		/**
		 * \brief Checks whether the current Drupal user is under his/her quota of allowed concerrent algorithm runs
		 *  
		 * @return \a true if the current Drupal user can launch at least one more algorithm run, \a false otherwise.
		 * 
		 * \todo This function is DAE Data Model dependant and should be moved to a more specific file.
		 */
		function checkConcurrentRuns()
		{
			global $user;
			
			if(user_access('unlimited_runs'))
				return true;
			else
			{
				$r = $this->q("select count(*) from (select ALGORITHM_RUN_ID from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID where END_TIME is null and PERSON_ID = ". getPID($user->uid) . " ) ",array());
				$row = $this->fetch_array($r,DATABASE_NUM);
				return($row[0] <= variable_get('algorithms_concurrent_runs', ''));
			}
		}
		
		/**
		 * Checks whether a Dataset contains sub-datasets (children).
		 * 
		 * @param [in] $id Dataset database id
		 * @return \a true if the dataset with given id is associated to another dataset 
		 *         through the \a ASSOCIATE_DATASET table, \a false otherwise
		 *         
		 * \bug This function ignores the \a TYPE field of the \a ASSOCIATE_DATASET table, making the assumption that all associations are one-way inclusions. 
		 */
		function checkDataSetHasChildren($id)
		{
		    if(is_numeric($id)) {
			    $r = $this->q("select count(*) from DATASET,ASSOCIATE_DATASET where ASSOCIATING_DATASET_ID=id and DATASET_id=" . $id,array());
			    $row = $this->fetch_array($r,OCI_NUM);
			    return($row[0]);
		    }
		    else return 0;
		}
		
		
		function runElapsedTime($run)
		{
			global $user;
			settype($run, 'integer');
			
			$query = "select " . $this->sysdateString . "-START_TIME as ELAPSED, " . 
					$this->dateFormat('START_TIME','MON DD, YYYY') . " as STARTDAY, ". 
					$this->dateFormat('START_TIME','HH:MI:SS AM') . " as STARTTIME, " .
					$this->dateFormat('SYSDATE','MON DD, YYYY') . " as CURDAY, " . 
					$this->dateFormat('SYSDATE','HH:MI:SS AM') . " as CURTIME
				from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
				where PERSON_ID=" .getPID($user->uid). " and ALGORITHM_RUN.ID=".$run;
			
			$result = $this->q($query,array());
			$row = $this->fetch_array($result,DATABASE_BOTH);
	
			/**
			 * \bug The following code does not give correct results with MySQL
			 */
			$diff = explode(" ", $row['ELAPSED']);
			$time = explode(":",$diff[1]);
			
			return array("days" => $diff[0], "time" => $time,'start_time' => $row['STARTTIME'],'start_day' => $row['STARTDAY'], 'cur_day' => $row['CURDAY'], 'cur_time' => $row['CURTIME'] );
		}
		
		
		function runLength($run)
		{
			global $user;
			settype($run, 'integer');
			
			$query = "select END_TIME-START_TIME as ELAPSED, " .
				$this->dateFormat('END_TIME','MON DD, YYYY') . " as ENDDAY, " .
				$this->dateFormat('START_TIME','MON DD, YYYY') . " as STARTDAY, " .
				$this->dateFormat('END_TIME','HH:MI:SS AM') . " as ENDTIME, " .
				$this->dateFormat('START_TIME','HH:MI:SS AM') . " as STARTTIME 
			from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
			where PERSON_ID=" .getPID($user->uid). " and ALGORITHM_RUN.ID=".$run;
			
			$result = $this->q($query,array());
			$row = $this->fetch_array($result,DATABASE_BOTH);

			$diff = explode(" ", $row['ELAPSED']);
			$time = explode(":",$diff[1]);
			
			return array(
			'start_time' => $row['STARTTIME'], 
			'days' => $diff[0], 
			'time' => $time, 
			'start_day' => $row['STARTDAY'],  
			'end_day' => $row['ENDDAY'], 
			'end_time' => $row['ENDTIME']
			);
	
		}		
}

class DAEMySQLDatabase extends DAEDatabase
{
		function __construct()
		{
			parent::__construct();
			
			parent::$this->dbmsType = 'MySQL';
			

			define(DATABASE_NUM, MYSQL_NUM);
			define(DATABASE_BOTH, MYSQL_BOTH);
			
			parent::$this->sysdateString = 'SYSDATE()';
			parent::$this->dateConversionString = 'DATE_FORMAT';
			parent::$this->timestampFormat = 'Y-m-d H:i:s';

			parent::$this->conn = mysql_connect(parent::$this->host,parent::$this->user,parent::$this->pass);
			if (!parent::$this->conn)
  			{
  				$e = mysql_error();
				databaseDebugMessage(print_r($e,true));
  			}
  			
			mysql_select_db(variable_get('dae_db_name',''), parent::$this->conn);
			
			parent::$this->safe_conn = mysql_connect(parent::$this->host,parent::$this->ro_user,parent::$this->ro_pass);
			if (!parent::$this->safe_conn)
  			{
  				$e = mysql_error();
				databaseDebugMessage(print_r($e,true));
  			}
  			
		}

		function q($query,$variables = -1)
		{
		    if($variables != -1) {
		        if(! is_array($variables))
		            drupal_set_message('Wrong variables argument in MySQL daedatabase q(): '.print_r($variables,true),error);
		        else 
		            foreach ($variables as $key => $value) {
		    /**
		     * @todo Substitute variables in query with appropriate mysql_escape_string calls
		     */
		                                        $query = str_replace($key,$value,$query);
                    }    
		    }
		    else {
		        databaseDebugMessage("MySQL daedatabase q() function called without substitution variables !");
		    }
		    
		    return(mysql_query($query));
		}

		function fetch_array($r,$mode=DATABASE_BOTH)
		{
			if(!$r) databaseDebugMessage(print_r($r,true));
			$rvalue = mysql_fetch_array($r,$mode);
			if(!$rvalue) databaseDebugMessage("Debug call stack",true);
			return(mysql_fetch_array($r,$mode));
		}

		function createUniqueEntry($table,$field)
		{
				$q = parent::$this->q("INSERT INTO $table ($field) values (null)");
				return mysql_insert_id(parent::$this->conn); 
		}
		
		function __destruct()
		{
			if(parent::$this->conn != null) {
				if(!mysql_close(parent::$this->conn))
				{
					//! \todo Log this error more appropriately
				}
				parent::$this->conn = null;
			}
		}

		
		/**
		 * Conversion from Oracle type date formatting strings to MySQL date formatting strings
		 * 
		 * @param [in] $formatstring the Oracle format string
		 * 
		 * @returns the equivalent (or as close as possible) MySQL format string
		 **/
		function convertDateFormat($formatstring)
		{
			$patterns = array();
			$replacements = array();
			$patterns[] = '/DY/';	$replacements[] = '%a';
			$patterns[] = '/MON/';	$replacements[] = '%b';
			$patterns[] = '/DDD/';	$replacements[] = '%j';
			$patterns[] = '/DD/';	$replacements[] = '%e';
			$patterns[] = '/MM/';	$replacements[] = '%c';
			$patterns[] = '/MONTH/';	$replacements[] = '%M';
			$patterns[] = '/MI/';	$replacements[] = '%i';
			$patterns[] = '/HH12/';	$replacements[] = '%l';
			$patterns[] = '/HH24/';	$replacements[] = '%k';
			$patterns[] = '/HH/';	$replacements[] = '%l';
			$patterns[] = '/AM/';	$replacements[] = '%p';
			$patterns[] = '/PM/';	$replacements[] = '%p';
			$patterns[] = '/SS/';	$replacements[] = '%s';
			$patterns[] = '/WW/';	$replacements[] = '%u';
			$patterns[] = '/IW/';	$replacements[] = '%u';
			$patterns[] = '/DAY/';	$replacements[] = '%W';
			$patterns[] = '/SYYYY/';	$replacements[] = '%Y';
			$patterns[] = '/YYYY/';	$replacements[] = '%Y';
			$patterns[] = '/IYYY/';	$replacements[] = '%Y';
			$patterns[] = '/IYY/';	$replacements[] = '%y';
			$patterns[] = '/YY/';	$replacements[] = '%y';
			
			/*
			$patterns[] = '/J/';	$replacements[] = '';
			$patterns[] = '/Q/';	$replacements[] = '';
			$patterns[] = '/RRRR/';	$replacements[] = '';
			$patterns[] = '/RR/';	$replacements[] = '';
			$patterns[] = '/W/';	$replacements[] = '';
			$patterns[] = '/SSSSS/';	$replacements[]= '';
			$patterns[] = '/X/';	$replacements[] = '';
			$patterns[] = '/Y,YYY/';	$replacements[] = '';
			$patterns[] = '/YEAR/';	$replacements[] = '';
			$patterns[] = '/SYEAR/';	$replacements[] = '';
			$patterns[] = '/YYY/';	$replacements[] = '';
			$patterns[] = '/IY/';	$replacements[] = '';
			$patterns[] = '/I/';	$replacements[] = '';
			$patterns[] = '/AD/';	$replacements[] = '';
			$patterns[] = '/A\.D\./';	$replacements[] = '';
			$patterns[] = '/BC/';	$replacements[] = '';
			$patterns[] = '/B\.C\./';	$replacements[] = '';
			$patterns[] = '/SCC/';	$replacements[] = '';
			$patterns[] = '/CC/';	$replacements[] = '';
			$patterns[] = '/D/';	$replacements[] = '';
			$patterns[] = '/A\.M\./';	$replacements[] = '';
			$patterns[] = '/P\.M\./';	$replacements[] = '';
			$patterns[] = '/TZH/';	$replacements[] = '';
			$patterns[] = '/TZM/';	$replacements[] = '';
			$patterns[] = '/TZR/';	$replacements[] = '';
			$patterns[] = '/RM/';	$replacements[] = '';
			*/
			
			$convertedstring = preg_replace($patterns, $replacements, $formatstring);
			return '\'' . $convertedstring . '\'';
		}
		
		/**
		 * \brief Overloaded call to a MYSQL specific version returning the number of fields returned by a query
		 * 
		 * @param [in] $query the query that was issued (not the SQL string, but the MySQL-specific 'ressource')
		 * @return the number of fields returned by the query
		 */
		function num_fields($query)
		{
			return mysql_num_fields($query);	
		}
		
		/**
		 * \brief Overloaded call to a MYSQL specific version returning the name of a field returned by a query
		 * 
		 * Fields are numbered starting from 0.
		 * 
		 * @param [in] $query the query that was issued (not the SQL string, but the MySQL-specific 'ressource')
		 * @param [in] $i index of the column the field name is to be returned
		 * @return the name of field number \a $i in query \a $query.
		 */
		function field_name($query, $i)
		{
			return mysql_field_name($query,$i);
		}
		
		/**
		 * \brief Overloaded call to a MYSQL specific version returning the current row of a query result
		 * 		
		 * Each call positions the current row to the next row.

		 * @param [in] $query the query that was issued (not the SQL string, but the MySQL-specific 'ressource')
		 * @return the current row of the query result
		 */
		function fetch_row($query)
		{
			return mysql_fetch_row($query);		
		}
		
}

class DAEOracleDatabase extends DAEDatabase
{
		function __construct()
		{
			parent::__construct();
			parent::$this->dbmsType = 'Oracle';
			
			define(DATABASE_NUM, OCI_NUM);
			define(DATABASE_BOTH, OCI_BOTH);

			parent::$this->systemdate = 'SYSDATE';
			parent::$this->timestampFormat = 'd-M-Y h:i:s A';
			
			$sid = variable_get('dae_db_name','');
			putenv("ORACLE_SID=".$sid); 
			
			if(parent::$this->ro_user != '') {
		        parent::$this->safe_conn = oci_connect(parent::$this->ro_user,parent::$this->ro_pass,parent::$this->host . '/' . $sid);
			    if (!$this->safe_conn) 
			    {
   			 	    $e = oci_error();
  			 	    databaseDebugMessage(print_r($e,true));
			    }
			}
			else {
			    $this->safe_conn = null;
			    databaseDebugMessage('Setting safe connexion to null, no ro_user defined');
			}
			
			parent::$this->conn = oci_connect(parent::$this->user,parent::$this->pass,parent::$this->host . '/' . $sid);
			if (!$this->conn) 
			{
   			 	$e = oci_error();
  			 	databaseDebugMessage(print_r($e,true));
			}
			
			///databaseDebugMessage("Oracle Database created " . print_r($this,true));
		}

		function fetch_array($r,$mode=DATABASE_BOTH)
		{
			return(oci_fetch_array($r,$mode));
		}

		function q($query,$variables = -1)
		{
		    $q= oci_parse(parent::$this->conn,$query);

		    /* Substituting variables in query for avoiding SQL code injection */
		    if($variables != -1) {
		        if(! is_array($variables))
		            drupal_set_message('Wrong variables argument in Oracle daedatabase q(): '.print_r($variables,true),error);
		        else 
		            foreach ($variables as $key => $value) {
		                /*
		                 * ATTENTION. If $key (preceded with ':') is a reserved SQL keyword binding will not fail.
		                 * oci_excecute() will fail later, however.
		                 */
                        if (!oci_bind_by_name($q, $key, $variables[$key])) { // This is a necessary construct !!! cannot bind to $value.
		                    $e = oci_error();
  			 	            databaseDebugMessage('OCI bind failed !' . print_r($e,true) . ' ' . $query . ' ' . print_r($variables,true));
                        }
                    }    
		    }
		    else {
		        databaseDebugMessage("Oracle daedatabase q() function called without substitution variables ! ".$query, true);
		        error_log(print_r(debug_backtrace(),true));
		    }

			if (!oci_execute($q))
				databaseDebugMessage('OCI execution failed on: ' . print_r(oci_error(),true) . ' ' . $query. ' ' .  print_r($variables,true));
				
  			return($q);
		}
		
		function u($q,$variables = -1)
		{
		    $q= oci_parse(parent::$this->conn,$query);

		    /* Substituting variables in query for avoiding SQL code injection */
		    if($variables != -1) {
		            foreach ($variables as $key => $value) {
                        if (!oci_bind_by_name($q, $key, $variables[$key])) { // This is a necessary construct !!! cannot bind to $value.
		                    $e = oci_error();
  			 	            databaseDebugMessage(print_r($e,true) . ' ' . $query . ' ' . print_r($variables,true));
                        }
                    }    
		    }
		    else {
		        databaseDebugMessage("Oracle daedatabase u() function called without substitution variables ! ".$query, true);
		        error_log(print_r(debug_backtrace(),true));
		    }

		    if (!oci_execute($q))
				databaseDebugMessage(print_r($e,true) . ' ' . $query . ' ' . print_r($variables,true));
				
			return oci_num_rows($q);
		}
		function searchTable($return, $search, $field, $select_fields, $from_table,$page_start = 0, $page_end = 10, $wildcard = array('start' => false, 'end' => true), $case_sensitive = false)
		{
			$fields = implode(", ",$select_fields);
	
			if($page_end > 0) {
			    $variables[':pagestart'] = $page_start;
			    $variables[':pageend'] = $page_end;
			    $paged = 'WHERE r > :pagestart and r < :pageend';
			}
				
			//$variables[':selectfield'] = $fields;
			//$variables[':fromtable'] = $from_table;
			$variables[':fieldname'] = $field;
			
			if($case_sensitive)
				$where = "WHERE :fieldname like :searchterm";
			else
				$where = "WHERE lower(:fieldname) like lower(:searchterm)";
			
			if($wildcard['start'] && $wildcard['end']) {
			    $variables[':searchterm'] = '%'.$search.'%';
			} elseif(!$wildcard['start'] && $wildcard['end']) {
			    $variables[':searchterm'] = $search.'%';
			} elseif($wildcard['start'] && !$wildcard['end']) {
				$variables[':searchterm'] = '%'.$search;
			} else {
				$variables[':searchterm'] = $search;
			}
			
			
			if($return == 'result')
			{
				$query = "SELECT *
						  FROM(
							  SELECT $fields, ROW_NUMBER() over(ORDER BY ID) as r  
							  FROM $from_table 
						  	  $where
						  )
						  $paged
					  ";
			
				return $this->q($query,$variables);
			}
			elseif($return == 'count')
			{
				$query = "SELECT count(*)
					  	  FROM $from_table
						  $where
					  	  ";
			
				return $this->r($query,$variables);
			}
		}
		function createUniqueEntry($table,$field)
		{
		    $variables = array();
		    /*
		    $variables[':table'] = $table;
			$variables[':field'] = $field;
			*/
		    
            $nextval = 'SEQ_' . $table . '.NEXTVAL';
            $curval = 'SEQ_' . $table . '.CURRVAL';
			
            // For some reason variable binding does not work here !!
            /** \todo Check if this has security impact **/
			parent::$this->i("INSERT INTO $table ($field) VALUES($nextval)",$variables);
			
			$uniqueID = parent::$this->r("SELECT $curval FROM DUAL",array());
			
			return $uniqueID; 
		}
		
		function __destruct()
		{
			oci_close($this->conn);
		}
		
				/**
		 * \brief Overloaded call to a OCI8 specific version returning the number of fields returned by a query
		 * 
		 * @param [in] $query the query that was issued (not the SQL string, but the OCI8-specific 'statement')
		 * @return the number of fields returned by the query
		 */
		function num_fields($query)
		{
			return oci_num_fields($query);	
		}
		
		/**
		 * \brief Overloaded call to a OCI8 specific version returning the name of a field returned by a query
		 * 
		 * Fields are numbered starting from 0.
		 * 
		 * @param [in] $query the query that was issued (not the SQL string, but the OCI8-specific 'statement')
		 * @param [in] $i index of the column the field name is to be returned
		 * @return the name of field number \a $i in query \a $query.
		 */
		function field_name($query, $i)
		{
			return oci_field_name($query,$i+1);
		}
		
		/**
		 * \brief Overloaded call to a OCI8 specific version returning the current row of a query result
		 * 		
		 * Each call positions the current row to the next row.

		 * @param [in] $query the query that was issued (not the SQL string, but the OCI8-specific 'statement')
		 * @return the current row of the query result
		 */
		function fetch_row($query)
		{
			return oci_fetch_row($query);		
		}
		
}

/**
 * \brief Generates an array representing the signature of the algorithm the ID of which
 * was specified in \a $algoId.
 * 
 * @param [in] $algoId
 * 
 * @returns an associative array containing the fields \a NAME, \a VERSION and 
 * \a DESCRIPTION of algorithm \a $algoId. 
 */
function retrieveAlgortithmInfo($algoId)
{
    settype($algoId, 'integer');
    $db = getDAEDatabase();

    return $db->fetch_array($db->q("select NAME, VERSION, DESCRIPTION, PATH from ALGORITHM where ID = $algoId",array()));
}

function retrieveAlgorithmSOAPOutputTypes($algoId,$prefix="#")
{
   $db = getDAEDatabase();
   $SOAPOutputType = array();

   settype($algoId, 'integer');
   $algoOutput = $db->q("select
                                DATA.ID as DID, DATATYPE.ID as TID, DATA.NAME as DNAME, DATATYPE.NAME as TNAME
                                from 
                                DATA, TYPE_OF, DATATYPE, ALGORITHM_OUTPUT ARG 
                        where 
                                DATA.ID = ARG.DATA_ID
                          and 
                                DATA.ID = TYPE_OF.DATA_ID
                          and 
                                DATATYPE.ID = TYPE_OF.DATATYPE_ID
                          and 
                                ARG.ALGORITHM_ID = $algoId",array());
    
    while($row = $db->fetch_array($algoOutput)) {
        // In the parameters, all spaces are replaced with "_".
        $DNAME = str_replace(array(' ','/'), array('_','_'), $row['DNAME']);
        $TNAME = str_replace(array(' ','/'), array('_','_'), $row['TNAME']);
        $label = $DNAME.'_'.$row['DID'];
        $SOAPOutputType[$label] = array($prefix.'name'=>$label, $prefix.'type'=>'tns:dae_'.$TNAME.'_'.$row['TID']);
        //$SOAPOutputType[$label] = array($prefix.'name'=>$label, $prefix.'type'=>'xsd:string');
    }
    
    return $SOAPOutputType;
}

/**
 * \brief Generates an array containing the SOAP-compatible representation of 
 * all input parameters of the algorithm the ID of which was specified in \a $algoId.
 * 
 * @param [in] $algoId
 * @param [in] $prefix
 * 
 * @returns a NuSoap compatible array of input parameters for algorithm \a $algoId.
 */
function retrieveAlgorithmSOAPInputTypes($algoId,$prefix="",$nameSpace="",$numbering=false)
{
   $db = getDAEDatabase();
   $SOAPInputType = array();
   $i = 0;
   
   settype($algoId, 'integer');
   $typedAlgoInput = $db->q("
    				select
    	        		DATA.ID as DID, DATATYPE.ID as TID, DATA.NAME as DNAME, DATATYPE.NAME as TNAME
        	  		from 
            			DATA, TYPE_OF, DATATYPE, ALGORITHM_INPUT ARG
              		where 
              			DATA.ID = ARG.DATA_ID 
              		  and 
              			DATA.ID = TYPE_OF.DATA_ID
              		  and 
              		  	DATATYPE.ID = TYPE_OF.DATATYPE_ID
              		  and 
              		  	ARG.ALGORITHM_ID = $algoId",array());
     
    while($row = $db->fetch_array($typedAlgoInput)) {
        // In the parameters, all spaces are replaced with "_".
        $DNAME = str_replace(array(' ','/'), array('_','_'), $row['DNAME']);
        $TNAME = str_replace(array(' ','/'), array('_','_'), $row['TNAME']);
        $label = $DNAME.'_'.$row['DID'];
        $SOAPInputType[($numbering) ? $i : $label] = array($prefix.'name' => $DNAME.'_'.$row['DID'], $prefix.'type' => ($nameSpace ? 'tns:' : '').'dae_'.$TNAME.'_'.$row['TID']);
        //$SOAPInputType[($numbering) ? $i : $label] = array($prefix.'name' => $label, $prefix.'type' => $nameSpace.'string');
        $i++;      
    }
    
   
   $unTypedAlgoInput = $db->q("
    				select
    	        		DATA.ID as DID, DATA.NAME as DNAME
        	  		from 
            			DATA, ALGORITHM_INPUT ARG
              		where 
              			DATA.ID = ARG.DATA_ID
              		  and 
              		  	ARG.ALGORITHM_ID = $algoId
                      and 
              		    DATA.ID not in (select DATA_ID from TYPE_OF)",array());
              		  
    while($row = $db->fetch_array($unTypedAlgoInput)) {
        // In the parameters, all spaces are replaced with "_".
        $DNAME = str_replace(array(' ','/'), array('_','_'), $row['DNAME']);
        $label = $DNAME.'_'.$row['DID'];
        $SOAPInputType[($numbering) ? $i : $label] = array($prefix.'name' => $label, $prefix.'type' => $nameSpace.'string');
        $i++;      
    }

    return $SOAPInputType;
}

/**
 * \brief Runtime debug message handler.
 * 
 * This function can be used un a running server without polluting the regular users. It is controlled by the 'debug DAE Database'
 * Drupal user access policy.
 * 
 * @param $msg Message to be displayed
 * @param $stacktrace flag determining whether to dump the call stack as well
 */
function databaseDebugMessage($msg, $stacktrace = false) 
{
    error_log($msg);
    
	if(user_access('debug DAE Database')) 
	{
		drupal_set_message($msg,warning);
		watchdog('Debug',$msg);
		if($stacktrace)
			drupal_set_message(print_r(debug_backtrace(),true),warning);
	}
}