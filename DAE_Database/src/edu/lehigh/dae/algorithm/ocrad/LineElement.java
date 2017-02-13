package edu.lehigh.dae.algorithm.ocrad;

import java.util.Vector;

public class LineElement extends TextElement {
	Vector<CharElement> chars;
	
	int height;
	
	int numberOfChars;
	
	public LineElement() {
		this.chars = new Vector<CharElement>();
	}
}
