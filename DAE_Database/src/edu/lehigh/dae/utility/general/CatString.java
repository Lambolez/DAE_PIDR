package edu.lehigh.dae.utility.general;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;

public class CatString {
	public static void main(String args[]) {
		CatString cat = new CatString();
		cat.doCat();
	}
	
	public void doCat() {
		try {
			BufferedReader br = new BufferedReader(new FileReader("molfiles.txt"));
			BufferedWriter bw = new BufferedWriter(new FileWriter("metadata_molfiles.txt"));
			String line = "";
			while((line=br.readLine())!=null) {
				/*String temp = "<includes_file>\n";
				temp += "<file_id type=\"internal\">";
				temp += "molfiles/"+line;
				temp += "</file_id>\n</includes_file>\n";*/
				String temp = "<files>\n" +
						"<id>molfiles/"+line+"</id>\n" +
						"<path>uspto-validation/molfiles/"+line+"</path>\n" +
						"</files>\n";
				bw.write(temp);
			}
			bw.close();
			br.close();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}