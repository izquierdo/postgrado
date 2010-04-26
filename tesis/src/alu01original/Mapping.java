/* 
 * Mapping.java
 * -------------
 *
 * Records a mapping from a set of arguments to another set of arguments.
 * For the 
 *
 * $Id: Mapping.java,v 1.14 2000/11/17 01:35:38 chenli Exp $
 */

import java.util.*;

class Mapping {
  Map    map = null;  // recording the map

  Mapping() {
    map = new HashMap();
  }

  Mapping(Map map) {
    this.map = map;
  }

  public Map getMap() {
    return map;
  }

  /**
   * Clones this mapping.
   */
  public Object clone() {
    HashMap newMap = new HashMap();

    Set entrySet = (Set) map.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument srcArg = (Argument) mapEntry.getKey();
      Argument dstArg = (Argument) mapEntry.getValue();
      newMap.put(srcArg, dstArg);
    }

    return new Mapping(newMap);
  }

  /**
   * Returns a vector of the source args.
   */
  public Vector getSrcArgs() {
    Vector srcArgs = new Vector();

    Set entrySet = (Set) map.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument srcArg = (Argument) mapEntry.getKey();
      srcArgs.add(srcArg);
    }

    return srcArgs;
  }

  /**
   * Returns a vector of the dst args.
   */
  public Vector getDstArgs() {
    Vector dstArgs = new Vector();

    Set entrySet = (Set) map.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument dstArg = (Argument) mapEntry.getValue();
      dstArgs.add(dstArg);
    }

    return dstArgs;
  }

  /**
   * Inserts a new pair of argument
   */
  public void put(Argument srcArg, Argument dstArg) {
    if (map.get(srcArg) != null)
      UserLib.myerror("Mapping.put(), srcArg already exists.");

    map.put(srcArg, dstArg);
  }

  /**
   * Extends this mapping: for the variables in args that are not
   * mapped in the map, we map them to themselves.  Note that args should
   * be a superset of srcArgs and dstArgs
   * 
   * This operation will set srcArgs and dstArgs to args
   */
  /*void extendToIndentity(Vector args) {
    Vector srcArgs = getSrcArgs();

    // check if args is a superset of srcArgs
    for (int i = 0; i < srcArgs.size(); i ++) {
      // if this arg is not mapped, map it to itself
      Argument arg = (Argument) srcArgs.elementAt(i);
      // !!!if (arg.isConst()) continue; // we don't check constants
      if (!args.contains(arg)) 
      UserLib.myerror("Mapping.extendToIndentity(): " +
      "args should be a superset of srcArgs.");
    }

    for (int i = 0; i < args.size(); i ++) {
      // if this arg is not mapped, maps it to itself
      if (!map.containsKey(args.elementAt(i)))
	map.put(args.elementAt(i), args.elementAt(i));
    }
  }*/

  /**
   * Applies the mapping on an argument to get a new argument. if the
   argument is mapped, use the identity mapping.
   */
  public Argument applyTryIdentity(Argument srcArg) {
    Argument dst = this.apply(srcArg);
    if (dst != null) return dst;
    return srcArg;  // identity
  }

  /**
   * Applies the mapping on an argument to get a new argument
   */
  public Argument apply(Argument srcArg) {
    Argument dst = (Argument) map.get(srcArg);
    return dst;
  }

  /**
   * Applies the mapping on a subgoal to get a new subgoal
   */
  public Subgoal apply(Subgoal srcSubgoal) {
    // the only difference is the list of arguments
    Vector args      = srcSubgoal.getArgs();

    // applies on each argument
    Vector newArgs = new Vector();
    for (int i = 0; i < args.size(); i ++) {
      Argument srcArg = (Argument) args.elementAt(i);
      Argument dstArg = this.applyTryIdentity(srcArg);
      
      newArgs.add(dstArg);
    }

    return new Subgoal(srcSubgoal.getName(), newArgs);
  }

 /**
  * Applies the mapping on a query and gets a new query
  */
  public Query apply(Query srcQuery) {
    // the only difference is the head and the body
    Vector     body      = srcQuery.getBody();

    // applies the head on the head
    Subgoal newHead   = this.apply(srcQuery.getHead());
    /* TODO
    for (int i = 0; i < head.size(); i ++) {
      Argument srcArg = (Argument) head.elementAt(i);
      Argument dstArg = this.apply(srcArg);
      if (dstArg == null) 
      UserLib.myerror("Mapping.apply(query): dstArg should not be null.");

      newHead.add(dstArg); // a new argument
    }*/
    
    // applies the hh on the body
    Vector newBody = new Vector(); 
    for (int i = 0; i < body.size(); i ++) {
      Subgoal srcSubgoal = (Subgoal) body.elementAt(i);
      Subgoal dstSubgoal = this.apply(srcSubgoal);
      
      if (dstSubgoal == null) 
	UserLib.myerror("Mapping.apply(query): " +
			" dstSubgoal should not be null.\n" +
			"srcSubgoal is: " + srcSubgoal.toString());
      newBody.add(dstSubgoal);
    }
    
    return (new Query(srcQuery.getName(), newHead, newBody));
  }
  
  /**
   * returns a new mapping accoroding to a renaming
   */
  public Mapping rename(Map renameMap) {
    // changes the mapping
    Map    newMap = new HashMap();
    Set entrySet  = (Set) map.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument srcArg = (Argument) mapEntry.getKey();
      Argument dstArg = (Argument) mapEntry.getValue();
      // new args
      Argument newSrcArg = (Argument) map.get(srcArg);
      Argument newDstArg = (Argument) map.get(dstArg);

      if (newSrcArg == null || newDstArg == null) 
	UserLib.myerror("Mapping.rename(), we should find the targets.");
      // add this pair
      newMap.put(newSrcArg, newDstArg);
    }

    return new Mapping(newMap);
  }

  /**
   * Given an argument, finds the tail argument on the list starting from
   * the head argument.
   */
  public Argument getTail(Argument headArg) {
    Argument tailArg = headArg;
    while (map.get(tailArg) != null)
      tailArg = (Argument) map.get(tailArg);
    return tailArg;
  }

  /**
   * Returns a string.
   */
  public String toString() {
    StringBuffer result = new StringBuffer();

    Set entrySet = (Set) map.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument srcArg = (Argument) mapEntry.getKey();
      Argument dstArg = (Argument) mapEntry.getValue();
      result.append(srcArg.toString() + "->" + dstArg.toString());
      if (iter.hasNext()) result.append(", ");
    }

    return result.toString();
  }
}
