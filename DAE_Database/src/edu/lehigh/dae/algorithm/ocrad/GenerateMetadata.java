package edu.lehigh.dae.algorithm.ocrad;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Vector;

import edu.lehigh.dae.utility.upload.oracle.OracleDB;

public class GenerateMetadata {	
	public static void main(String args[]) {

	}

	/*
	 * Generate the XML format for the ocrad results.
	 */
	public void generateXML(Vector<BlockElement> blocks, String outputFilePath) {
		System.out.println("generating XML");

		StringBuffer output = new StringBuffer(100 * 1024 * 1024);  
		output.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<metadata>\n");
		output.append("<page_image>\n");
		output.append("<id>"+outputFilePath+"</id>\n");
		try {
			output.append("<page_element_list>\n");
			for (int i=0;i<blocks.size();i++) {
				BlockElement block = blocks.elementAt(i);
				String id_block = "block"+(i+1);
				Vector<LineElement> lines = block.lines;
				for (int j=0;j<lines.size();j++) {
					LineElement line = lines.elementAt(j);
					String id_line = "block"+(i+1)+"-line"+(j+1);
					Vector<CharElement> chars = line.chars;
					for (int k=0;k<chars.size();k++) {
						//manage the text for the page element of char element
						String id = "block"+(i+1)+"-line"+(j+1)+"-char"+(k+1)+"-text";
						output.append(pageElementListItem(id, chars.elementAt(k).text, chars.elementAt(k).boundingBox + ";", id_line));
					}
					output.append(pageElementListItem(id_line, "" + line.numberOfChars, line.boundingBox + ";", id_block));
				}
				output.append(pageElementListItem(id_block, block.text, block.boundingBox + ";", null));
			}
			output.append("</page_element_list>\n");
			output.append("</page_image>\n");
			output.append("</metadata>"+"\n");
			BufferedWriter bw = new BufferedWriter(new FileWriter(ParseOCRResult.outputFilePath));
			bw.write(output.toString());
			bw.close();
		} catch (Exception e) {
			System.err.println("Error when generate xml from ocrad. " + e);
		}
	}

	private String pageElementListItem(String id, String value, String boundary, String idParent) {
		String output = "";
		output += "<page_element_list_item>\n";
		output += "<id>"+id+"</id>"+"\n";
		int pos1 = 0, pos2 = 0;
		int xy[] = {0, 0, 0, 0};
		for (int i = 0; i < 4; ++i) {
			pos1 = boundary.indexOf(";", pos2);
			try {
				xy[i] = Integer.parseInt(boundary.substring(pos2, pos1));
			} catch (Exception e) {
				System.err.println(boundary + ", " + i);
				return output;
			}
			pos2 = pos1 + 1;
		}
        output += "<topleftx>" + Math.min(xy[0], xy[2])+ "</topleftx>\n";
        output += "<toplefty>" + Math.min(xy[1], xy[3])+ "</toplefty>\n";
        output += "<width>" + (Math.max(xy[0], xy[2]) - Math.min(xy[0], xy[2]))+ "</width>\n";
        output += "<height>" + (Math.max(xy[1], xy[3]) - Math.min(xy[1], xy[3]))+ "</height>\n";
		output += "<value_list>\n";
		output += "<value_list_item>\n";
		output += "<id>"+id+"-value"+"</id>\n";
		output += "<value>" + value + "</value>\n";
		output += "</value_list_item>"+"\n";
		if (idParent != null) {
			output += "<value_list_item>\n";
			output += "<id/>\n";
			output += "<value>" + idParent + "</value>\n";
			output += "<property><id>ocrad-Page-Element-Included</id><name>ocrad-Page-Element-Included</name><description>ocrad-Page-Element-Included</description></property>";
			output += "</value_list_item>"+"\n";
		}
		output += "</value_list>\n";
		output += "</page_element_list_item>\n";
		return output;
	}
	
