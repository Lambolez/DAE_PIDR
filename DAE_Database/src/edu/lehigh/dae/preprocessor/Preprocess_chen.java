package edu.lehigh.dae.preprocessor;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Vector;

import oracle.xml.parser.v2.DOMParser;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

import com.sun.org.apache.xerces.internal.dom.DocumentImpl;
import com.sun.org.apache.xml.internal.serialize.OutputFormat;
import com.sun.org.apache.xml.internal.serialize.XMLSerializer;

import edu.lehigh.dae.utility.general.Utility;
import edu.lehigh.dae.utility.upload.oracle.Operation;
import edu.lehigh.dae.utility.upload.oracle.OracleDB;

/**This class is used to pre-process the transformed XML metadata in the DAE format.
 * All IDs without provided mappings will be treated as local IDs and thus will be
 * replaced with database IDs in the process.*/
public class Preprocess_chen {
	/*userMapping stores the user-provided mappings between database IDs and some local IDs
	 *that are used in the XML file being pre-processed.
	 *
	 *mapping stores the mappings between local IDs and database IDs that are automatically
	 *generated during pre-processing because no user-provided were provided.*/
	Hashtable<String, String> userMapping, mapping;

	//the mapping between XML elements and view/table names. Each type of XML element 
	//needs to be inserted to a designated database table/view.
	Hashtable<String, String> xsuMapping;

	//We store a certain ordering here so that different types of metadata will be 
	//managed according to such ordering.
	Hashtable<String, Integer> xsuOrdering;

	//An XML document
	Document document;

	//The entire XML document provided by an user will be split into several small ones and
	//each of them only has one type of metadata, such as page_image, file, etc.
	Vector<Document> splitDocs;

	//Stores the paths of the split small XML files.
	Vector<String> splitFilePaths;

	//Stores all the types of data_items that are currently supported by the DAE XML format.
	Vector<String> dataitemEntities;

	//Stores types of data_items that need to have an <ID/> element.
	Vector<String> ids;

	//Stores types of data_items that need to have an <path/> element.
	Vector<String> paths;

	//Stores all the data_items in this XML file so that later we could correctly link
	//each data_item to the current upload for easy maintenance.
	HashSet<Integer> dataItems;

	//An database instance
	OracleDB db;

	/*Two SQL statements instances that are used to do some necessary inserts during
	 *pre-processing.*/
	/**This is not the best way to do this, since we do not want to do any inserts 
	 * while doing pre-processing. This needs to be fixed.*/
	Statement stmt, stmtPerson;

	// The path prefix on the server, accepted from the Upload class.
	String pathPrefix;
	// The ID for the current upload, accepted from the Upload class.
	int uploadID;
	// Not inserting new page_image data, but updating only
	boolean updateOnly;

	public boolean isUpdateOnly() {
		return updateOnly;
	}

	public void setUpdateOnly() {
		this.updateOnly = true;
	}

	public void unSetUpdateOnly() {
		this.updateOnly = false;
	}

	//This is used to execute an external process to generate thumbnails.
	Runtime runtime = Runtime.getRuntime();

	public Preprocess_chen() {
		this.dataItems = new HashSet<Integer>();
		this.updateOnly = false;
		
		//initialize the mappings between types of data_items and the view/table names
		//that are used to handle them.
		this.xsuMapping = new Hashtable<String, String>();
		this.xsuMapping.put("dataset", "dataset_view");
		this.xsuMapping.put("page_image", "page_image_view");
		this.xsuMapping.put("file", "file_view");
		this.xsuMapping.put("copyright", "file_view");
		this.xsuMapping.put("upload", "associate_upload_data_item");

		//initialize the ordering between different types of metadata.
		this.xsuOrdering = new Hashtable<String, Integer>();
		this.xsuOrdering.put("copyright", 1);
		this.xsuOrdering.put("dataset", 2);
		this.xsuOrdering.put("file", 3);
		this.xsuOrdering.put("page_image", 4);
		this.xsuOrdering.put("upload", 5);

		this.splitDocs = new Vector<Document>();
		this.splitFilePaths = new Vector<String>();

		this.userMapping = new Hashtable<String, String>();
		this.mapping = new Hashtable<String, String>();

		//initialize all the supported data_items by the DAE XML format.
		this.dataitemEntities = new Vector<String>();
		this.dataitemEntities.add("dataset");
		this.dataitemEntities.add("file");
		this.dataitemEntities.add("copyright");
		this.dataitemEntities.add("page_image");
		this.dataitemEntities.add("page_element");
		this.dataitemEntities.add("image_dataset_list_item");
		this.dataitemEntities.add("dataset_list_item");
		this.dataitemEntities.add("associating_dataset_list_item");
		this.dataitemEntities.add("page_element_list_item");
		this.dataitemEntities.add("value_list_item");
		this.dataitemEntities.add("copyright_list_item");

		//initialize the vector with all possible names for ID elements.
		this.ids = new Vector<String>();
		this.ids.add("id");
		this.ids.add("person_id");

		//initialize the vector with all possible names for path elements.
		this.paths = new Vector<String>();
		this.paths.add("path");
	}

