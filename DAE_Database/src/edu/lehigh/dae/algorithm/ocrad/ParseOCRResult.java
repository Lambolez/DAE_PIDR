package edu.lehigh.dae.algorithm.ocrad;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.Vector;

public class ParseOCRResult {
	static String inputFilePath = null;
	static String outputFilePath = null;
	
	static int blockNumber = 0;

	Vector<CharElement> chars;
	Vector<LineElement> lines;
	static Vector<BlockElement> blocks;

	public ParseOCRResult() {
		chars = new Vector<CharElement>();
		lines = new Vector<LineElement>();
		blocks = new Vector<BlockElement>();
	}

	public static void main(String args[]) {
		//argument zero is the path of the OCR result file.
		ParseOCRResult.inputFilePath = args[0];
		ParseOCRResult.outputFilePath = args[1];
		
		ParseOCRResult parse = new ParseOCRResult();
		
		//do the parsing job
		parse.doParse();
		
		//print out the parsed results for debugging
		parse.printParsingResult();
		
		//after get the results parsed, we will then generate the XML metadata
		//that can be parsed into database later.
		GenerateMetadata generate = new GenerateMetadata();
		
		//do the metadata generation job
		generate.doGenerate(blocks);
		
		// tranform the ocrad to xml, by yang
		generate.generateXML(blocks, outputFilePath);
	}

	public void printParsingResult() {
		System.out.println("printing parsing results");
		
		for (int i=0;i<this.blocks.size();i++) {
			BlockElement block = blocks.elementAt(i);
			System.out.println("block "+i);
			System.out.println("bounding box: "+block.boundingBox);
			
			Vector<LineElement> lines = block.lines;
			for (int j=0;j<lines.size();j++) {
				LineElement line = lines.elementAt(j);
				System.out.println("line "+j);
				System.out.println("number of chars: "+line.numberOfChars);
				System.out.println("line height: "+line.height);
				
				Vector<CharElement> chars = line.chars;
				System.out.println("chars:");
				for (int k=0;k<chars.size();k++) {
					System.out.print("bounding box: "+chars.elementAt(k).boundingBox+"\t");
					System.out.println(chars.elementAt(k).text);
				}
			}
			
			System.out.println();
		}
	}
	
	public void doParse() {
		System.out.println("start parsing OCR results from ocrad ...");

		//create a new BlockElement object to store information
		//about this block.
		BlockElement block = null;
		LineElement line = null;

		try {
			//use a read buffer to open the input file
			BufferedReader br = new BufferedReader(new FileReader(this.inputFilePath));
			String str = "";
			while((str = br.readLine()) != null) {
				//read the input file line by line
				if(str.startsWith("source file") || 
						str.startsWith("total text blocks") ||
						str.startsWith("lines")) {
					//if we see the line read from the output file starting with
					//such any of such strings, we will not do anything becuase these
					//lines do not provide any useful information.
				} else if(str.charAt(0) == '#') {
					System.out.println("start of input file");
				} else if(str.startsWith("text block")) {
					//parse information for a text block.
					System.out.println("seeing a new text block");

					//we need to re-new the block element whenever we see a new block.
					block = new BlockElement();
					this.blocks.add(block);
					
					//parse the coordinates of this text block.
					//the parsed coordinates will be formed into a string.
					//bounding box is the only information available to a text block.
					parseBlock(str, block);
				} else if(str.startsWith("line") && !str.startsWith("lines")) {
					//parse the information for a text line.
					/**note that, if and only if a line read from the result starts with "line" not
					 * "lines" can be treated as a text line.*/

					//we need to re-new the line when seeing a new line.
					line = new LineElement();
					
					//add the new line element to the current block element.
					block.lines.add(line);
					parseLine(str, line);
				} else {
					//now, we see a line from the output file describing the information
					//about a particular character in a line.
					CharElement charElement = new CharElement();
					parseChar(str, charElement);
					
					//add this char to the line element.
					line.chars.add(charElement);
				}
			}
			br.close();
		} catch (Exception ex) {
			System.err.println("ERROR: open input file failed!");
			ex.printStackTrace();
			System.exit(0);
		}
		System.out.println("parsing accomplished.");
	}

	private void parseChar(String str, CharElement charElement) {
		// TODO Auto-generated method stub
		//first, we need to split the boundingbox from the recognized text
		String[] segments = str.split(";");
		String[] bounding = segments[0].split(" ");

		//parse the boundingbox first
		String boundingbox = "";
		for (int i=0;i<bounding.length;i++) {
			if(i != bounding.length-1) {
				boundingbox += bounding[i]+";";
			} else {
				boundingbox += bounding[i];
			}
		}
		charElement.boundingBox = boundingbox;
		
		//parse the recognized text
		String text = "";
		String[] ocr = segments[1].split(",");
		if(ocr.length == 1) {
			text = "";
		} else {
			String temp = ocr[ocr.length-1];
			temp = temp.trim();
			text = String.valueOf(temp.charAt(1));
		}
		charElement.text = text;
	}

	private void parseLine(String str, LineElement line) {
		// TODO Auto-generated method stub
		String[] segments = str.split(" ");

		//the indexes in the segments are hard-coded, because
		//the format of the output file from ocrad is uniform 
		//and won't change.
		line.numberOfChars = Integer.valueOf(segments[3]);
		line.height = Integer.valueOf(segments[5]);
	}

	private void parseBlock(String str, BlockElement block) {
		// TODO Auto-generated method stub
		String boundingBox = "";
		String[] segments = str.split(" ");
		for (int i=3;i<segments.length;i++) {
			if(i != segments.length-1) {
				boundingBox += segments[i]+";";
			} else {
				boundingBox += segments[i];
			}
		}
		block.boundingBox = boundingBox;
	}
}