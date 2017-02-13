package edu.lehigh.dae.utility.general;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.Vector;

public class CompareVector {
	public static void main(String args[]) {
		new CompareVector().compare();
	}

	public void compare() {
		try {
			BufferedReader br = new BufferedReader(new FileReader(
					"/home/song/Documents/daetoy_tables.txt"));
			Vector<String> daetoy = new Vector<String>();
			String line = null;
			while((line=br.readLine())!=null) {
				daetoy.add(line);
			}
			br.close();

			br = new BufferedReader(new FileReader(
					"/home/song/Documents/dae0_tables.txt"));
			Vector<String> dae0 = new Vector<String>();
			line = null;
			while((line=br.readLine())!=null) {
				dae0.add(line);
			}
			
			for (int i=0;i<daetoy.size();i++) {
				if(!dae0.contains(daetoy.elementAt(i))) {
					System.out.println(daetoy.elementAt(i));
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}