	public void setPathPrefix(String pathPrefix) {
		this.pathPrefix = pathPrefix;
	}

	/**This method shows the general control flow of doing pre-processing.
	 * @metaPath - the path of the user provided XML file.
	 * @thumbnails - if thumbnails will be generated during pre-processing.*/
	public void doPreprocess(String metaPath, boolean thumbnails) {
		String filePath = metaPath;
		System.out.println("metapath in preprocessing: "+filePath);

		File file = new File(filePath);

		//open the user-provided XML file.		
		openXML(file);

		//retrive the user-provided mappings from the opened XML file.
		retrieveMapping();
		//print out all the user-provided mappings.
		printMappings();

		//replace all local IDs with database IDs
		replaceLocalID(thumbnails);

		//write split XML files back to disk
		putXMLToDisk(filePath);

		//print generated files
		printSplitFiles();

		//commit changes
		/*Note this command may not be needed because the person inserts and
		 *the following metadata inserts are in one session.*/
		commitChanges();

		System.out.println("xml files generated, start to parse to database\n");

		//store data to database
		storeMetadata();

		System.out.println("done");
	}
	
	/**This method stores the metadata included in all the split small XML files into
	 * database by calling the XSU.*/
	public void storeMetadata() {
		try {
			//set autocommit to false in order to guarantee database consistency and 
			//prevent redundant.
			this.db.getConnection().setAutoCommit(false);
			//declare a XSU instance.
			GenerateXML xsu = new GenerateXML();
			for (int i=0;i<this.splitFilePaths.size();i++) {
				//parse each split XML file and decide which table/view it should be
				//inserted into.
				DOMParser parser = new DOMParser();
				parser.parse(new File(this.splitFilePaths.elementAt(i)).toURI().toURL());
				Element root = parser.getDocument().getDocumentElement();
				NodeList list = root.getChildNodes();
				if(list.getLength() > 0) {
					/*Note the following for loop may not be needed.*/
					for (int j=0;j<list.getLength();j++) {
						//row tag represents the metadata type.
						String rowTag = list.item(j).getNodeName();
						//retrieve the view/table name based upon metadata type.
						String viewTable = this.xsuMapping.get(rowTag.toLowerCase());
						if(viewTable != null) {
							//call XSU
							System.out.println("rowTag = "+rowTag);
							System.out.println("viewTable = "+viewTable);
							
//chen: no need for update, oracle database has trigger on insert to handle it all
							System.out.println("Insert of " + rowTag);

							if(!rowTag.equalsIgnoreCase("upload") && !xsu.insert(db.getConnection(), rowTag, viewTable, 
								this.splitFilePaths.elementAt(i))) {
								System.out.println("error occurred in calling XSU, exit...");
								System.exit(1);
							}
							
/*							if(this.isUpdateOnly() && !rowTag.equalsIgnoreCase("upload")) {
								System.out.println("Update of " + rowTag);
								if(!xsu.update(db.getConnection(), rowTag, viewTable, 
									this.splitFilePaths.elementAt(i))) {
									System.out.println("error occurred in calling XSU, exit...");
									System.exit(1);
								}
							} else {
								System.out.println("Insert of " + rowTag);

								if(!rowTag.equalsIgnoreCase("upload") && !xsu.insert(db.getConnection(), rowTag, viewTable, 
									this.splitFilePaths.elementAt(i))) {
									System.out.println("error occurred in calling XSU, exit...");
									System.exit(1);
								}
							}*/
							
							break;
						}
					}
				}
			}
			
			//after successfully managing all the split small XML files,
			//we commit all the changes to database and make all data in
			//the entire upload process available.
			this.db.getConnection().commit();
			//disconnect SQL statements and database connection.
			stmt.close();
			stmtPerson.close();
			db.disconnect();
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error: XSU failed");
			System.exit(1);
		}
	}

