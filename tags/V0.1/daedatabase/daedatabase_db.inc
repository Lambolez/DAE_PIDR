<?php
// $Id:$

/**
 * @file
 * Provides the oracle and mysql database functions.
 */
  
/**
 * 
 */ 

function getDAEDatabase()
{
	if(variable_get('dae_db_application',1) == 1)
		return(new DAEMySQLDatabase());
	else
		return(new DAEOracleDatabase());

}

class DAEDatabase
{
		protected $conn;
		protected $user;
		protected $pass;
		protected $host;
		
		//! string representing the database system date variable or function
		var $defaultSysdateString = 'SYSDATE';
		var $sysdateString = 'SYSDATE';
		var $dateConversionString = 'to_char';

		function __construct()
		{
		   $this->user = variable_get('dae_db_user','');
		   $this->pass = variable_get('dae_db_pass','');
		   $this->host = variable_get('dae_db_server','');		
		}

/*
 		function fetch_array($r)
		{}

		function q($query)
		{}
		function runElapsedTime($run)
		{}
		function runLength($run)
		{}
*/
		function r($query)//for getting a single unique value from a query
		{
			$result = $this->q($query);
			$row = $this->fetch_array($result);
			return $row[0];
		}		
		
		/**
		 * Converts a date formatting string from the standard Oracle syntax to the effecively
		 * used DBMS syntax. This function is supposed to be overloaded by inheriting classes.
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
		 * The query is supposed to be in the form of F($fields, $format) like \a to_char(SYSDATE,"MON DD").
		 * This function will replace
		 * 	- the standard conversion function name by the overridden function name
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
		
		function getAlgName($id)
		{
			if(is_int((int)$id))
			{
				$r = $this->q("select NAME from ALGORITHM where ID = ". $id);
				$row = $this->fetch_array($r,DATABASE_NUM);
				return $row[0];
			}
			return false;

		}
		function checkAlgExists($id)
		{
			$r = $this->q("select count(ID) from ALGORITHM where ID = ". $id);
			$row = $this->fetch_array($r,DATABASE_NUM);
			return ($row[0] > 0);
		}
		function checkConcurrentRuns()
		{
			global $user;
			
			if(user_access('unlimited_runs'))
				return true;
			else
			{
				$r = $this->q("select count(*) from (select ALGORITHM_RUN_ID from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID where END_TIME is null and PERSON_ID = ". $user->uid . " ) ");
				$row = oci_fetch_array($r,OCI_NUM);
				return($row[0] <= variable_get('algorithms_concurrent_runs', ''));
			}
		}
		function runElapsedTime($run)
		{
			global $user;
			
			$query = "select " . $this->sysdateString . "-START_TIME as ELAPSED, " . 
					$this->dateFormat('START_TIME','MON DD, YYYY') . " as STARTDAY, ". 
					$this->dateFormat('START_TIME','HH:MI:SS AM') . " as STARTTIME, " .
					$this->dateFormat('SYSDATE','MON DD, YYYY') . " as CURDAY, " . 
					$this->dateFormat('SYSDATE','HH:MI:SS AM') . " as CURTIME
				from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
				where PERSON_ID=" .$user->uid. " and ALGORITHM_RUN.ID=".$run;
			
			$result = $this->q($query);
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

			$query = "select END_TIME-START_TIME as ELAPSED, " .
				$this->dateFormat('END_TIME','MON DD, YYYY') . " as ENDDAY, " .
				$this->dateFormat('START_TIME','MON DD, YYYY') . " as STARTDAY, " .
				$this->dateFormat('END_TIME','HH:MI:SS AM') . " as ENDTIME, " .
				$this->dateFormat('START_TIME','HH:MI:SS AM') . " as STARTTIME 
			from ALGORITHM_RUN join EXECUTES_RUN on ID = ALGORITHM_RUN_ID natural join ALGORITHM_RUN_OF join ALGORITHM on ALGORITHM.ID = ALGORITHM_ID 
			where PERSON_ID=" .$user->uid. " and ALGORITHM_RUN.ID=".$run;
			
			$result = $this->q($query);
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

			define(DATABASE_NUM, MYSQL_NUM);
			define(DATABASE_BOTH, MYSQL_BOTH);
			
			parent::$this->sysdateString = 'SYSDATE()';
			parent::$this->dateConversionString = 'DATE_FORMAT';

			parent::$this->conn = mysql_connect(parent::$this->host,parent::$this->user,parent::$this->pass);
			if (!parent::$this->conn)
  			{
  				$e = mysql_error();
				print_r($e);
  			}
  			
			mysql_select_db(variable_get('dae_db_name',''), parent::$this->conn);
		}

		function q($query)
		{
			return(mysql_query($query));
		}
		function i($query)
		{	
			$this->q($query);
		}
		function fetch_array($r,$mode=DATABASE_BOTH)
		{
			if(!$r) debug_print_backtrace();
			return(mysql_fetch_array($r,$mode));
		}

		function createUniqueEntry($table,$field)
		{
				$q = $this->q("INSERT INTO $table ($field) values (null)");
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
		
}

class DAEOracleDatabase extends DAEDatabase
{
		function __construct()
		{
			parent::__construct();

			define(DATABASE_NUM, OCI_NUM);
			define(DATABASE_BOTH, OCI_BOTH);

			parent::$this->systemdate = 'SYSDATE';
			
			$sid = variable_get('dae_db_name','');
			putenv("ORACLE_SID=".$sid); 
			parent::$this->conn = oci_connect(parent::$this->user,parent::$this->pass,parent::$this->host . '/' . $sid);
			if (!$this->conn) 
			{
   			 	$e = oci_error();
  			 	print_r($e);
			}
		}

		function fetch_array($r,$mode=DATABASE_BOTH)
		{
			return(oci_fetch_array($r,$mode));
		}
		function i($query)
		{	
			$this->q($query);
		}


		function q($query)
		{
			$q= oci_parse($this->conn,$query);
			oci_execute($q);
			return($q);
		}
		
		function createUniqueEntry($table,$field)
		{
				$this->i("INSERT INTO $table ($field) VALUES(SEQ_$table.NEXTVAL)");
				return $this->r("SELECT SEQ_$table.CURRVAL FROM DUAL"); 
		}
		
		function __destruct()
		{
			oci_close($this->conn);
		}
}
