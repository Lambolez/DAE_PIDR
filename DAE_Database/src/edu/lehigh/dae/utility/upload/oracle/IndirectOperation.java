package edu.lehigh.dae.utility.upload.oracle;

import java.io.File;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Vector;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.parsemetadata.newformat.oracle.Parse;

public class IndirectOperation {
	public static String translateParamStatus(int paramStatus) {
		// TODO Auto-generated method stub
		String status = "";
		switch (paramStatus) {
		case -1:
			status = "error in the format of the provided parameters.";
			break;
		case 1:
			status = "parameters parsed successfully";
		}
		return status;
	}

	public static int verifyCredential(String username, String password) {
		int personID = -1;
		OracleDB db = new OracleDB();
		if(db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD())) {
			Statement stmt = null;
			try {
				stmt = db.getConnection().createStatement(
						ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_UPDATABLE);
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String sqlCommand = "select associate_person_drupal_person.person_ID " +
			" from associate_person_drupal_person, drupal_person " +
			" where drupal_person.email='"+username+"' " +
			" and drupal_person.password='"+password+"' " +
			" and associate_person_drupal_person.email=drupal_person.email";
			//String sqlCommand = "select count(*) count from drupal_person where email='"+this.username+"' and password='"+this.password+"'";
			System.out.println(sqlCommand);

			try {
				ResultSet rs = db.doSelect(stmt, sqlCommand);
				if(!rs.next()) {
					personID =  -1;
				} else {
					personID = rs.getInt(1);
				}
			} catch (Exception ex) {
				//database execution error.
				db.disconnect();
				ex.printStackTrace();
				personID = -1;
			} finally {
				try {
					stmt.close();
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				db.disconnect();
				if(personID == -1) {
					System.out.println("unable to locate the personID with provided username and password.");
					System.exit(0);
				}
				System.out.println("credential verified\n");
			}
		}
		return personID;
	}

	public static String generateUploadDescription(String username, String type, String directory) {
		// TODO Auto-generated method stub
		String description = "";
		description = username+"-----"+type+"-----"+directory;
		System.out.println("upload description generated\n");
		return description;
	}

	synchronized public static int preserveUploadID(String type, String uploadDescription) {
		// TODO Auto-generated method stub
		int id = -1;
		OracleDB db = new OracleDB();
		try {
			if(db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD())) {
				String command = "insert into upload (id, time, type, description, status) " +
				"values (seq_upload.nextval, current_timestamp, "+Integer.valueOf(type)+", '"+uploadDescription+"', 0)";
				PreparedStatement pstmt = 
					db.getConnection().prepareStatement(command, new String[] { "id" });
				pstmt.executeUpdate();
				ResultSet rs = pstmt.getGeneratedKeys();
				if (rs.next()) {
					id = rs.getInt(1);
				}
				/*Statement stmt = db.getConnection().
				createStatement(ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_UPDATABLE);
				stmt.execute(command, Statement.RETURN_GENERATED_KEYS);

				command = "select seq_upload.currval as id from upload";
				ResultSet rs = db.getConnection().createStatement().executeQuery(command);
				if(!rs.next()) {
					id = rs.getInt("id");
				}*/
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			db.disconnect();
			return -1;
		} finally {
			db.disconnect();
			if(id == -1) {
				System.out.println("unable to preserve an upload ID.");
				System.exit(0);
			}
			System.out.println("upload ID preserved: "+id+"\n");
		}
		return id;
	}

	public static String makeDirectory(int personID, int uploadID) {
		// TODO Auto-generated method stub
		File file = null;
		try {
			String batchCommand = "/dae/database/dae0-upload/"+uploadID;
			file = new File(batchCommand);
			//recursively create the directory structure.
			file.mkdirs();
		} catch (Exception ex) {
			ex.printStackTrace();
			IndirectOperation.rollbackDataitem(uploadID);
		} finally {
			if(file == null) {
				System.out.println("making directory failed.");
				System.exit(0);
			}
			System.out.println("directory built: "+file.getAbsolutePath()+"\n");
		}
		return file.getAbsolutePath();
	}

	public static void linkPersonUpload(int personID, int uploadID) {
		// TODO Auto-generated method stub
		boolean status = false;
		OracleDB db = new OracleDB();
		try {
			db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD());
			String sqlCommand = "insert into has_upload (upload_ID, person_ID) " +
			"values ("+uploadID+", "+personID+")";
			status = db.doUpdate(db, sqlCommand);
		} catch (Exception ex) {
			ex.printStackTrace();
			IndirectOperation.rollbackDataitem(uploadID);
		} finally {
			db.disconnect();
			if(!status) {
				System.out.println("linking person and upload failed.");
				System.exit(0);
			}
			System.out.println("user and upload linked.\n");
		}
	}