	/**This method commits the person-related changes to the database.*/
	public void commitChanges() {
		try {
			this.db.getConnection().commit();
		} catch (SQLException e) {
			e.printStackTrace();
			System.err.println("error: failed to commit changes, rollback");
			try {
				this.db.getConnection().rollback();
				System.exit(1);
			} catch (SQLException e1) {
				e1.printStackTrace();
				System.err.println("error: rollback failed");
				System.exit(1);
			}
		}
	}

	/**This method prints out the paths of the split small XML files.*/
	private void printSplitFiles() {
		for (int i=0;i<this.splitFilePaths.size();i++) {
			System.out.println(this.splitFilePaths.elementAt(i));
		}
	}

	/**This method simply prints all the user-provided mappings.*/
	public void printMappings() {
		Enumeration<String> enu = this.userMapping.keys();
		while(enu.hasMoreElements()) {
			String localID = enu.nextElement();
			System.out.println(localID+"<->"+
					this.userMapping.get(localID.toLowerCase()));
		}
	}

	/**This method writes separate XML files after splitting the original XML file.
	 * @param originalPath - the path of the original XML file.*/
	public void putXMLToDisk(String originalPath) {
		try {            
			//we need to write the modified complete XML back to disk and paths of the
			//split files are based upon the path of the original file.
			String path = originalPath.substring(0, originalPath.lastIndexOf("/"));
			String name =originalPath.substring(originalPath.lastIndexOf("/")+1, 
					originalPath.length()); 		
			writeXML(this.document, path+"/complete-"+name);

			//the we also write split XML files back to disk
			for (int i=0;i<this.splitDocs.size();i++) {
				name = this.splitDocs.elementAt(i).
				getDocumentElement().getChildNodes().item(0).getNodeName();
				String writePath = path+"/split-"+name+".xml";
				writeXML(this.splitDocs.elementAt(i), writePath);
				this.splitFilePaths.add(new File(writePath).getAbsolutePath());
			}

			//write a separate XML file with the relationships between
			//the current upload and all the uploaded data_items
			Document doc = new DocumentImpl();
			//set the root element.
			Element root = doc .createElement("ROWSET");
			doc.appendChild(root);
			Vector<Integer> uploadedItems = new Vector<Integer>(this.dataItems);
			for (int i=0;i<uploadedItems.size();i++) {
				String value = String.valueOf(uploadedItems.elementAt(i));
				//add a upload element
				Element row = doc.createElement("upload");
				
				//add a upload_id element to be a child of the added <upload/> element.
				Element eleUpload = doc.createElement("upload_id");
				//set the content of this <upload_id/> element to be the current upload ID.
				Text textUpload = doc.createTextNode(String.valueOf(this.uploadID));
				eleUpload.appendChild(textUpload);
				
				//add a data_item_id element to be a child of the added <upload/> element.
				Element eleDataItem = doc.createElement("data_item_id");
				//add the ID of a data_item that is upload with the current upload as
				//the text content.
				Text textDataItem = doc.createTextNode(value);
				eleDataItem.appendChild(textDataItem);

				row.appendChild(eleUpload);
				row.appendChild(eleDataItem);
				root.appendChild(row);
			}
			//write the XML file describing the relationships between the current and its
			//data_items to disk.
			writeXML(doc, path+"/split-"+"upload.xml");
			//add this small XML file to the split XML files list.
			this.splitFilePaths.add(new File(path+"/split-"+"upload.xml").getAbsolutePath());
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error: failed to write split XML files to disk");
			System.exit(1);
		}
	}

	/**This method serialize a Document object to a disk file.
	 * @param document - a XML document
	 * @param filePath - the path of the serialized XML file.
	 */
	public void writeXML(Document document, String filePath) {
		try {
			BufferedWriter bw = new BufferedWriter(new FileWriter(filePath));
			OutputFormat of = new OutputFormat("XML","utf-8",true);
			of.setIndent(1);
			of.setIndenting(true);
			XMLSerializer serializer = new XMLSerializer(bw,of);
			serializer.asDOMSerializer();
			serializer.serialize(document);
			bw.close();
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error: write modified XML failed");
			System.exit(1);
		}
	}

