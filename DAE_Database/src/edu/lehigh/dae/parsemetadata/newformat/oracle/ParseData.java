package edu.lehigh.dae.parsemetadata.newformat.oracle;

import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.IndirectOperation;
import edu.lehigh.dae.utility.upload.oracle.Structure;

public class ParseData {
	Element root;
	Node entityNode;
	Structure structure;

	int dataID;
	
	public ParseData(Element root, Node node, Structure structure) {
		// TODO Auto-generated constructor stub
		this.root = root;
		this.entityNode = node;
		this.structure = structure;
	}

	public void doParseData() {
		//get all the child nodes for the current data
		NodeList nodeList = this.entityNode.getChildNodes();

		/**insert records into data table*/
		insertEntitySetRecords(nodeList);

		/**insert records into relationship set tables*/
		insertRelationshipSetRecords(nodeList);

		System.out.println();
	}
	
	private void insertRelationshipSetRecords(NodeList nodeList) {
		// TODO Auto-generated method stub
		try {
			//manage each record in a relationship set table
			for (int i=0;i<nodeList.getLength();i++) {
				Node node = nodeList.item(i);
				if(node.getChildNodes().getLength()>1) {
					String tableName = node.getNodeName();

					Vector<String> columns = new Vector<String>();
					Vector<String> values = new Vector<String>();

					//manage the columns for this relationship set record
					NodeList childs = node.getChildNodes();										
					for (int j=0;j<childs.getLength();j++) {
						String columnName = childs.item(j).getNodeName();
						String value = childs.item(j).getTextContent();

						if(childs.item(j).getChildNodes().getLength()==1) {
							/*if(tableName.equalsIgnoreCase("includes_page_image") &&
									columnName.equalsIgnoreCase("page_image_id")) {
								if(Parse.internalMap.containsKey(value)) {
									value = String.valueOf(
											Parse.internalMap.get(value).intValue());
								}								
							} else if(tableName.equalsIgnoreCase("associate_dataset") &&
									columnName.equalsIgnoreCase("associating_dataset_id")) {
								if(Parse.internalMap.containsKey(value)) {
									value = String.valueOf(
											Parse.internalMap.get(value).intValue());
								}
							}*/

							columns.add(columnName);
							values.add(value);
						}
					}

					//for relationship set records, we need to add some additional columns
					if(tableName.equalsIgnoreCase("type_of")) {
						columns.add("data_id");
						values.add(String.valueOf(this.dataID));
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
		String sqlCommand = "insert into data (id) values (seq_data.nextval)";
		//this.dataID = (int)(Math.random()*10000);
		this.dataID = this.structure.db.doSelect(sqlCommand);
		
		//declare two empty vectors for columns and values
		//replace the internal ID with the real data item ID
		NodeList list = this.entityNode.getChildNodes();
		for (int i=0;i<list.getLength();i++) {
			if(list.item(i).getNodeName().equalsIgnoreCase("id")) {
				IndirectOperation.replaceInternalID(list.item(i), this.dataID);
			}
		}
		
		Vector<String> columns = new Vector<String>();
		Vector<String> values = new Vector<String>();
		IndirectOperation.parseColumnsValues(columns, values, nodeList);

		Vector<String> conditionColumns = new Vector<String>();
		Vector<String> conditionValues = new Vector<String>();
		conditionColumns.add("id");
		conditionValues.add(String.valueOf(this.dataID));
		
		sqlCommand = this.structure.db.composeUpdateSQLQuery(
				this.structure.db, columns, values, 
				conditionColumns, conditionValues, this.entityNode.getNodeName());
		/*sqlCommand = this.structure.db.composeSQLQuery(
				this.structure.db, columns, values, this.entityNode.getNodeName());*/
		System.out.println(sqlCommand);
		
		this.structure.db.doUpdate(this.structure.db, sqlCommand);
	}
}
