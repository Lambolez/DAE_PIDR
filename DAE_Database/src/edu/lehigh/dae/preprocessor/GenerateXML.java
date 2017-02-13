package edu.lehigh.dae.preprocessor;

import java.sql.*;

import edu.lehigh.dae.utility.upload.oracle.OracleDB;

import oracle.xml.sql.dml.OracleXMLSave;
import oracle.xml.sql.query.OracleXMLQuery;

/**This class is used to accept requests from pre-process and send
 * pre-processed metadata file to database for parsing.*/
public class GenerateXML {
	public static void main(String[] args) {
		//establish database connection
		OracleDB db = new OracleDB();
		db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(),OracleDB.getPASSWORD());
		Connection conn  = db.getConnection();

		//declare a new instance.
		GenerateXML xsu = new GenerateXML();

		//call insert function to send a XML file to database.
		xsu.insert(conn, "file", "file_view", args[0]);

		//disconnect from database.
		db.disconnect();
	}

	/**This method shows how to query the database and export
	 * metadata to the DAE XML format.*/
	public void query(Connection conn) {
		try {
			OracleXMLQuery qry = new OracleXMLQuery(conn, 
					" select p.* from page_image_view p, associate_datatype_data_item a" +
					" where p.id=a.data_item_id and a.datatype_id=1 and rownum<2");
			qry.setRowTag("page_image");
			System.out.println(qry.getXMLString());
			qry.close();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	/**This method send a XML file to database and call the INSTEAD-OF trigger
	 * designed for tables/views.
	 * @conn - the database connection
	 * @rowTag - the type of metadata included in the XML file, such as page_image, file, etc
	 * @xmlPath - the path of the XML file with metadata*/
	public boolean insert(Connection conn, String rowTag, String viewTable, String xmlPath) {
		try {
			//declare a OracleXMLSave instance with the database connection and 
			//the view/table name to which metadata will be inserted.
			OracleXMLSave sav = new OracleXMLSave(conn, viewTable);
			//let the database ignore case when parsing and inserting.
			sav.setIgnoreCase(true);
			//let the database know the metadata type so that it will look for 
			//such type of metadata in the provided XML file.
			sav.setRowTag(rowTag);
			//do the insert
			sav.insertXML(OracleXMLSave.getURL(xmlPath));
			System.out.println(xmlPath+" saved");
			return true;
		} catch (Exception ex) {
			ex.printStackTrace();
			return false;
		}
	}
	
	/**This method send a XML file to database and call the INSTEAD-OF trigger
	 * designed for tables/views.
	 * @conn - the database connection
	 * @rowTag - the type of metadata included in the XML file, such as page_image, file, etc
	 * @xmlPath - the path of the XML file with metadata*/
	public boolean update(Connection conn, String rowTag, String viewTable, String xmlPath) {
		try {
			//declare a OracleXMLSave instance with the database connection and 
			//the view/table name to which metadata will be inserted.
			OracleXMLSave sav = new OracleXMLSave(conn, viewTable);
			//let the database ignore case when parsing and inserting.
			sav.setIgnoreCase(true);
			//let the database know the metadata type so that it will look for 
			//such type of metadata in the provided XML file.
			sav.setRowTag(rowTag);
			//do the insert
			String[] tempArray = new String[1];
			tempArray[0] = "ID";
			/*
			String[]tempArray = new String[1];
			tempArray[0] =
			    "IMAGE_DATASET_LIST";
			tempArray[1] =
			    "PAGE_ELEMENT_LIST";
			tempArray[2] =
			    "TYPE_LIST";*/
			sav.setKeyColumnList(tempArray);
			sav.updateXML(OracleXMLSave.getURL(xmlPath));
			System.out.println(xmlPath+" saved");
			return true;
		} catch (Exception ex) {
			ex.printStackTrace();
			return false;
		}
	}
}