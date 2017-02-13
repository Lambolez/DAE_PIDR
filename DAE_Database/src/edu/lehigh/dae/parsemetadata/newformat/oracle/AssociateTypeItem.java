package edu.lehigh.dae.parsemetadata.newformat.oracle;

import java.sql.ResultSet;
import java.sql.Statement;

import edu.lehigh.dae.utility.upload.oracle.OracleDB;

public class AssociateTypeItem {
	public static void main(String args[]) {
		AssociateTypeItem asso = new AssociateTypeItem();
		asso.doAssociation();
	}
	
	private void doAssociation() {
		// TODO Auto-generated method stub
		try {
			OracleDB db = new OracleDB();
			if(db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD())) {
				String sqlCommand = "select id from page_image where path like '%.jpg'";
				Statement stmt = db.getConnection().createStatement(
						ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_UPDATABLE);
				//get all the page images
				ResultSet rs = db.doSelect(stmt, sqlCommand);
				rs.last();
				int total = rs.getRow();
				System.out.println("total: "+total);
				rs.beforeFirst();
				
				Statement insertStmt = db.getConnection().createStatement();
				while(rs.next()) {
					int id = rs.getInt("id");
					String insertCommand = "insert into associate_datatype_data_item " +
							" (datatype_ID, data_item_id) " +
							" values (1, "+id+")";
					insertStmt.executeUpdate(insertCommand);
					System.out.println((--total)+" left");
				}
				insertStmt.close();
				stmt.close();
				db.disconnect();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}
