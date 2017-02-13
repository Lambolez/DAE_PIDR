package edu.lehigh.dae.utility.general;

import java.io.File;
import java.util.Vector;

public class LookupDirectory {
	public static void main(String args[]) {
		LookupDirectory sf = new LookupDirectory();
		sf.listFilesInFolder(args[0], null);
	}

	public String[] listFilesInFolder(String folderPath, String extension) {
		System.out.println("search file: "+folderPath);
		File file = null;
		Vector<String> filePaths = new Vector<String>();
		Vector<String> folders = new Vector<String>();

		folders.add(folderPath);

		for (int i=0;i<folders.size();i++) {
			file = new File(folders.elementAt(i));
			File[] tempFiles = file.listFiles();

			for (int j=0;j<tempFiles.length;j++) {
				if(tempFiles[j].isDirectory()) {
					folders.add(tempFiles[j].getAbsolutePath());

				} else if(tempFiles[j].isFile() && 
						getExtension(tempFiles[j]).equalsIgnoreCase(extension)) {
					filePaths.add(tempFiles[j].getAbsolutePath());
				}
			}
		}

		String[] files=new String[filePaths.size()];
		for (int i=0;i<filePaths.size();i++) {
			files[i]=filePaths.elementAt(i);
			//System.out.println(files[i]);
		}

		System.out.println("total files: "+files.length);
		return files;
	}
	
	private String getExtension(File file) {
		// TODO Auto-generated method stub
		int dot = file.getName().lastIndexOf(".");
		if(dot<0) {
			return null;
		} else {
			return file.getName().substring(dot+1);
		}
	}
}