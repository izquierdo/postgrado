/* 
 * Attribute.java
 * -------------
 * $Id: Attribute.java,v 1.2 2000/10/20 04:02:05 chenli Exp $
 */

class Attribute {
    String name;

    Attribute(String name) {
	this.name = name;
    }

    public String toString() {
	return name;
    }
}
