package edu.lehigh.dae.upload.oracle;

import java.io.File;

import edu.lehigh.dae.preprocessor.Preprocess;
import edu.lehigh.dae.utility.upload.oracle.Operation;

public class Upload {
	//the username and password must be provided to initiate an upload process
	//currently, such information is stored in tables: person and drupal_person.
	String username = null;
	String password = null;

	//datapath points to the path of the top level directory under which and its 
	//subdirectories all the data to be uploaded is stored.
	String dataPath = null;
	//metapath points to the XML file that includes all the metadata about the 
	//data to be uploaded.
	String metaPath = null;

	//this variable tells if we want to generate thumbnails in the upload process,
	//by default it is set to true.
	boolean thumbnails = true;

	public static void main(String args[]) {
		//declare a new instance
		Upload upload = new Upload();

		//initiate the upload the process
		upload.doUpload(args);
	}

	public boolean doUpload(String args[]) {
		//parse program parameters.
		parseParameters(args);

		//verify user credentials.
		int personID = Operation.verifyCredential(username, password);

		//generate a short description of the current upload.
		String uploadDescription = 
			Operation.generateUploadDescription(username, dataPath, metaPath);

		//send a request to the database asking for an ID to identify the current upload.
		int uploadID = Operation.reserveUploadID(uploadDescription);

		//create neccessary directories for the current upload
		String uploadDir = Operation.makeDirectory(personID, uploadID);

		//record the relationship between the current upload and is uploader.
		Operation.linkPersonUpload(personID, uploadID);

		//copy all files from the specified datapath to the designated path on the server,
		//including data and the metadata file.
		Operation.copyFiles(this.dataPath, uploadDir, uploadID, metaPath);
		//update the path of the metadata on the server so that it can be recognized when
		//doing metadata pre-processing and parsing.
		this.metaPath = uploadDir+new File(metaPath).getName();
		System.out.println("files successfully copied to server.\n");

		//start the process to parse metadata
		Preprocess pre = new Preprocess();
		//notify the preprocess of the current upload ID and the prefix of the upload path,
		//so that the preprocess can correctly concatenate the prefix with the relative path
		//provided in the metadata file.
		pre.setUploadID(uploadID);
		pre.setPathPrefix(uploadDir);
		pre.doPreprocess(metaPath, this.thumbnails);

		return true;
	}

	/**This method parses the parameters for the upload program.*/
	private void parseParameters(String[] args) {
		int paramStatus = 1;
		try {
			if(args.length%2!=0) {
				//parameters do not meet required alignment
				paramStatus = -1;
			} else {
				for (int i=0;i<args.length;i++) {
					if(args[i].startsWith("--")) {
						String param = args[i].substring(2);
						String argument = args[i+1];
						//match the parameter name with its argument
						this.matchParamArugment(param, argument);
						i++;
					}
				}

				//the following four parameters are required. So, if any of them is missed,
				//the upload process will be stopped.
				if(this.username==null || this.password==null || 
						this.dataPath==null || this.metaPath==null) {
					paramStatus = -1;
				}
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		} finally {
			String getParamStatus = Operation.translateParamStatus(paramStatus);		
			if(paramStatus != 1) {
				System.err.println("parameters parsing results: "+getParamStatus);
				System.exit(0);
			}
		}
	}

	/**This method matches argument to a parameter.*/
	private void matchParamArugment(String param, String argument) {
		if(param.equals("username")) {
			this.username = argument;
		} else if(param.equals("password")) {
			this.password = argument;
		} else if(param.equals("datapath")) {
			this.dataPath = argument;
		} else if(param.equals("metapath")) {
			this.metaPath = argument;
		} else if(param.equals("thumbnails")) {
			this.thumbnails = Boolean.parseBoolean(argument);
		}
	}
}