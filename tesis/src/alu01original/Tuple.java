/* 
 * Tuple.java
 * -------------
 * $Id: Tuple.java,v 1.8 2000/11/23 22:22:44 chenli Exp $
 */

import java.util.*;

class Tuple {
  String name = null;
  Vector args = null;  // a list of arguments
  Mapping phi = null;  // mapping from query args to the database deriving
		       // the tuple
  Subgoal subgoal = null; // the subgoal that produces this tuple in canDb.
  HashMap mapSubgoals = null;
  Query   query = null;
  HashSet core = new HashSet();

  HashSet equTuples = new HashSet(); // set of view tuples with the same tuple-core

  Tuple(Subgoal subgoal) {
    this.subgoal = subgoal;
    this.name = subgoal.getName();
    this.args = (Vector) subgoal.getArgs().clone();
  }

  Tuple(String name, Vector args, Mapping phi, HashMap mapSubgoals) {
    this.name = name;
    this.args = args;
    this.phi  = phi;
    this.mapSubgoals = mapSubgoals;
  }

  public String getName() {
    return name;
  }

  public Vector getArgs() {
    return args;
  }

  public Mapping getMapping() {
    return phi;
  }

  public Query getQuery() {
    return query;
  }

  public Subgoal getSubgoal() {
    return subgoal;
  }

  public HashMap getMapSubgoals() {
    return mapSubgoals;
  }

  // set the query of the tuple, and compute the head of each tuple
  public void setQuery(Query query) {
    this.query = query;
    this.name = query.getName();

    // compute the new head under the phi
    Subgoal head = query.getHead();
    if (head.size() != args.size())
      UserLib.myerror("Tuple.setQuery(), wrong args!");
    Subgoal newHead = phi.apply(head);
    args = newHead.getArgs();  // set the new args
  }

  public void setCore(HashSet core) {
    this.core = core;
  }

  public HashSet getCore() {
    return core;
  }

  /**
   * gets the target query subgoals under the mapping 
   */
  public HashSet getTargetSubgoals() {
    HashSet result = new HashSet();

    Set entrySet = (Set) mapSubgoals.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Subgoal querySubgoal = (Subgoal) mapEntry.getValue();
      result.add(querySubgoal);
    }

    return result;
  }

  /**
   * Indicates whether some other tuple is "equal to" this one.
   * Notice that the parameter must be an "Object" class in order to
   * override Object.equals(), not an "Tuple" class.
   *
   * @param argument The reference object with which to compare.
   * @return <code>true(false)</code> if this item is the same(not same) as 
   * the argument.
   */
  public boolean equals(Object tuple){
    Tuple t = (Tuple) tuple;

    // same name
    if (!this.getName().equals(t.getName())) return false;

    Vector tArgs = t.getArgs();
    if (args.size() != tArgs.size()) return false;

    for (int i = 0; i < args.size(); i ++) {
	Argument arg  = (Argument) args.elementAt(i);
	Argument tArg = (Argument) tArgs.elementAt(i);
	if (!arg.equals(tArg)) return false;
    }

    return true;
  }

  /**
   *
   */
  public boolean sameCore(Tuple viewTuple) {
    return this.getCore().equals(viewTuple.getCore());
  }

  /**
   * adds itself to the equivalence class
   */
  public void setEquSelf() {
    equTuples.add(this); // add itself
  }

  /**
   * adds a view tuple to the equivalence class
   */
  public void addEquTuple(Tuple viewTuple) {
    equTuples.add(viewTuple);
  }
  
  /**
   * Overrides the hash function Object.hasCode().  As an item in a
   * HashSet collection, called when HashSet.equals() is called.
   */
  public int hashCode() {
    int hashCode = this.getName().hashCode();
    for (int i = 0; i < args.size(); i ++) {
      Argument arg  = (Argument) args.elementAt(i);
      hashCode += arg.hashCode();
    }
    
    return hashCode;
  }

  public String toString() {
    StringBuffer result = new StringBuffer();
    if (name != null)
      result.append(name);
    result.append(args.toString());

    /*if (phi != null)
      result.append("; " + phi.toString());*/
    return (result.toString());
  }
}
