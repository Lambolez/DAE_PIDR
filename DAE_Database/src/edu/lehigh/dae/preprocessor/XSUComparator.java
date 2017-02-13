package edu.lehigh.dae.preprocessor;

import java.util.Comparator;
import java.util.Hashtable;

import org.w3c.dom.Document;

/**This class help sort files with different types of metadata, such as
 * page_image, dataset, copyright, file, etc. Currently, we need to assign
 * ordering between files with different types of metadata. For example, 
 * dataset metadata needs to be parsed before page_image metadata.*/

/*An ongoing modification to the object-relational view trigger might let
 *us drop this class. This means different types of metadata can be sent 
 *to database in any order.*/
public class XSUComparator implements Comparator<Document> {

	Hashtable<String, Integer> xsuOrdering;

	public XSUComparator(Hashtable<String, Integer> xsuOrdering) {
		this.xsuOrdering = xsuOrdering;
	}

	public int compare(Document doc1, Document doc2) {
		//the two names are the names of the two first XML nodes in two XML files.
		String name1 = doc1.getDocumentElement().getChildNodes().item(0).getNodeName();
		String name2 = doc2.getDocumentElement().getChildNodes().item(0).getNodeName();

		//get the relative ordering between two types of metadata represented by the
		//two names. The orderings are pre-set.
		int order1 = this.xsuOrdering.get(name1.toLowerCase()).intValue();
		int order2 = this.xsuOrdering.get(name2.toLowerCase()).intValue();
		
		//compare for sorting
		if(order1 > order2) {
			return 1;
		}
		return 0;
	}
}