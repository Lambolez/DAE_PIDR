package edu.lehigh.dae.parsemetadata.oracle;

import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.IndirectOperation;
import edu.lehigh.dae.utility.upload.oracle.Structure;

public class ParsePageElement {
	Element root;
	Node pageElementNode;
	Structure structure;

	int dataitemID;

	public ParsePageElement(Element root, Node node, Structure structure) {
		// TODO Auto-generated constructor stub
		this.root = root;
		this.pageElementNode = node;
		this.structure = structure;
	}

	public void doParsePageElement() {
		// TODO Auto-generated method stub		
		if(hasOtherPEAssociations(this.pageElementNode)) {
			//if the page element that we are parsing has associations with
			//other page elements that have not been parsed yet, we will
			//remove this page element node and add it to the tail of XML root.
			Node node = this.root.removeChild(this.pageElementNode);
			this.root.appendChild(node);
			return;
		}

		//get all the child nodes for the current page element
		NodeList nodeList = this.pageElementNode.getChildNodes();		

		/**insert records into data_item and page_image tables*/
		insertEntitySetRecords(nodeList);

		/**insert records into relationship set tables*/
		insertRelationshipSetRecords(nodeList);

		System.out.println();
	}

	private boolean hasOtherPEAssociations(Node node) {
		// TODO Auto-generated method stub
		for (int i=0;i<node.getChildNodes().getLength();i++) {
			Node tempNode = node.getChildNodes().item(i);
			if(tempNode.getNodeName().equalsIgnoreCase("associate_page_element")) {
				NodeList list = ((Element)tempNode).getElementsByTagName("associating_page_element_ID");
				Node tempNode1 = list.item(list.getLength()-1);
				String tempText = tempNode1.getTextContent();
				if(!Parse.internalMap.containsKey(tempText)) {
					return true;
				}
			}
		}
		return false;
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
							if(columnName.equalsIgnoreCase("associating_page_element_ID")) {
								String str = String.valueOf(Parse.internalMap.get(value));
								values.add(str);
							} else if(columnName.equalsIgnoreCase("page_element_property_value_id")) {
								String str = String.valueOf(Parse.internalMap.get(value));
								values.add(str);
							} else {
								values.add(value);
							}
						}
					}

					//for relationship set records, we need to add some additional columns
					if(tableName.equalsIgnoreCase("associate_page_element")) {
						columns.add("PAGE_ELEMENT_ID");
						values.add(String.valueOf(this.dataitemID));
					} else if(tableName.equalsIgnoreCase("has_value")) {
						columns.add("PAGE_ELEMENT_ID");
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
		 * physical_image_data_item and page_element due to their ISA relationships*/
		this.dataitemID = this.insertDataItemRecord();
		this.insertImageDataItemRecord();
		this.insertPhysicalImageDataItemRecord();

		//if everything works fine with previous three table, then we will be 
		//serting records into the "page_element" table now
		this.insertPageElementRecord(nodeList);
	}

	private void insertPageElementRecord(NodeList nodeList) {
		// TODO Auto-generated method stub
		//for a page_element, we need to map its internal id 
		//which we assign when parsing ocr output to the real 
		//data item from the data item record that we just inserted.
		IndirectOperation.setID(((Element) this.pageElementNode).getElementsByTagName("id"), 
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
				this.structure.db, columns, values, this.pageElementNode.getNodeName());
		System.out.println(sqlCommand);

		this.structure.db.doUpdate(this.structure.db, sqlCommand);
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
		"values (seq_data_item.nextval, 'parsed page element')";				
		int dataitemID = this.structure.db.doSelect(sqlCommand);
		return dataitemID;
	}

}