	/**This method opens a XML file and get the Dom document of it.
	 * @param file - the user provided XML file.
	 * 
	 * @bug Since this approach is based on a DOM parser, it loads the complete
	 * XML document tree into memory. This has severe limitations on very large files
	 * (the memory footprint for a DOM tree is reported to be roughly four times the
	 * size of the initial XML file; we have configurations where the XML file can
	 * be >1GB)
	 * 
	 * @toto Convert this code to SAX rather than DOM.
	 * 
	 * */
	public void openXML(File file) {
		try {
			//declare a XML parser.
			DOMParser parser = new DOMParser();
			//parse to get the DOM representation of the XML file
			parser.parse(file.toURI().toURL());
			//get the XML document representing this XML file
			this.document = parser.getDocument();
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error: cannot open the XML file, exit program");
			//program will exit if the user provided XML file cannot be opened.
			System.exit(1);
		}
	}

	/**This method reads this XML file and extract the mappings
	 * provided by user. Such mappings are stored in a hashtable.*/
	public boolean retrieveMapping() {
		try {
			//get all the XML elements with the tag "pair". The DAE XML format
			//uses <pair/> to declare a single between database ID and local ID.
			NodeList pairList = this.document.getElementsByTagName("pair");

			//go through the list of mapping pairs
			for (int i=0;i<pairList.getLength();i++) {
				//get the current mapping pair
				Node node = pairList.item(i);
				//for each pair, get the local ID and the corresponding database ID
				String localString = null;
				String dbString = null;
				//manage a single mapping pair
				NodeList singlePair = node.getChildNodes();
				int dbIDCount = 0;
				for (int j=0;j<singlePair.getLength();j++) {
					//get a node within a single mapping pair
					Node singleID = singlePair.item(j);
					if(singleID.getNodeName().equalsIgnoreCase("local")) {
						//when seeing a <local/>, get the string as a local ID
						localString = singleID.getTextContent();
					} else if(singleID.getNodeName().equalsIgnoreCase("db")) {
						dbIDCount++;
						//when seeing a <db/>, get the string as a database ID
						dbString = singleID.getTextContent();
					}
				}

				/**for each mapping pair, we check the following:
				 * 1. if one local ID maps to multiple db IDs; 
				 *    however, we allow distinct local IDs map to the same db ID.
				 * 2. if either local ID or db ID or both are missing*/
				//check if a local ID or a database ID is null.
				if(isNull(localString, i+1) || isNull(dbString, i+1)) {
					System.exit(1);
				} else if (dbIDCount > 1) {
					//check if one local ID maps to multiple database IDs
					System.err.println("error: mapping to multiple db IDs " +
							"near the "+(i+1)+"th pair element");
					System.exit(1);
				} else if (isNumber(dbString, i+1) == false) {
					//check if the provided database ID is an integer
					System.exit(1);
				} else if (!hasDistinctMapping(localString, dbString, i+1)) {
					//check if the local ID maps to a previously different database ID.					
					System.exit(1);
				} else {
					//if this mapping pair passes all the checks, we add it.
					this.userMapping.put(localString.toLowerCase(), dbString);
				}
			}

			//copy user provided mapping to the overall mapping
			this.mapping = new Hashtable<String, String>(this.userMapping);
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error occurred in parsing metadata, exit...");
			System.exit(1);
		}
		return true;
	}

	/**This method checks if a local ID provided in a mapping pair maps to the same
	 * database IDs when it is mapped multiple times. We allow a local ID to appear
	 * in different mapping pairs but it must map to the same database ID.
	 * @param localString - a local ID
	 * @param dbString - a database ID
	 * @param i - the index of this string among the original mapping pairs. We have
	 * this parameter because we want to know which mapping pair is not valid.*/
	private boolean hasDistinctMapping(String localString, String dbString, int i) {
		//check if the local ID has been mapped before and if the mapped-to database ID
		//is the same.
		if(this.userMapping.containsKey(localString.toLowerCase()) &&
				!this.userMapping.get(localString.toLowerCase()).equalsIgnoreCase(dbString)) {
			System.err.println("error: local ID maps to distinct db IDs " +
					"near the "+i+"th pair element");
			return false;
		}
		return true;
	}

	/**This method checks if a string is null or is blank.	 * 
	 * @param str - the string to check
	 * @param i - the index of this string among the original mapping pairs. We have
	 * this parameter because we want to know which mapping pair is not valid.*/
	private boolean isNull(String str, int i) {
		if(str == null || str.equalsIgnoreCase("")) {
			System.err.println("error: missing local or db ID " +
					"near the "+i+"th pair element");
			return true;
		}
		return false;
	}

