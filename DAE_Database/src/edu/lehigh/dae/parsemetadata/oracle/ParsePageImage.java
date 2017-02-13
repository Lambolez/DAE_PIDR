package edu.lehigh.dae.parsemetadata.oracle;

import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.IndirectOperation;
import edu.lehigh.dae.utility.upload.oracle.Structure;


public class ParsePageImage {
	Element root;
	Node pageImageNode;
	Structure structure;

	int dataitemID;

	public ParsePageImage(Element root, Node node, Structure structure) {
		// TODO Auto-generated constructor stub
		this.root = root;
		this.pageImageNode = node;
		this.structure = structure;
	}

	public void doParsePageImage() {
		//get all the child nodes for the current page image
		NodeList nodeList = this.pageImageNode.getChildNodes();

		/**insert records into data_item and page_image tables*/
		insertEntitySetRecords(nodeList);

		/**insert records into relationship set tables*/
		insertRelationshipSetRecords(nodeList);

		System.out.println();
	}

	private void insertRelationshipSetRecords(NodeList nodeList) {
		// TODO Auto-generated method stub
		try {
			for (int i=0;i<nodeList.getLength();i++) {
				Node node = nodeList.item(i);
				if(node.getChildNodes().getLength()>1) {
					String tableName = node.getNodeName();

					Vector<String> columns = new Vector<String>();
					Vector<String> values = new Vector<String>();

					//get the child nodes
					NodeList childs = node.getChildNodes();										
					for (int j=0;j<childs.getLength();j++) {
						String columnName = childs.item(j).getNodeName();
						String value = childs.item(j).getTextContent();

						if(childs.item(j).getChildNodes().getLength()==1) {
							columns.add(columnName);
							if(columnName.equalsIgnoreCase("person_id")) {
								values.add(String.valueOf(this.structure.personID));
							} else {
								values.add(value);
							}
						}
					}

					//for relationship set records, we need to add some additional columns
					if(tableName.equalsIgnoreCase("contribute")) {
						columns.add("data_item_id");
						values.add(String.valueOf(this.dataitemID));
					} else if(tableName.equalsIgnoreCase("generates_page_image")) {
						columns.add("page_image_id");
						values.add(String.valueOf(this.dataitemID));
					}

					String sqlCommand = this.structure.db.composeSQLQuery(
							this.structure.db, columns, values, tableName);
					System.out.println(sqlCommand);
					this.structure.db.doUpdate(this.structure.db, sqlCommand);
				}
			}
		} catch (Exception ex) {
			System.err.println("Error occurred while inserting records into relationship set tables");
			ex.printStackTrace();
			System.exit(0);
		}
	}

	private void insertEntitySetRecords(NodeList nodeList) {
		// TODO Auto-generated method stub
		/**we need to insert record to all of the four tables: data_item, iamge_data_item, 
		 * physical_image_data_item and page_image due to their ISA relationships*/
		this.dataitemID = this.insertDataItemRecord();
		this.insertImageDataItemRecord();
		this.insertPhysicalImageDataItemRecord();

		//if everything works fine with previous three table, then we will be inserting records
		//into the "page_image" table now
		this.insertPageImageRecord(nodeList);
	}

	private void insertPhysicalImageDataItemRecord() {
		// TODO Auto-generated method stub
		String sqlCommand = "insert into physical_image_data_item " +
		" (id) " +
		" values ("+this.dataitemID+")";
		System.out.println(sqlCommand);
		this.structure.db.doUpdate(this.structure.db, sqlCommand);
	}

	private void insertImageDataItemRecord() {
		// TODO Auto-generated method stub
		String sqlCommand = "insert into image_data_item " +
		" (id) " +
		" values ("+this.dataitemID+")";
		System.out.println(sqlCommand);
		this.structure.db.doUpdate(this.structure.db, sqlCommand);
	}

	private int insertDataItemRecord() {
		// TODO Auto-generated method stub
		String sqlCommand = "insert into data_item (id, description) " +
		"values (seq_data_item.nextval, 'uploaded image')";				
		int dataitemID = this.structure.db.doSelect(sqlCommand);
		return dataitemID;
	}

	public void insertPageImageRecord(NodeList nodeList) {
		//for a page image, we need to concatenate the uploadPath and the path provided by the user
		IndirectOperation.setPathNode(((Element) this.pageImageNode).getElementsByTagName("path"), 
				this.structure.uploadPath);

		//declare two empty vectors for columns and values
		Vector<String> columns = new Vector<String>();
		Vector<String> values = new Vector<String>();
		IndirectOperation.parseColumnsValues(columns, values, nodeList);

		//after having other columns parsed, we need to add the primary
		//key for this page image record. The primary key should be the 
		//id of data item record that we just inserted.
		columns.add("id");
		values.add(String.valueOf(this.dataitemID));

		String sqlCommand = this.structure.db.composeSQLQuery(
				this.structure.db, columns, values, this.pageImageNode.getNodeName());
		System.out.println(sqlCommand);

		this.structure.db.doUpdate(this.structure.db, sqlCommand);
	}
}