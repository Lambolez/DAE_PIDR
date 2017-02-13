package edu.lehigh.dae.utility.general;

import java.sql.ResultSet;
import java.sql.Statement;

import edu.lehigh.dae.utility.upload.mysql.MySqlDB;

public class ClearDB {
	public static void main(String args[]) {
		try {
			MySqlDB db = new MySqlDB("localhost", "DEZHAOTEST", MySqlDB.MYSQL_LOGIN, MySqlDB.MYSQL_PASSWORD);
			db.connect();
			Statement stmt = db.getConnection().createStatement();
			
			ResultSet rs = stmt.executeQuery("show tables");
			rs.beforeFirst();
			while(rs.next()) {
				String tablename = rs.getString(1);
				if(!tablename.equals("person")) {
					//clear the tables
					db.doUpdate(db, "delete from "+tablename);
				}
			}
			db.disconnect();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}
