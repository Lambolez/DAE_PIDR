package edu.lehigh.dae.parsemetadata.newformat.oracle;

import java.io.File;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Scanner;
import java.util.Vector;

import oracle.xml.parser.v2.DOMParser;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.OracleDB;
import edu.lehigh.dae.utility.upload.oracle.Structure;

public class Parse {
	private boolean metaOnly;

	public static HashMap<String, Integer> entityMap;

	public static HashSet<String> stringColumnTypes;

	public static HashMap<String, Integer> internalMap;

	public Parse() {
		this.setMetaOnly(false);
	}

	public static void main(String args[]) {
		initializeEntityMap();
		initializeStringColumnTypes();
		initializeInternalIDs();

		Parse parse = new Parse();
		parse.doParse();
	}

	private static void initializeInternalIDs() {
		// TODO Auto-generated method stub
		Parse.internalMap = new HashMap<String, Integer>();
	}

	private static void initializeStringColumnTypes() {
		// TODO Auto-generated method stub
		Parse.stringColumnTypes = new HashSet<String>();
		Parse.stringColumnTypes.add("varchar");
		Parse.stringColumnTypes.add("varchar2");
		Parse.stringColumnTypes.add("datetime");
		Parse.stringColumnTypes.add("timestamp");
		Parse.stringColumnTypes.add("blob");
	}

	private static void initializeEntityMap() {
		// TODO Auto-generated method stub
		//Initialize entity map between strings and integers.
		Parse.entityMap = new HashMap<String, Integer>();
		entityMap.put("page_image", 0);
		entityMap.put("algorithm", 1);
		entityMap.put("algorithm_run", 2);
		entityMap.put("page_element", 3);
		entityMap.put("page_element_property_value", 4);
	}

	public void doParse() {
		OracleDB db = null;
		try {
			db = new OracleDB();
			if(db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD())) {
				/**get the uploads that are of type 0 and status 0.
				 * type = 0: metadata upload;
				 * status = 0: files associated with this metadata upload
				 * has not been parsed yet.*/
				Statement stmt = db.getConnection().createStatement(
						ResultSet.TYPE_SCROLL_SENSITIVE, ResultSet.CONCUR_UPDATABLE);
				String sqlCommand = "select u.id as id " +
				" from upload u where type=0 and status=0";
				ResultSet rs = db.doSelect(stmt, sqlCommand);

				Vector<Integer> metadataUploads = new Vector<Integer>();
				while(rs.next()) {
					int id = rs.getInt("id");
					metadataUploads.add(Integer.valueOf(id));
				}

				/**for every metadata upload, we need to go through the following three steps:
				 * 1. retrieve the data_upload_ID associated with the metadata upload.
				 * 2. retrieve the files associated with the data_upload.
				 * 3. retrieve the files of the metadata_upload.*/
				Scanner scanner = new Scanner(System.in);
				for (int i=0;i<metadataUploads.size();i++) {
					System.out.println("total unparsed metadata uploads: "+metadataUploads.size()+
							"\nthe next to parse is metadata upload ID: "+metadataUploads.elementAt(i)+
					"\nnow start to parse this one? (yes/no/abort): ");
					/**To give us the option to choose to parse the metadata or not*/
					int goOn = -1;
					while(goOn == -1) {
						String choice = scanner.next();
						if(choice.equalsIgnoreCase("no")) {
							System.out.println("total: "+metadataUploads.size()+", finished: "+(i)+", go to next...");
							goOn = 0;
						} else if(choice.equalsIgnoreCase("abort")) {
							System.out.println("total: "+metadataUploads.size()+", finished: "+(i)+", exit...");
							System.exit(0);
						} else if(choice.equalsIgnoreCase("yes")) {
							System.out.println("total: "+metadataUploads.size()+", finished: "+(i)+", start parsing the next...");
							goOn = 1;
						} else {
							System.out.println("please input yes, no or abort: ");
						}
					}					
					if(goOn == 0) {
						continue;
					}

					//sub-step1
					int metadataUploadID = metadataUploads.elementAt(i).intValue();
					sqlCommand = "select data_upload_id from associate_upload " +
					" where metadata_upload_id="+metadataUploadID;
					rs = db.doSelect(stmt, sqlCommand);

					/**one metadata_upload_id only maps to a single data_upload_id.
					 * when doing the upload, we enforced that each metadata upload
					 * must be describing some data upload except for algorithm_run.*/
					this.setMetaOnly(false);
					if(rs.next()) {
						int dataUploadID = rs.getInt(1);

						//sub-step1.5
						//get the person that uploaded the data upload.
						sqlCommand = "select a.email as email, a.person_id as id" +
						" from has_upload h, associate_person_drupal_person a" +
						" where h.upload_id = "+dataUploadID+ " and h.person_id = a.person_id";
						ResultSet rsPerson = db.doSelect(stmt, sqlCommand);
						int personID = -1;
						if(rsPerson.next()) {
							personID = rsPerson.getInt("id");
						} else {
							System.out.println("unkwown person for data upload: "+dataUploadID);
							System.exit(0);
						}

						//sub-step2: retrieve the files assciated with the data upload.
						sqlCommand = "select f.id as id, f.path as path" +
						" from files f, upload_has_file u " +
						" where u.upload_id="+dataUploadID+" and u.file_id=f.id";
						ResultSet rsData = db.doSelect(stmt, sqlCommand);

						//sub-step3: retrieve the files assciated with the ,metadata upload.
						sqlCommand = "select f.id as id, f.path as path" +
						" from files f, upload_has_file u " +
						" where u.upload_id="+metadataUploadID+" and u.file_id=f.id";
						ResultSet rsMetadata = db.doSelect(stmt, sqlCommand);

						String dataUploadPath = "/dae/database/dae0-upload/"+dataUploadID+"/";
						Structure structure = new Structure(db, personID, rsMetadata, rsData, dataUploadPath);
						/**Get the metadata file path from the result set.*/
						while(rsMetadata.next()) {
							String filepath = rsMetadata.getString("path");
							filepath = filepath.replace('\\', '/');

							this.parseXML(filepath, structure);
						}

						//finally, mark the current metadata upload as parsed.
						sqlCommand = "update upload set status=1 where id="+metadataUploadID;
						boolean status = db.doUpdate(db, sqlCommand);
						if(!status) {
							System.out.println("unable to mark upload status, now exit");
							System.exit(0);
						}
					} else {
						//if the program goes here, that means we are parsing some 
						//metadata without referring to any data files.
						this.metaOnly = true;
						int personID = -1;
						sqlCommand = "select person_id from has_upload where upload_id="+metadataUploadID;
						ResultSet rsPerson = db.doSelect(stmt, sqlCommand);
						if(rsPerson.next()) {
							personID = rsPerson.getInt(1);
						}

						sqlCommand = "select f.id as id, f.path as path " +
						" from files f, upload_has_file u " +
						" where u.upload_id="+metadataUploadID+" and u.file_id=f.id";
						ResultSet rsMetadata = db.doSelect(stmt, sqlCommand);

						Structure structure = new Structure(db, personID, rsMetadata, null, "");
						/**Get the metadata file path from the result set.*/
						while(rsMetadata.next()) {
							String filepath = rsMetadata.getString("path");

							//filepath = "ocrad_metadata.xml";
							this.parseXML(filepath, structure);
						}

						//finally, mark the current metadata upload as parsed.
						sqlCommand = "update upload set status=1 where id="+metadataUploadID;
						boolean status = db.doUpdate(db, sqlCommand);
						if(!status) {
							System.out.println("unable to mark upload status, now exit");
							System.exit(0);
						}
					}
				}
				if(metadataUploads.size()==0) {
					System.out.println("no unparsed metadata upload");
				}
				stmt.close();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		} finally {
			if(db != null) {
				db.disconnect();
			}
		}
	}

