package edu.lehigh.dae.algorithm.ocrad;

import java.util.Vector;

public class BlockElement extends TextElement {
	String text = "";
	
	Vector<LineElement> lines;
	
	public BlockElement() {
		this.lines = new Vector<LineElement>();
	}
}
