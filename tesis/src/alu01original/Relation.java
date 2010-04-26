/* 
 * Relation.java
 * -------------
 * $Id: Relation.java,v 1.12 2000/11/25 02:34:37 chenli Exp $
 */

import java.util.*;

class Relation {
  String  name   = "_unknown_";  // relation name
  Vector  schema = new Vector(); // a vector of attributes
  HashSet tuples = new HashSet();

  /**
   * Creates a relation.
   */
  Relation(String name) {
    this.name = name;
  }
  
  /**
   * Creates a relation given an id.
   */
  Relation(int relIndex) {
    name   = new String("r" + relIndex);
    schema = new Vector();
    int attrNum = GoodPlan.relAttrNum;
    
    for (int i = 1; i <= attrNum; i ++) {
      String attrName = UserLib.getChar(i) + i;
      schema.add(new Attribute(attrName));
    }
  }

  /**
   * Constructor
   */
  Relation(String name, Vector schema, HashSet tuples) {
    this.name   = name;
    this.schema = schema;
    this.tuples = tuples;
  }
  
  /**
   * Constructor
   */
  Relation(String name, Vector schema) {
    this.name   = name;
    this.schema = schema;
  }

  public Vector getSchema() {
    return schema;
  }
  
  public String getName() {
    return name;
  }

  public int getAttrNum() {
    return schema.size();
  }

  public HashSet getTuples() {
    return tuples;
  }

  /**
   * set the query of each tuple
   */
  public void setQuery(Query query) {
    for (Iterator iter = tuples.iterator(); iter.hasNext();) {
      Tuple tuple = (Tuple) iter.next();
      tuple.setQuery(query);
    }    
  }

  public Relation project(Vector usefulArgs) {
    Vector  resSchema = new Vector();
    HashSet resTuples = new HashSet();

    // computes the schema
    for (int i = 0; i < schema.size(); i ++) {
      Argument arg = (Argument) schema.elementAt(i);
      if (usefulArgs.contains(arg)) 
	resSchema.add(arg);
    }

    // computes the tuples

    for (Iterator iter = tuples.iterator(); iter.hasNext();) {
      Tuple  tuple = (Tuple) iter.next();
      Vector resTupleArgs = new Vector();

      for (int i = 0; i < schema.size(); i ++) {
	Argument argSchema = (Argument) schema.elementAt(i);
	Argument argVale   = (Argument) tuple.getArgs().elementAt(i);
	if (usefulArgs.contains(argSchema)) 
	  resTupleArgs.add(argVale);
      }

      // yields one tuple
      resTuples.add(new Tuple(name, resTupleArgs, 
			      tuple.getMapping(), tuple.getMapSubgoals()));
    }
    
    return new Relation(this.getName(), resSchema, resTuples);
  }
  
  public String toString() {
    StringBuffer result = new StringBuffer(name);
    result.append("(");
    for (int i = 0; i < schema.size(); i ++) {
      result.append(schema.elementAt(i).toString());
      if ( i < schema.size() - 1)
	result.append(",");
    }
    result.append(")");

    if (tuples.isEmpty()) {
      //result.append(", tuples = empty");
    } else {
	result.append("\n");
	for (Iterator iter = tuples.iterator(); iter.hasNext();) {
	    Tuple tuple = (Tuple) iter.next();
	    result.append("\t" + tuple.toString() + "\n");
	}
    }
    return result.toString();
  }
}
