/* 
 * Rewriting.java
 * --------------
 * Records a rewriting.
 * $Id: Rewriting.java,v 1.5 2000/11/13 07:51:51 chenli Exp $
 */

import java.util.*;

class Rewriting {
  Query   rew  = null; // a rewriting is essentially a query
  HashSet mcds = null; // the corresponding mcds
  Mapping ec   = null; // the EC function
  HashSet viewTuples = null;

  /**
   * Creates a query given an id.
   */
  Rewriting(Query rew, HashSet mcds, Mapping ec) {
    this.rew  = rew;
    this.mcds = mcds;
    this.ec   = ec;
  }

  Rewriting(HashSet viewTuples, Query query) {
    this.viewTuples = viewTuples;

    Vector body = new Vector();  // body
    for (Iterator iter = viewTuples.iterator(); iter.hasNext();) {
      Tuple tuple = (Tuple) iter.next();
      body.add(new Subgoal(tuple.getName(), tuple.getArgs()));
    }
    
    rew = new Query(query.getName(), query.getHead(), body);
  }

  public HashSet getViewTuples() {
    return viewTuples;
  }

  public HashSet getMCDs() {
    return mcds;
  }

  public String toString() {
      /*    return "rew:\n   " + rew.toString() + "\n" +
      "ec:\n  " + ec + "\n" +
      "mcds:\n   " + mcds + "\n";*/
      return rew.toString();
  }
}
