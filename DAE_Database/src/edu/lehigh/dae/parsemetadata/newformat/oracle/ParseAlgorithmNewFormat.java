package edu.lehigh.dae.parsemetadata.newformat.oracle;

import java.util.Vector;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.IndirectOperation;
import edu.lehigh.dae.utility.upload.oracle.Structure;

public class ParseAlgorithmNewFormat {
	Element root;
	Node entityNode;
	Structure structure;

	int algorithmID;

	public ParseAlgorithmNewFormat(Element root, Node node, Structure structure) {
		// TODO Auto-generated constructor stub
		this.root = root;
		this.entityNode = node;
		this.structure = structure;
	}

	public void doParseAlgorithm() {
		//get all the child nodes for the current page image
		NodeList nodeList = this.entityNode.getChildNodes();

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
							if(tableName.equalsIgnoreCase("algorithm_input") &&
									columnName.equalsIgnoreCase("data_id")) {
								if(Parse.internalMap.containsKey(value)) {
									value = String.valueOf(
											Parse.internalMap.get(value).intValue());
								}								
							} else if(tableName.equalsIgnoreCase("algorithm_output") &&
									columnName.equalsIgnoreCase("data_id")) {
								if(Parse.internalMap.containsKey(value)) {
									value = String.valueOf(
											Parse.internalMap.get(value).intValue());
								}
							} else if(tableName.equalsIgnoreCase("defines_algorithm") &&
									columnName.equalsIgnoreCase("person_id")) {
								value = String.valueOf(this.structure.personID);
							}

							columns.add(columnName);			
							values.add(value);
						}
					}

					//for relationship set records, we need to add some additional columns
					if(tableName.equalsIgnoreCase("algorithm_input")) {
						columns.add("algorithm_id");
						values.add(String.valueOf(this.algorithmID));
					} else if(tableName.equalsIgnoreCase("algorithm_output")) {
						columns.add("algorithm_id");
						values.add(String.valueOf(this.algorithmID));
					} else if(tableName.equalsIgnoreCase("defines_algorithm")) {
						columns.add("algorithm_id");
						values.add(String.valueOf(this.algorithmID));
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
		//for a page image, we need to concatenate the uploadPath 
		//and the path provided by the user		
		IndirectOperation.setPathNode(((Element) this.entityNode).getElementsByTagName("path"), 
				this.structure.uploadPath);

		//declare two empty vectors for columns and values
		Vector<String> columns = new Vector<String>();
		Vector<String> values = new Vector<String>();
		IndirectOperation.parseColumnsValues(columns, values, nodeList);

		//after having other columns parsed, we need to add the primary
		//key for this page image record. The primary key comes from the
		//sequence "seq_algorithm".
		columns.add("id");
		values.add("seq_algorithm.nextval");

		String sqlCommand = this.structure.db.composeSQLQuery(
				this.structure.db, columns, values, this.entityNode.getNodeName());
		System.out.println(sqlCommand);

		//this.algorithmID = (int)(Math.random()*100);
		this.algorithmID = this.structure.db.doSelect(sqlCommand);
	}
}