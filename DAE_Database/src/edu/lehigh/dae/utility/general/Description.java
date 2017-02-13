package edu.lehigh.dae.utility.general;

import java.util.Vector;

import org.xml.sax.Attributes;

public class Description {
	public Vector<String> elements;
	public Vector<Attributes> attrs;
	public Vector<String> values;
	
	public Description() {
		elements = new Vector<String>();
		attrs = new Vector<Attributes>();
		values = new Vector<String>();
	}
}