	public static void copyFiles(String source, String dest, int uploadID) {
		// TODO Auto-generated method stub
		boolean status = false;
		try {
			Runtime runtime = Runtime.getRuntime();
			String command = "cp -r "+source+" "+dest;
			System.out.println(command);
			Process process = runtime.exec(command);
			process.waitFor();
			status = true;
		} catch (Exception ex) {
			ex.printStackTrace();
		} finally {
			if(!status) {
				System.out.println("copy files failed.");
				IndirectOperation.rollbackDataitem(uploadID);
				System.exit(0);
			}
			System.out.println("copy files done.\n");
		}
	}

	public static void rollbackDataitem(int uploadID) {
		// TODO Auto-generated method stub
		OracleDB db = new OracleDB();
		try {
			db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD());
			//2 represents "failing upload".
			String sqlCommand = "update upload set status = 2 where id="+uploadID;
			db.doUpdate(db, sqlCommand);
		} catch (Exception ex) {
			ex.printStackTrace();
		} finally {
			db.disconnect();
		}
	}

	public static void associateDataMetadataUploads(int metadata_upload_ID, int data_upload_ID) {
		// TODO Auto-generated method stub
		boolean status = false;
		OracleDB db = new OracleDB();
		try {
			if(db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD())) {
				String sqlCommand = "insert into associate_upload (data_upload_ID, metadata_upload_ID) " +
				" values ("+data_upload_ID+","+metadata_upload_ID+")";
				status = db.doUpdate(db, sqlCommand);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			IndirectOperation.rollbackDataitem(metadata_upload_ID);
			System.out.println("associating data upload and metadata upload failed.");
		} finally {
			if(!status) {
				IndirectOperation.rollbackDataitem(metadata_upload_ID);
				System.out.println("associating data upload and metadata upload failed.");
				System.exit(0);
			}
			db.disconnect();
		}
		System.out.println("data upload and metadata upload linked.");
	}

	public static int retrievePersonID(OracleDB db, String username) {
		// TODO Auto-generated method stub
		int personID = -1;
		try {
			Statement stmt = db.getConnection().createStatement(
					ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_UPDATABLE);
			String sqlCommand = "select person_id from associate_person_drupal_person a where email='"+username+"'";
			ResultSet rsPerson = db.doSelect(stmt, sqlCommand);
			if(rsPerson.next()) {
				personID = rsPerson.getInt(1);
			}
			rsPerson.close();
			stmt.close();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		return personID;
	}

	public static void parseColumnsValues(
			Vector<String> columns, 
			Vector<String> values, 
			NodeList nodeList) {
		//parse all information that should be inserted into "page_image" table
		for (int i=0;i<nodeList.getLength();i++) {
			Node node = nodeList.item(i);

			String columnName = node.getNodeName();
			String value = node.getTextContent();
			//any child that does not have a child contains information that should
			//be inserted into "page_image" table
			if(node.getChildNodes().getLength()==1) {
				//put column name and its value into two separate vectors
				columns.add(columnName);
				values.add(value);
			}
		}
	}

	public static void setPathNode(NodeList nodeList, String uploadPath) {
		Node pathNode = nodeList.item(nodeList.getLength()-1);
		String imagePath = pathNode.getTextContent();
		imagePath = uploadPath+imagePath;
		pathNode.setTextContent(imagePath);
	}

	public static void setID(NodeList nodeList, String ID) {
		// TODO Auto-generated method stub
		Node idNode = nodeList.item(nodeList.getLength()-1);

		//build the mappings between an internalID and its real data item ID.
		String internalID = idNode.getTextContent();

		Parse.internalMap.put(internalID, Integer.parseInt(ID));

		//update the internalID to be real data item ID
		idNode.setTextContent(ID);
	}

	public static void replaceInternalID(Node idNode, int ID) {
		// TODO Auto-generated method stub
		try {
			//build the mappings between an internalID and its real data item ID.
			String internalID = idNode.getTextContent();

			Parse.internalMap.put(internalID, Integer.valueOf(ID));
			
			//update the internalID to be real data item ID
			idNode.setTextContent(String.valueOf(ID));
		} catch (Exception ex ) {
			System.err.println(ex.getStackTrace());
			System.out.println("error in replacing internal ID, exit.");
			System.exit(0);
		}
	}
}