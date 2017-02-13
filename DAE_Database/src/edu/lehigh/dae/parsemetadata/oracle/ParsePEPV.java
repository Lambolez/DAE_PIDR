package edu.lehigh.dae.parsemetadata.oracle;

import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.IndirectOperation;
import edu.lehigh.dae.utility.upload.oracle.Structure;

public class ParsePEPV {
	Element root;
	Node pepvNode;
	Structure structure;

	int dataitemID;

	public ParsePEPV(Element root, Node node, Structure structure) {
		// TODO Auto-generated constructor stub
		this.root = root;
		this.pepvNode = node;
		this.structure = structure;
	}

	public void doParsePEPV() {
		// TODO Auto-generated method stub
		//get all the child nodes for the current page element property value
		NodeList nodeList = this.pepvNode.getChildNodes();

		/**insert records into data_item 
		 * page_element_property_value tables*/
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
					if(tableName.equalsIgnoreCase("value_of")) {
						columns.add("PAGE_ELEMENT_PROPERTY_VALUE_ID");
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
		 * logical_image_data_item and page_element_property_value due to 
		 * their ISA relationships.*/
		this.dataitemID = this.insertDataItemRecord();
		this.insertImageDataItemRecord();
		this.insertLogicalImageDataItemRecord();

		//if everything works fine with previous three table, then we 
		//will be inserting records into the "page_element_property_value" table now
		this.insertPEPVRecord(nodeList);
	}

	public void insertPEPVRecord(NodeList nodeList) {
		//for a page_element_property_value, we need to 
		//map its internal id which we assign when parsing 
		//ocr output to the real data item from the data item
		//record that we just inserted.
		IndirectOperation.setID(((Element) this.pepvNode).getElementsByTagName("id"), 
				String.valueOf(this.dataitemID));
		
		//declare two empty vectors for columns and values
		Vector<String> columns = new Vector<String>();
		Vector<String> values = new Vector<String>();
		IndirectOperation.parseColumnsValues(columns, values, nodeList);

		/*//after having other columns parsed, we need to add the primary
		//key for this page_element_property_value record. The primary key 
		//should be the id of data item record that we just inserted.
		columns.add("id");
		values.add(String.valueOf(this.dataitemID));*/

		String sqlCommand = this.structure.db.composeSQLQuery(
				this.structure.db, columns, values, this.pepvNode.getNodeName());
		System.out.println(sqlCommand);

		this.structure.db.doUpdate(this.structure.db, sqlCommand);
	}

	private void insertLogicalImageDataItemRecord() {
		// TODO Auto-generated method stub
		String sqlCommand = "insert into logical_image_data_item " +
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
		"values (seq_data_item.nextval, 'page element property value')";				
		int dataitemID = this.structure.db.doSelect(sqlCommand);
		return dataitemID;
	}

}
