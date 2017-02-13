package edu.lehigh.dae.utility.upload.oracle;

import java.io.BufferedReader;
import java.io.FileReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.Statement;
import java.util.Hashtable;
import java.util.Vector;

import edu.lehigh.dae.parsemetadata.newformat.oracle.Parse;

public class OracleDB {
	private Connection connection;

	private static String HOST;
	private static String PORT;
	private static String SID;
	private static String USERNAME;
	private static String PASSWORD;

	Hashtable<String, ResultSetMetaData> metadataHashtable = 
		new Hashtable<String, ResultSetMetaData>();

	Statement stmt = null;
	ResultSet rs = null;

	public OracleDB() {
		try {
			BufferedReader br = new BufferedReader(new FileReader("config.txt"));
			String line = "";
			if((line = br.readLine()) != null) {
				String[] split = line.split("#");
				OracleDB.HOST = split[0];
				OracleDB.PORT = split[1];
				OracleDB.setSID(split[2]);
				OracleDB.setUSERNAME(split[3]);
				OracleDB.setPASSWORD(split[4]);
			} else {
				System.err.println("no db configuration provided, exit program.");
				System.exit(0);
			}
		} catch (Exception ex) {
			//ex.printStackTrace();
			System.err.println("no db configuration provided, exit program.");
			System.exit(0);
		}
	}
	
	public static void main(String args[]) {
		OracleDB db = new OracleDB();
		boolean connected = db.connect("dae0", OracleDB.getUSERNAME(), OracleDB.getPASSWORD());
		System.out.println("connection status: "+connected);
		db.disconnect();
	}

	public ResultSet doSelect(Statement stmt, String sqlCommand) {
		ResultSet rs = null;
		try {
			rs = stmt.executeQuery(sqlCommand);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		return rs;
	}

	public boolean doUpdate(OracleDB db, String sqlCommand) {
		try {
			Statement stmt = db.getConnection().createStatement(
					ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_UPDATABLE);
			stmt.executeUpdate(sqlCommand);
			stmt.close();
		} catch (Exception ex) {
			ex.printStackTrace();
			return false;
		}
		return true;
	}

	public boolean connect(String sid, String username, String password) {
		try { 
			String driverName = "oracle.jdbc.driver.OracleDriver";
			Class.forName(driverName);
			String url = "jdbc:oracle:thin:@" + OracleDB.HOST + ":" + OracleDB.PORT + ":" + sid;  
			connection = DriverManager.getConnection(url, username, password);
			stmt = this.getConnection().createStatement();
		}
		catch (Exception ex) {
			ex.printStackTrace();
			return false;
		}
		return true;
	}

	public boolean disconnect() {
		try {
			if(this.connection!=null && !this.connection.isClosed()) {
				this.connection.close();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			return false;
		}
		return true;
	}

	public Connection getConnection() {
		return this.connection;
	}

	public String composeSQLQuery(
			OracleDB db, 
			Vector<String> columns, 
			Vector<String> values,
			String entityType) {
		// TODO Auto-generated method stub
		//compose columns part
		String sqlCommand = "insert into "+entityType+" ( ";
		for (int i=0;i<columns.size();i++) {
			if(i!=0) {
				sqlCommand += ", ";
			}
			sqlCommand += columns.elementAt(i);
		}		
		sqlCommand += " ) values ( ";

		//compose the values part
		for (int i=0;i<values.size();i++) {
			boolean strValue = false;
			strValue = this.isStrValue(db, columns.elementAt(i), entityType);
			if(strValue) {
				sqlCommand += "'";
			}
			sqlCommand += values.elementAt(i);
			if(strValue) {
				sqlCommand += "'";
			}
			if(i!=values.size()-1) {
				sqlCommand += ", ";
			}
		}
		sqlCommand += " )";

		return sqlCommand;
	}

	public String composeUpdateSQLQuery(
			OracleDB db, 
			Vector<String> columns, 
			Vector<String> values,
			Vector<String> conditionColumns,
			Vector<String> conditionValues,
			String entityType) {
		// TODO Auto-generated method stub
		//compose columns part
		String sqlCommand = "update "+entityType+" set ";
		for (int i=0;i<columns.size();i++) {
			boolean strValue = false;
			strValue = this.isStrValue(db, columns.elementAt(i), entityType);
			String value = "";
			if(strValue) {
				value = "'"+values.elementAt(i)+"'";
			} else {
				value = values.elementAt(i);
			}
			sqlCommand += columns.elementAt(i)+"="+value;
			if(i!=columns.size()-1) {
				sqlCommand += ", ";
			}
		}		
		sqlCommand += " where  ";

		//compose the values part
		for (int i=0;i<conditionColumns.size();i++) {
			boolean strValue = false;
			strValue = this.isStrValue(db, conditionColumns.elementAt(i), entityType);
			String value = "";
			if(strValue) {
				value = "'"+conditionValues.elementAt(i)+"'";
			} else {
				value = conditionValues.elementAt(i);
			}
			sqlCommand += conditionColumns.elementAt(i)+"="+value;
			if(i!=conditionColumns.size()-1) {
				sqlCommand += "and";
			}
		}

		return sqlCommand;
	}
	
	private boolean isStrValue(OracleDB db, String columnName, String tableName) {
		// TODO Auto-generated method stub
		try {
			//retrieve the metadata for table entityType if it does not exist now
			if(!this.metadataHashtable.containsKey(tableName)) {
				String sqlCommand = "select * from "+tableName+" where rownum=1";
				System.out.println(sqlCommand);
				stmt = db.getConnection().createStatement();
				rs = this.doSelect(stmt, sqlCommand);
				this.metadataHashtable.put(tableName, rs.getMetaData());
			}

			String columnType = "";
			ResultSetMetaData rsMetadata = this.metadataHashtable.get(tableName);
			if(rsMetadata==null) {
				System.out.println("null");
			}
			//get the datatype for the current column
			for (int i=1;i<=rsMetadata.getColumnCount();i++) {
				if(rsMetadata.getColumnName(i).equalsIgnoreCase(columnName)) {
					columnType = rsMetadata.getColumnTypeName(i);
					break;
				}
			}

			//the column type has been retrieved; now needs to decide if we should add 
			//single quote to its value when composing a SQL query
			if(Parse.stringColumnTypes.contains(columnType) ||
					Parse.stringColumnTypes.contains(columnType.toLowerCase())) {
				return true;
			}
		} catch (Exception ex) {
			System.err.println("Error occurred in retrieving "+tableName+" table metadata");
			ex.printStackTrace();
			System.exit(0);
		}
		return false;
	}

	public int doSelect(String sqlCommand) {
		int id = -1;
		try {
			PreparedStatement pstmt = this.getConnection().prepareStatement(sqlCommand, new String[] { "id" });
			pstmt.executeUpdate();
			ResultSet rs = pstmt.getGeneratedKeys();
			if (rs.next()) {
				id = rs.getInt(1);
			}
			pstmt.close();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		return id;
	}

	public static void setPASSWORD(String pASSWORD) {
		PASSWORD = pASSWORD;
	}

	public static String getPASSWORD() {
		return PASSWORD;
	}

	public static void setUSERNAME(String uSERNAME) {
		USERNAME = uSERNAME;
	}

	public static String getUSERNAME() {
		return USERNAME;
	}

	public static void setSID(String sID) {
		SID = sID;
	}

	public static String getSID() {
		return SID;
	}
}