	int imageNumber = 0;
	int fileNumber = 0;
	private void parseXML(String filePath, Structure structure){
		//get the factory
		//DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

		try {
			//Using factory get an instance of document builder
			//DocumentBuilder db = dbf.newDocumentBuilder();

			//parse using builder to get DOM representation of the XML file
			DOMParser parser = new DOMParser();
			parser.parse(new File(filePath).toURL());
			Document document = parser.getDocument();

			Element root = getRootElement(document);

			/**This piece of code must be executed before parsing others*/
			//in order to parse page element, we need to parse all their
			//page_element_property_value first which will be stored as
			//new data items, so that we can associate them with the page
			//elements together later.
			NodeList list = root.getChildNodes();
			//System.out.println(list.getLength());
			for (int i=0;i<list.getLength();i++) {
				Node node = list.item(i);
				//System.out.println(node.getNodeName());
				if(hasInternalID(list.item(i))) {
					root.removeChild(node);
					root.appendChild(node);
					i--;
				} else {
					String elementName = list.item(i).getNodeName();
					if(elementName.equalsIgnoreCase("page_image")) {
						ParsePageImageNewFormat parse = 
							new ParsePageImageNewFormat(root, list.item(i), structure);
						parse.doParsePageImage();
						System.out.println("image: "+(++this.imageNumber));
					} else if(elementName.equalsIgnoreCase("dataset")) {
						ParseDataset parse = 
							new ParseDataset(root, list.item(i), structure);
						parse.doParseDataset();
					} else if(elementName.equalsIgnoreCase("data")) {
						ParseData parse = 
							new ParseData(root, list.item(i), structure);
						parse.doParseData();
					} else if(elementName.equalsIgnoreCase("algorithm")) {
						ParseAlgorithmNewFormat parse = 
							new ParseAlgorithmNewFormat(root, list.item(i), structure);
						parse.doParseAlgorithm();
					} else if(elementName.equalsIgnoreCase("files")) {
						ParseFiles parse = 
							new ParseFiles(root, list.item(i), structure);
						parse.doParseFiles();
						System.out.println("file: "+(++this.fileNumber));
					}
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			System.out.println("error occurred in parsing metadata, exit...");
			System.exit(0);
		}
	}

	private boolean hasInternalID(Node item) {
		// TODO Auto-generated method stub
		//check if this top element itself has any attribute
		NamedNodeMap attributes = item.getAttributes();
		if(attributes != null) {
			for (int j=0;j<attributes.getLength();j++) {
				Node attr = attributes.item(j);
				String value = item.getTextContent(); 
				if(attr.getNodeName().equalsIgnoreCase("type") &&
						attr.getNodeValue().equalsIgnoreCase("internal") &&
						!internalMap.containsKey(value)) {
					System.out.println("not in:\t"+value);
					return true;
				}
			}
		}

		//check its sub-element's sub-elements
		NodeList subList = item.getChildNodes();
		for (int k=0;k<subList.getLength();k++) {
			//System.out.println(subList.item(k).getNodeName());
			boolean check = this.hasInternalID(subList.item(k));
			if(check) {
				return true;
			}
		}
		return false;
	}

	private Element getRootElement(Document document){
		//get the root element
		Element root = document.getDocumentElement();		
		return root;
	}

	public void setMetaOnly(boolean metaOnly) {
		this.metaOnly = metaOnly;
	}

	public boolean isMetaOnly() {
		return metaOnly;
	}
}