	/**This method checks if a string can be converted to a valid integer.	 * 
	 * @param str - the string to check
	 * @param i - the index of this string among the original mapping pairs. We have
	 * this parameter because we want to know which mapping pair is not valid.*/
	private boolean isNumber(String str, int i) {
		try {
			//trying to convert this string.
			Integer.valueOf(str);
		} catch (Exception ex) {
			System.err.println("error: db ID is not integer " +
					"near the"+i+"th pair element");
			return false;
		}
		return true;
	}

	/**This method replaces all local IDs or null ID values with database IDs.
	 * @param thumbnails - indicates whether thumbnails will be generated.*/
	public void replaceLocalID(boolean thumbnails) {
		//this is used to store all the thumbnail generation commands.
		FileWriter fw = null;
		try {
			fw = new FileWriter(this.pathPrefix+"commands.sh");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	    BufferedWriter bw = new BufferedWriter(fw);
	    PrintWriter commands = new PrintWriter(bw, true);

		//declare a database instance.
		db = new OracleDB();
		try {
			//if connecting to database fails, program will exit.
			if(!db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD())) {
				System.err.println("error: cannot connect to database");
				System.exit(1);
			} else {
				//set automit true in order not to leave redundant information in database
				//in case the entire upload and pre-processing would fail.
				db.getConnection().setAutoCommit(false);
				//initiate two statements for reserving IDs from database and 
				//insert necessary person information about the <contributor/> element.
				stmt = db.getConnection().createStatement();
				stmtPerson = db.getConnection().createStatement();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error: cannot connect to database");
			System.exit(1);
		}

		//get all the level-1 XML elements from the user-provided XML file.
		NodeList nodeList = this.document.getDocumentElement().getChildNodes();
		//do ID replacements for each of these elements and their child 
		//elements recursively.
		for (int i=0;i<nodeList.getLength();i++) {
			//get the current level-1 XML element.
			Node node = nodeList.item(i);
			//continue if this element has at least one child element.
			if(node.getChildNodes().getLength() > 0) {
				//get the tag of this XML element.
				String entityName = node.getNodeName();
				//we ignore mapping element because we have already handled them
				if(!entityName.equalsIgnoreCase("mapping")) {
					//start to do the real ID replacements for this element and
					//its child elements.
					doReplace(node, entityName, commands);
					//after replacing local IDs, we then split the metadata on the fly
					//based on the type of this XML element, such as page_image, file, etc.
					this.splitData(node);
				}
			}
		}

		//sort the split xml files because XSU needs to process different types of
		//metadata in a certain order.
		java.util.Collections.sort(this.splitDocs, new XSUComparator(this.xsuOrdering));

		//insert any necessary records
		try {
			//we may have encountered some new contributors during the ID replacement
			//process, so now we need to add them into database.
			stmtPerson.executeBatch();
		} catch (SQLException ex) {
			// TODO Auto-generated catch block
			ex.printStackTrace();
			System.err.println("error: cannot insert data");
			System.exit(1);
		}

		//whether or not we generate thumbnails during pre-processing, we will
		//store the command for doing this on disk.
	    commands.close();
				
		//chen: this is to be run at the end, not here, so I comment it out
		if(thumbnails) {
			createThumbs();
		}
	}
	
	public void createThumbs(){		

		System.out.println("generating thumbnails...");
		long start = System.currentTimeMillis();
		String[] command = {"/bin/bash", "-c", this.pathPrefix + "commands.sh > " + this.pathPrefix + "commands.log 2>&1"};
		this.executeCommand(command);
		long end = System.currentTimeMillis();
		System.out.println("time for running commands.sh: "+(end-start));
	}

	/**This method does local ID replacement.
	 * @param node - a XML element
	 * @param parentName - the tag of the XML element that this node is embedded.
	 * @param commands - the thumbnails generation command.*/
	private void doReplace(Node node, String parentName, PrintWriter commands) {
		//get the sub elements of this node.
		NodeList nodeList = node.getChildNodes();
		//this flag tells if a node has an local ID associated. Sometimes, users may
		//simply leave a empty <ID/> element for page_element and others.
		boolean hasIDNode = true;
		//the nodes that must have an <ID/> element.
		if(this.dataitemEntities.contains(parentName.toLowerCase()) ||
				parentName.equalsIgnoreCase("type_list_item") ||
				parentName.equalsIgnoreCase("property")) {
			//such entities must have an ID associated
			hasIDNode = false;
		}

		//manage each of the child elements of the node.
		for (int i=0;i<nodeList.getLength();i++) {
			//get the current child node
			Node child = nodeList.item(i);
			//if this child node does not have any further child nodes
			//but having a local ID
			if(child.getChildNodes().getLength() <= 1) {
				//if this is an id node
				if(this.ids.contains(child.getNodeName().toLowerCase())) {
					hasIDNode = true;
					String dbID = null;
					if(child.getChildNodes().getLength() == 0) {
						//if this child node does not have any further information.
						dbID = reserveDBID(child.getTextContent(), parentName);
						//this this child's parent is a data_item, then we add its 
						//reserved database ID so that it can be correctly linked to 
						//the current upload.
						if(this.dataitemEntities.contains(parentName.toLowerCase())) {
							this.dataItems.add(Integer.valueOf(dbID));
						}
					} else if(!this.mapping.containsKey(
							child.getTextContent().toLowerCase())) {
						//if the user provides an id but it is currently not mapped to 
						//any database ID, we will reserve one and assign it to dbID
						dbID = reserveDBID(child.getTextContent(), parentName);
						this.mapping.put(child.getTextContent().toLowerCase(), dbID);
						if(this.dataitemEntities.contains(parentName.toLowerCase())) {
							this.dataItems.add(Integer.valueOf(dbID));
						}
					} else if(this.mapping.containsKey(child.getTextContent().toLowerCase())) {
						//if we currently have the mapping for this local ID,
						//we will retrieve the db id for it and assign it to db ID
						dbID = this.mapping.get(child.getTextContent().toLowerCase());
						//check if the thing represented by this ID mathces the type
						//of the data conveyed by parentName. For example, if we have
						//parentName equals to datatype, then we need to check if there
						//exists a datatype in the db with that id.
						if(this.userMapping.containsKey(child.getTextContent().toLowerCase()) &&
								!checkTypeMatch(parentName, dbID)) {
							System.err.println("error: there is no "+parentName+
							" with this id in the database");
							System.exit(1);
						}
					}
					//replace local id with db ID
					child.setTextContent(dbID);
				} else if(this.paths.contains(child.getNodeName().toLowerCase()) &&
						!htmlPath(child)) {
					//if we see a path node, we need to update its path from its original
					//relative path to its absolute path on the server after the entire
					//upload process successfully finishes.
					String absolutePath = this.pathPrefix+child.getTextContent();
					child.setTextContent(absolutePath);
					
					//if the parent node is a page_image, then we append the thumbnails
					//generation command with the following.
					if(parentName.equalsIgnoreCase("page_image")) {
						commands.append("/opt/csw/bin/convert -resize 75x75 '"+
								absolutePath+"' '"+absolutePath+"-tn-75.png';\n");						
						commands.append("/opt/csw/bin/convert -resize 250x200 '"+
								absolutePath+"' '"+absolutePath+"-tn-250.png';\n");						
						commands.append("/opt/csw/bin/convert -resize '1000x1000>' '"+
								absolutePath+"' '"+absolutePath+"-tn-1000.png';\n");
					}						
				}
			} else if (child.getChildNodes().getLength() > 1){
				//if this node has child nodes, then we recursively apply the current
				//method to it to do necessary replacements
				doReplace(child, child.getNodeName(), commands);
			}
		}

		//if the current parentName should have an ID associated but it is not
		//then we need to append an ID node for it.
		if(!hasIDNode) {
			System.err.println("error: could not find an ID for "+parentName);
			System.exit(1);
		}
	}

	/**This method executes an external command.
	 * @param command - the real command.*/
	public void executeCommand(String[] command) {
		try {
			System.out.println(command[0] + ' ' + command[1] + " ...");
			//wait until the command finishes.
			Process process = runtime.exec(command);
			process.waitFor();
			
			InputStream stderr = process.getErrorStream();
            InputStreamReader isr = new InputStreamReader(stderr);
            BufferedReader br = new BufferedReader(isr);
            
            if(process.exitValue() != 0) {
				System.err.println("Failed to execute " + command[0]);
				
	            String line = null;
	            while ( (line = br.readLine()) != null)
	                System.err.println(line);

				System.exit(process.exitValue());
			}

		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}

	/**This method checks if the path of an XML element
	 * is a local path or a URL.
	 * @param child - the path node to check.*/
	private boolean htmlPath(Node child) {
		//get all the attributes of this <path/> element
		NamedNodeMap attr = child.getAttributes();
		if(attr != null) {
			for (int i=0;i<attr.getLength();i++) {
				//check if the "html" attribute of this <path/> node is set to true.
				if(attr.item(i).getNodeName().equalsIgnoreCase("html")) {
					if(attr.item(i).getTextContent().equalsIgnoreCase("true")) {
						attr.removeNamedItem("html");
						return true;
					}
				}
			}
		}
		return false;
	}

	/**This method checks if the type provided by user mappings actually
	 * matches a data_item's real datatype when it is described in the XML file.
	 * @param parentName - data_item type
	 * @param dbID - the database ID of this data_item*/
	private boolean checkTypeMatch(String parentName, String dbID) {
		try {
			String sqlCommand = null;
			ResultSet rs = null;
			if(parentName.equalsIgnoreCase("CONTRIBUTOR_LIST_ITEM")) {
				//check in person table
				sqlCommand = "select count(*) from person where id="+dbID;
			} else if(parentName.equalsIgnoreCase("image_dataset_list_item") ||
					parentName.equalsIgnoreCase("ASSOCIATING_DATASET_LIST_ITEM") ||
					parentName.equalsIgnoreCase("dataset_list_item")) {
				//check in dataset table
				sqlCommand = "select count(*) from dataset where id="+dbID;
			} else if(parentName.equalsIgnoreCase("PAGE_ELEMENT_LIST_ITEM")) {
				//check in page_element table
				sqlCommand = "select count(*) from page_element where id="+dbID;
			} else if(parentName.equalsIgnoreCase("property")) {
				//check in datatype_property table
				sqlCommand = "select count(*) from datatype_property where id="+dbID;
			} else if(parentName.equalsIgnoreCase("type_list_item")) {
				//check in datatype table
				sqlCommand = "select count(*) from datatype where id="+dbID;
			} else if(parentName.equalsIgnoreCase("copyright_list_item") ||
					parentName.equalsIgnoreCase("file")) {
				//check in datatype table
				sqlCommand = "select count(*) from files where id="+dbID;
			} else {
				//check in the table named by paramenter parentName
				sqlCommand = "select count(*) from "+parentName+" where id="+dbID;
			}
			rs = db.doSelect(stmt, sqlCommand);
			if(rs.next() && rs.getInt(1) == 0) {
				return false;
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			System.err.println("error: cannot check id existence");
			return false;
		}
		return true;
	}

	/**This method reserves a database ID for an encountered local ID.
	 * @param nodeContent - the local ID
	 * @param parentName - the entity name. It is used to determine from which
	 * database sequence we should ask for a new database ID.*/
	/*Note, the code here can be improved.*/
	private String reserveDBID(String nodeContent, String parentName) {
		int id = -1;
		String sqlCommand = null;
		ResultSet rs = null;

		if(this.dataitemEntities.contains(parentName.toLowerCase())) {
			//if we are giving a data item an database id
			sqlCommand = "select seq_data_item.nextval as id from dual";
			rs = db.doSelect(stmt, sqlCommand);
			try {
				if(rs.next()) {
					id = rs.getInt("id"); 
				}
			} catch (Exception ex) {
				ex.printStackTrace();
				System.err.println("error: cannot allocate a data item id");
				System.exit(1);
			}
		} else if(parentName.equalsIgnoreCase("property")) {
			//if we notice a datatype_property
			sqlCommand = "select seq_property.nextval as id from dual";
			rs = db.doSelect(stmt, sqlCommand);
			try {
				if(rs.next()) {
					id = rs.getInt("id"); 
				}
			} catch (Exception ex) {
				ex.printStackTrace();
				System.err.println("error: cannot allocate a property id");
				System.exit(1);
			}
		} else if(parentName.equalsIgnoreCase("type_list_item")) {
			//if we notice a datatype
			sqlCommand = "select seq_datatype.nextval as id from dual";
			rs = db.doSelect(stmt, sqlCommand);
			try {
				if(rs.next()) {
					id = rs.getInt("id"); 
				}
			} catch (Exception ex) {
				ex.printStackTrace();
				System.err.println("error: cannot allocate a property id");
				System.exit(1);
			}
		} else if(parentName.equalsIgnoreCase("CONTRIBUTOR_LIST_ITEM")) {
			//if we notice a contributor
			if(!nodeContent.equalsIgnoreCase("")) {
				//if we see a person name that is not null
				sqlCommand = "select id from person " +
				"where name='"+nodeContent+"'";
				rs = db.doSelect(stmt, sqlCommand);
				try {
					//we first check if the provided user already exists in the database 
					if(rs.next()) {
						//if we have this email in db, then we retrieve its db ID
						id = rs.getInt("id");
					} else {
						//if we do not have this email in db, we reserve a person id
						//and insert a record for it as well
						sqlCommand = "select seq_person.nextval as id from dual";
						rs = db.doSelect(stmt, sqlCommand);
						if(rs.next()) {
							id = rs.getInt("id");
							//insert command
							sqlCommand = "insert into person (id, name) " +
							"values ("+id+", '"+nodeContent+"')";
							stmtPerson.addBatch(sqlCommand);
						} else {
							//if we cannot reserve a person id
							System.err.println("error: see a person_id not in db, " +
							"but cannot allocate an ID for it");
							System.exit(1);
						}
					}
				} catch (Exception ex) {
					//if there is anything when trying to get a new person id or retrieving
					//an existing person id, the program will exit.
					ex.printStackTrace();
					System.err.println("error: sql not correctly executed");
					System.exit(1);
				}
			} else {
				//if the provided user name is null or empty, then we cannot do anything
				//but stop pre-processing the current XML file.
				System.err.println("error: cannot leave person_id blank");
				System.exit(1);
			}
		}
		
		//return the reserved database ID.
		return String.valueOf(id);
	}

	/**This method will split a user-provided XML into separate files.
	 * Each of these files will contain metadata for different entity or
	 * relationship sets in the database, such as datasets and page image.*/
	public void splitData(Node node) {
		for (int i=0;i<this.splitDocs.size();i++) {
			Document doc = this.splitDocs.elementAt(i);
			Element root = doc.getDocumentElement();
			NodeList childs = root.getChildNodes();
			Node first = childs.item(0);
			String name = first.getNodeName();
			if(node.getNodeName().equalsIgnoreCase(name)) {
				//if this type of metadata has been seen before
				Node newNode = doc.importNode(node, true);
				root.appendChild(newNode);
				//doc.appendChild(root);
				return;
			}
		}
		//if we notice a new type of metadata
		Document newDoc = new DocumentImpl();
		Element newRoot = newDoc.createElement("ROWSET");
		Node newNode = newDoc.importNode(node, true);
		newRoot.appendChild(newNode);
		newDoc.appendChild(newRoot);
		this.splitDocs.add(newDoc);
	}

	/**This method sets the upload ID to be current upload, accepted from the Upload class.
	 * @param uploadID - the current upload ID.*/
	public void setUploadID(int uploadID) {
		// TODO Auto-generated method stub
		this.uploadID = uploadID;
	}



	public void insertHas_Stat(int transID) {
		// TODO Auto-generated method stub
		boolean status = false;
		OracleDB db = new OracleDB();
		try {
			db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD());
			
			Vector<Integer> datas = new Vector<Integer>(this.dataItems);
			for (int i=0;i<datas.size();i++) {
				String value = String.valueOf(datas.elementAt(i));
				String sqlCommand = "insert into has_stat (data_item_id, data_stat_id) " +
		"values ("+value+", "+transID+")";
				status = db.doUpdate(db, sqlCommand);
			}
			
		} catch (Exception ex) {
			ex.printStackTrace();
			Operation.rollbackDataitem(uploadID);
		} finally {
			db.disconnect();
			if(!status) {
				System.err.println("insertion into has_stat failed.");
				System.exit(1);
			}
			System.out.println("insertion into has_stat done!\n");
		}
	}

	public void lockDatset() {
		// TODO Auto-generated method stub
		boolean status = false;
		OracleDB db = new OracleDB();
		try {
			db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD());
			
			Vector<Integer> datas = new Vector<Integer>(this.dataItems);
			for (int i=0;i<datas.size();i++) {
				String value = String.valueOf(datas.elementAt(i));
				String sqlCommand = "update dataset_underlying set purpose=1 where id="+value;
				System.out.println(sqlCommand);
				
				status = db.doUpdate(db, sqlCommand);

			}
			
		} catch (Exception ex) {
			ex.printStackTrace();
			Operation.rollbackDataitem(uploadID);
		} finally {
			db.disconnect();
			if(!status) {
				System.err.println("Datasets can not be locked.");
				System.exit(1);
			}
		System.out.println("Datasets are locked!\n");
		}
	}
}