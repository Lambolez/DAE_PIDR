package edu.lehigh.dae.utility.general;

import java.io.BufferedWriter;
import java.io.FileWriter;

public class Utility {
	public static void log(String message, String filename) {
			// TODO Auto-generated method stub
			try {
				BufferedWriter bw = new BufferedWriter(
						new FileWriter (filename, true));
				bw.write(message);
				bw.newLine();
				bw.close();
			} catch (Exception ex) {
				ex.printStackTrace();
			}
		}
}

