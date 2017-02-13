package edu.lehigh.dae.utility.upload.oracle;

import java.sql.ResultSet;

public class Structure {
	public OracleDB db = null;
	
	public int personID = -1;
	
	public String uploadPath = "";
	
	ResultSet rsData = null;
	ResultSet rsMetadata = null;
	
	public Structure() {
		
	}

	public Structure(OracleDB db, int personID, ResultSet rsMetadata, ResultSet rsData, String uploadPath) {
		// TODO Auto-generated constructor stub
		this.db = db;
		
		this.personID = personID;
		this.uploadPath = uploadPath;
		
		this.rsMetadata = rsMetadata;
		this.rsData = rsData;
	}
}
