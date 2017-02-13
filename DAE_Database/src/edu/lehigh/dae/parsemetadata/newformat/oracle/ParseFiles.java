package edu.lehigh.dae.parsemetadata.newformat.oracle;

import java.sql.ResultSet;
import java.sql.Statement;

import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import edu.lehigh.dae.utility.upload.oracle.Structure;

public class ParseFiles {
	Element root;
	Node entityNode;
	Structure structure;

	int dataitemID;

	public ParseFiles(Element root, Node node, Structure structure) {
		// TODO Auto-generated constructor stub
		this.root = root;
		this.entityNode = node;
		this.structure = structure;
	}

	public void doParseFiles() {
		//get all the child nodes for the current page image
		NodeList nodeList = this.entityNode.getChildNodes();

		String ID = getFileElementID(nodeList);

		/**insert records into data_item and page_image tables*/
		retrieveFileID(ID, nodeList);

		System.out.println();
	}

	private String getFileElementID(NodeList nodeList) {
		// TODO Auto-generated method stub
		String internalID = "";
		for (int i=0;i<nodeList.getLength();i++) {
			Node node = nodeList.item(i);
			if(node.getNodeName().equalsIgnoreCase("id")) {
				internalID = node.getTextContent();
				break;
			}
		}
		return internalID;
	}

	private void retrieveFileID(String internalID, NodeList nodeList) {
		// TODO Auto-generated method stub
		try {
			for (int i=0;i<nodeList.getLength();i++) {
				Node node = nodeList.item(i);
				if(node.getNodeName().equalsIgnoreCase("path")) {
					String path = node.getTextContent();
					path = this.structure.uploadPath+path;
					String sqlCommand = "select id from files " +
					" where path='"+path+"'";
					Statement stmt = this.structure.db.getConnection().createStatement();
					ResultSet rs = this.structure.db.doSelect(stmt, sqlCommand);
					int fileID = -1;
					if(rs.next()) {
						fileID = rs.getInt("id");
					}
					Parse.internalMap.put(internalID, Integer.valueOf(fileID));
					stmt.close();
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}