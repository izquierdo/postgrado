/* 
 * Subgoal.java
 * -------------
 * $Id: Subgoal.java,v 1.14 2000/11/25 02:34:37 chenli Exp $
 */

import java.util.*;

class Subgoal {
  String name = null;  // subgoal name
  Vector args = null;  // a list of arguments

  Subgoal(String name, Vector args) {
    this.name = name;
    this.args = args;
  }

  Subgoal(Vector relations, int relIndex, Vector argDom) {

    args = new Vector();  // a list of variables    
    Relation relation = ((Relation) relations.elementAt(relIndex));
    name = relation.getName();
    int argNum = relation.getAttrNum();
    for (int i = 0; i < argNum; i ++) {
      // randomly picks one argument
      int argIndex = GoodPlan.random.nextInt(argDom.size());
      Argument arg = (Argument) argDom.elementAt(argIndex);
      args.add(arg);
    }
  }

  /**
   * Checks if two subgoals have the same name
   */
  public boolean isSameName(Subgoal subgoal) {
    return this.getName().equalsIgnoreCase(subgoal.getName());
  }

  /**
   * renames the arguments in this subgoal given a new id Notice
   * after this steps, two subgoals can share the same argument
   * which is stored in two argument objects.  returns a new subgoal.
   */
  public Subgoal rename(int newId, Map argMap) {

    Vector newArgs = new Vector();
    for (int i = 0; i < args.size(); i ++) {
      Argument oldArg = (Argument) args.elementAt(i);
      Argument newArg = oldArg.rename(newId, argMap);
      if (!argMap.containsKey(oldArg))
	argMap.put(oldArg, newArg);
      
      newArgs.add(newArg);
    }
    
    return new Subgoal(this.getName(), newArgs);
  }
  
  public Vector getArgs() {
    return args;
  }

  public String getName() {
    return name;
  }

  public int size() {
    return args.size();
  }

  public boolean contains(Argument arg) {
    return args.contains(arg);
  }

  /**
   * given a partial mapping, tests if this subgoal can be mapped to
   * another mapped by extending the subgoal.
   */
  public boolean unifiable(Subgoal subgoal, Mapping cm, boolean checkName) {

    if (checkName && !isSameName(subgoal))
      return false;

    Vector srcArgs = this.getArgs();
    Vector dstArgs = subgoal.getArgs();
    if (srcArgs.size() != dstArgs.size()) 
      UserLib.myerror("Subgoal.unifiable(), wrong args!");
    
    // checks if a src arg is unifiable with the corresponding dst arg
    for (int i = 0; i < srcArgs.size(); i ++) {
      Argument srcArg = (Argument) srcArgs.elementAt(i);
      Argument dstArg = (Argument) dstArgs.elementAt(i);

      if (srcArg.isConst()) {  // constant -> same constant
	if (dstArg.isConst() && dstArg.equals(srcArg)) {
	  cm.put(srcArg, dstArg);  // const -> const
	  continue;
	}
	return false;
      }

      Argument image = (Argument) cm.apply(srcArg);
      if (image == null) {
	cm.put(srcArg, dstArg);  // var -> var/const
      }
      else {
	if (!image.equals(dstArg)) 
	  return false;
      }
    }

    return true;
  }

  public String toString() {
    StringBuffer result = new StringBuffer(name);
    result.append("(");
    for (int i = 0; i < args.size(); i ++) {
      result.append(args.elementAt(i).toString());
      if ( i < args.size() - 1)
	result.append(",");
    }
    result.append(")");
    
    return result.toString();
  }
}