	public void doGenerate(Vector<BlockElement> blocks) {
		//the metadata should be generated from inner to outside, i.e., 
		//from char element to line element and to page element, so that
		//all these page elements at different levels can be associated.
		System.out.println("generating metadata");

		String output = "<metadata>";
		try {
			OracleDB db = new OracleDB();
			db.connect(OracleDB.getSID(), OracleDB.getUSERNAME(), OracleDB.getPASSWORD());
			for (int i=0;i<blocks.size();i++) {
				BlockElement block = blocks.elementAt(i);

				Vector<String> lineIDs = new Vector<String>();
				Vector<LineElement> lines = block.lines;
				for (int j=0;j<lines.size();j++) {
					LineElement line = lines.elementAt(j);

					Vector<CharElement> chars = line.chars;
					Vector<String> charIDs = new Vector<String>();
					for (int k=0;k<chars.size();k++) {
						//manage the text for the char element
						String id = "block"+(i+1)+"-line"+(j+1)+"-char"+(k+1)+"-text";
						output += "<page_element_property_value>"+"\n";
						output += "<id>"+id+"</id>"+"\n";
						output += "<value>"+chars.elementAt(k).text+"</value>"+"\n";
						output += "<value_of>"+"\n";
						//assuming that 1 represents the data type property "recognized text".
						output += "<data_type_property_id>"+1+"</data_type_property_id>"+"\n";
						output += "</value_of>"+"\n";
						output += "</page_element_property_value>"+"\n";

						//manage the char element itself
						String charID = "block"+(i+1)+"-line"+(j+1)+"-char"+(k+1);
						charIDs.add(charID);
						output += "<page_element>"+"\n";
						output += "<id>"+charID+"</id>"+"\n";
						output += "<boundary>"+chars.elementAt(k).boundingBox+"</boundary>"+"\n";

						//add the above page_element_property_value in
						output += "<has_value>"+"\n";
						output += "<page_element_property_value_id type=\"indirect\">"+id+"</page_element_property_value_id>"+"\n";						
						output += "</has_value>"+"\n";

						output += "</page_element>"+"\n";
					}

					/**after we've got the information for a char element, we can
					 * then start adding the line element that is associated with it.*/
					//manage the number of chars and height of a line element
					String id_chars = "block"+(i+1)+"-line"+(j+1)+"-numberofchars";
					output += "<page_element_property_value>"+"\n";
					output += "<id>"+id_chars+"</id>"+"\n";
					output += "<value>"+line.numberOfChars+"</value>"+"\n";
					output += "<value_of>"+"\n";
					//assuming that 2 represents the data type property "number of chars".
					output += "<data_type_property_id>"+2+"</data_type_property_id>"+"\n";
					output += "</value_of>"+"\n";
					output += "</page_element_property_value>"+"\n";

					String id_height = "block"+(i+1)+"-line"+(j+1)+"-height";
					output += "<page_element_property_value>"+"\n";
					output += "<id>"+id_height+"</id>"+"\n";
					output += "<value>"+line.height+"</value>"+"\n";
					output += "<value_of>"+"\n";
					//assuming that 3 represents the data type property "height".
					output += "<data_type_property_id>"+3+"</data_type_property_id>"+"\n";
					output += "</value_of>"+"\n";
					output += "</page_element_property_value>"+"\n";

					//manage the line element itself and its associations with
					//other char elements.
					String lineID = "block"+(i+1)+"-line"+(j+1);
					lineIDs.add(lineID);
					output += "<page_element>"+"\n";
					output += "<id>"+lineID+"</id>"+"\n";
					
					//add the above page_element_property_values in
					output += "<has_value>"+"\n";
					output += "<page_element_property_value_id type=\"indirect\">"+id_chars+"</page_element_property_value_id>"+"\n";						
					output += "</has_value>"+"\n";

					output += "<has_value>"+"\n";
					output += "<page_element_property_value_id type=\"indirect\">"+id_height+"</page_element_property_value_id>"+"\n";						
					output += "</has_value>"+"\n";

					//mangage the associations with other char elements
					for (int m=0;m<charIDs.size();m++) {
						String tempID = charIDs.elementAt(m); 
						output += "<associate_page_element>"+"\n";
						output += "<associating_page_element_ID type=\"indirect\">"+tempID+"</associating_page_element_ID>"+"\n";						
						output += "<type>"+1+"</type>"+"\n";
						output += "</associate_page_element>"+"\n";
					}
					output += "</page_element>"+"\n";
				}

				//manage the block element itself
				output += "<page_element>"+"\n";
				output += "<id>"+"block"+(i+1)+"</id>";
				output += "<boundary>"+block.boundingBox+"</boundary>"+"\n";
				
				//add the associations with other line elements
				for (int m=0;m<lineIDs.size();m++) {
					String tempID = lineIDs.elementAt(m); 
					output += "<associate_page_element>"+"\n";
					output += "<associating_page_element_ID type=\"indirect\">"+tempID+"</associating_page_element_ID>"+"\n";						
					output += "<type>"+1+"</type>"+"\n";
					output += "</associate_page_element>"+"\n";
				}
				output += "</page_element>"+"\n";
			}

			output += "</metadata>"+"\n";
			BufferedWriter bw = new BufferedWriter(new FileWriter(ParseOCRResult.outputFilePath));
			bw.write(output);
			bw.close();

			db.disconnect();
		} catch (Exception ex) {
			System.err.println("ERROR: generating metadata for the parsed ocrad result");
			ex.printStackTrace();
			System.exit(0);
		}

		System.out.println("ocrad metadata generated");
	}
}