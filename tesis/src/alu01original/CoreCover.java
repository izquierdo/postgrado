/* 
 * CoreCover.java
 * -------------
 * Implements our CoreCover algorithm.
 * $Id: CoreCover.java,v 1.25 2000/11/28 00:48:08 chenli Exp $
 */

import java.util.*;

class CoreCover {

  public static int numVTs = 0;
  public static long groupedNumVTs = 0;   // grouped view tuples
  public static int numMR = 0;
  public static int numGMR = 0;
  public static int sizeGMR = 0;
  public static long timeMR = 0;
  public static long timeFirstGMR = 0; // time when the 1st GMR is generated
  public static long timeAllGMR = 0;   // time when all GMRs are generated


  static long startTime = 0;
  static long endTime   = 0;

  // calls our CoreCover algorithm to generate rewritings
  static HashSet genPlan(Query query, Vector views) {

    numVTs = 0;
    numMR         = 0;
    numGMR        = 0;
    sizeGMR       = 0;
    timeMR        = 0;
    timeFirstGMR  = 0;
    timeAllGMR    = 0;  
    startTime     = 0;
    endTime       = 0;

    startTime  = System.currentTimeMillis();

    // minimize the query
    query = query.minimize();

    // construct the canonical db
    Database canDb = constructCanonicalDB(query);
    UserLib.myprintln("canDb = " + canDb.toString());

    // compute view tuples
    HashSet viewTuples = computeViewTuples(canDb, views);

    // compute tuple-cores
    computeTupleCores(viewTuples, query);

    // group view tuples according to their tuple-cores
    numVTs        = computeNumVTs(viewTuples);
    viewTuples    = groupViewTuples(viewTuples);
    groupedNumVTs = viewTuples.size();

    UserLib.myprintln("There are ***" + numVTs + 
		      "*** compressed view tuples = " + viewTuples);

    int newNum = viewTuples.size();
    //System.out.println("oldTupleNum = " + oldNum + " newTupleNum = " + newNum);
    UserLib.myprintln("viewTuples = " + viewTuples);
    // use tuple-cores to cover query subgoals
    HashSet rewritings = coverQuerySubgoals(viewTuples, query);
    UserLib.myprintln("rewritings = " + rewritings);

    if (!rewritings.isEmpty()) { // only count non-empty rewritings
      endTime = System.currentTimeMillis();
      timeMR = endTime - startTime;
    }

    return rewritings;
  }

  /**
   * constructs a canonical database of a query
   */
  static Database constructCanonicalDB(Query query) {
    HashSet tuples = new HashSet();

    for (Iterator iter = query.getBody().iterator(); iter.hasNext();) {
      Subgoal subgoal = (Subgoal) iter.next();
      Tuple tuple = new Tuple(subgoal);
      tuples.add(tuple);  // we treat each subgoal as a "tuple"
    }

    return new Database(tuples);
  }
    
 /**
  * given a canonical db and a set of views, computes the view tuples
  */
  static HashSet computeViewTuples(Database canDb, Vector views) {
    HashSet viewTuples = new HashSet();
    for (int i = 0; i < views.size(); i ++) {
      Query view = (Query) views.elementAt(i);

      Relation rel = canDb.execQuery(view);
      viewTuples.addAll(rel.getTuples()); // add them to the results
    }
    return viewTuples;
  }
  
  /**
   * computes the tuple cores of all view tuples.
   */
  static void computeTupleCores(HashSet viewTuples, Query query) {
    for (Iterator iter = viewTuples.iterator(); iter.hasNext();) {
      Tuple viewTuple = (Tuple) iter.next();
      computeTupleCore(viewTuple, query);
    }
  }

  /**
   * Computes the tuple core of a vew tuple, assuming the query is minimal.
   * find a maximum subset G of query subgoals such that there is a set
   * H of subgoals in t^exp, such that there is a mapping \mu from G to
   * H ,and the mapping has the three properties
   */ 
  static void computeTupleCore(Tuple viewTuple, Query query) {

    // view-tuple expansion
    Mapping phi          = viewTuple.getMapping();
    Query   view         = viewTuple.getQuery();
    Query   viewExp      = phi.apply(view);
    Vector  viewSubgoals = viewExp.getBody();
    HashSet vSubgoalSubsets = UserLib.genSubsets(viewSubgoals);
    
    // query subgoals
    Vector  querySubgoals   = query.getBody();
    HashSet qSubgoalSubsets = UserLib.genSubsets(querySubgoals);

    int upperBound = querySubgoals.size();
    if (upperBound > viewSubgoals.size())
      upperBound = viewSubgoals.size();

    // scans the subsets from the largests one to the smallest one
    for (int size = upperBound; size >= 1; size --) 
    {
      for (Iterator iter = qSubgoalSubsets.iterator(); iter.hasNext();) 
      {
	// for this "i"^th around, we consider only subsets with size i 
	HashSet qSubgoalSubset = (HashSet) iter.next();
	if (qSubgoalSubset.size() != size) 
	  continue;

	if (findCore(qSubgoalSubset, query, viewTuple, viewExp,
		     vSubgoalSubsets)) {
	  UserLib.myprintln("!!!!Find a core: " + qSubgoalSubset);
	  viewTuple.setCore(qSubgoalSubset);
	  return;
	}
      }
    }

    // empty core
    viewTuple.setCore(new HashSet());
  }

  /**
   * Given a set of query subgoals, a view tuple, and all subsets of the
   * subgoals in the tuple's expansion, checks if there is a mapping from
   * this query-subgoal subset to a view subgoal subset such that the
   * mapping has the "three properties."
   */
  static boolean findCore(HashSet qSubgoalSubset, Query query,
			  Tuple viewTuple, Query viewExp,
			  HashSet vSubgoalSubsets) {

    // change the set to a sequence
    Vector qSubgoalSeq = new Vector(qSubgoalSubset);
    
    for (Iterator iter1 = vSubgoalSubsets.iterator(); iter1.hasNext();) 
    {
      HashSet vSubgoalSubset = (HashSet) iter1.next();
      if (vSubgoalSubset.size() != qSubgoalSubset.size()) 
	continue;

      HashSet vSubgoalsPermus = UserLib.genPermutations(vSubgoalSubset);

      for (Iterator iter2 = vSubgoalsPermus.iterator(); iter2.hasNext();) 
      {
	Vector vSubgoalSeq = (Vector) iter2.next();
	if (testCoreMapping(qSubgoalSeq, query, vSubgoalSeq, viewExp)) 
	  return true; // found it!
      }
    }
    
    return false;
  }

  /**
   * Given a sequence of query subgoals and a sequence of view subgoals,
   * tests whether there is a mapping from the former to the latter that
   * satisifies the three properties in the definition of tuple-core.
   */
  static boolean testCoreMapping(Vector qSubgoalSeq, Query query,
				 Vector vSubgoalSeq, Query viewExp) {
    if (qSubgoalSeq.size() != vSubgoalSeq.size())
      UserLib.myerror("CoreCover.testCoreMapping(): wrong sequences");

    HashMap mu = new HashMap();
    for (int i = 0; i < qSubgoalSeq.size(); i ++) {
      Subgoal querySubgoal = (Subgoal) qSubgoalSeq.elementAt(i);
      Subgoal viewSubgoal  = (Subgoal) vSubgoalSeq.elementAt(i);

      if (!querySubgoal.isSameName(viewSubgoal))
	return false;

      Vector queryArgs = querySubgoal.getArgs();
      Vector viewArgs  = viewSubgoal.getArgs();
      if (queryArgs.size() != viewArgs.size()) 
	UserLib.myerror("CoreCover.testCoreMapping(), wrong args!");

      // checks if a query arg is unifiable with the corresponding view arg
      for (int j = 0; j < queryArgs.size(); j ++) {
	Argument queryArg = (Argument) queryArgs.elementAt(j);
	Argument viewArg  = (Argument)  viewArgs.elementAt(j);
	
	if (queryArg.isConst()) {
	  if (!viewArg.isConst()) // constant -> same constant
	    return false;
	  if (!queryArg.equals(viewArg))
	    return false;
	  continue;
	}

	// queryArg is a var
	if (viewExp.isDistVar(viewArg)) {
	  if (!queryArg.equals(viewArg))  // View D vars, identity
	    return false;
	  continue;
	}

	// viewArg is ND
	if (query.isDistVar(queryArg))  // NOT D (query) -> ND (view)
	  return false;

	// queryArg = ND, and viewArg = ND
	if (checkProperty2(queryArg, qSubgoalSeq, query))
	  return false;

	// remember the 1-to-1 mapping
	Argument image = (Argument) mu.get(queryArg);
	if (image == null)
	  mu.put(queryArg, viewArg);
	else {
	  if (!image.equals(viewArg)) // one-to-one
	    return false;
	}
      }
    }
    return true;
  }

  /**
   * checks the second property of the mapping. i.e., tests if all the
   * subgoals in the query that use a given queryArg appear in qSubgoalSeq.
   */
  static boolean checkProperty2(Argument queryArg, Vector qSubgoalSeq, 
				Query query) {

    Vector body = query.getBody();
    //UserLib.myprintln("body = " + body);

    for (int i = 0; i < body.size(); i ++) {
      Subgoal subgoal = (Subgoal) body.elementAt(i);
      Vector args = subgoal.getArgs();
      if (args.contains(queryArg) && !qSubgoalSeq.contains(subgoal))
	return false;
    }

    return true;
  }
  
  /**
   * Computes the tuple core of a vew tuple, assuming both the query and
   * the view are minimal already.
   * TODO: what if they are NOT minimal.
   */ 
  static void computeTupleCore_OLD(Tuple viewTuple, Query query) {
    Query   view = viewTuple.getQuery();
    
    // sets core to the target query subgoals under the mapping 
    HashSet core = viewTuple.getTargetSubgoals();
    UserLib.myprintln("before shrinking: core = " + core);

    // shrinking core
    Mapping phi = viewTuple.getMapping();
    HashMap reversedMap = new HashMap();
    // scans the args in phi
    Set entrySet = phi.getMap().entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument viewArg   = (Argument) mapEntry.getKey();
      Argument queryArg  = (Argument) mapEntry.getValue();

      if (removeable(core, phi, reversedMap, view, viewArg, query, queryArg)) {
	// finds a conflict, remove all query subgoals that use this arg
	core = removeConflictSubgoals(core, queryArg);
      } else {
	reversedMap.put(queryArg, viewArg); // reverses the mapping
      }
    }    

    UserLib.myprintln("Final core = " + core);
    viewTuple.setCore(core);
  }

  /*
   * checks if some subgoals should be removed from a core
   */
  static boolean removeable(HashSet core, Mapping phi, HashMap reversedMap,
			    Query view, Argument viewArg, 
			    Query query, Argument queryArg) {

    // Removes subgoals if one of the following conditions is satisfied:
    Argument otherViewArg = (Argument) reversedMap.get(queryArg);
    if (otherViewArg != null && !otherViewArg.equals(viewArg)) {
      // we have a different view arg mapped to this query arg
      Argument image = null;
      if (view.isDistVar(otherViewArg))
	image = phi.apply(otherViewArg); // ?????
      else
	image = otherViewArg;
    
      if (image == null)
	UserLib.myerror("CoreCover.removeable(): image is missing, 1.");
      if (!image.equals(queryArg)) // they should have the same image
	return true;
    }

    // 1. viewArg = "D", not removed
    if (view.isDistVar(viewArg))
      return false;

    // 2. viewArg = "ND" and queryArg = "D"
    if (query.isDistVar(queryArg)) {
      //System.out.println("case 2: queryArg = D, viewArg = ND");
      return true;
    }

    // 3. both are "ND", and some query subgoals that use this query arg
    //    are not in the core
    Vector body = query.getBody();
    for (int i = 0; i < body.size(); i ++) {
      Subgoal subgoal = (Subgoal) body.elementAt(i);
      if (subgoal.getArgs().contains(queryArg) && !core.contains(subgoal))
	return true;
    }

    // 1. two variables in the view are mapped to
    //    the same variable Y in the query, then remove all query
    //    subgoals that use this variable Y    
    /*Argument otherViewArg = (Argument) reversedMap.get(queryArg);
    if (otherViewArg != null && !otherViewArg.equals(viewArg)) {

      Argument image = null;
      if (view.isDist(otherViewArg))
	image = phi.get(otherViewArg); // ?????
      else
	image = otherViewArg;

      if (image == null)
	UserLib.myerror("CoreCover.removeable(): image is missing, 1.");
      if (!image.equals(queryArg)) // they should have the same image
	return true;
    }*/
    
    return false;
  }

  /**
   * removes the subgoals that use a query arg from a core
   */
  static HashSet removeConflictSubgoals(HashSet core, Argument queryArg) {
    HashSet result = new HashSet();

    for (Iterator iter = core.iterator(); iter.hasNext();) {
      Subgoal subgoal = (Subgoal) iter.next();

      // checks if this subgoal uses this query arg
      if (!subgoal.getArgs().contains(queryArg)) 
	result.add(subgoal);
      else {
	UserLib.myprintln("Remove subgoal " + subgoal);
      }
    }
    
    return result;
  }

  static HashSet coverQuerySubgoals(HashSet viewTuples, Query query) {
    HashSet rewritings = new HashSet();
    
    // considers all the subsets in the descending order
    //HashSet tupleSubsets = UserLib.genSubsets(viewTuples);
    //UserLib.myprintln("after UserLib.genSubsets()");

    sizeGMR = 0;
    numMR   = 0;
    numGMR  = 0;

    int upperBound = viewTuples.size();
    if (upperBound >= query.getSubgoalNum()) // according to LMSS95
      upperBound = query.getSubgoalNum();

    // if the union of the tuple-cores doesn't cover all query subgoals,
    // just return no rewriting
    if (!unionCoverSubgoals(viewTuples, query))
	return rewritings;

    // scans the subsets from the smallest one to the largest one
    boolean found = false;
    for (int size = 1; (size <= upperBound) && !found; size ++) 
    {
      // considers subsets with the current size 
      //System.out.println("# of view tuples = " + viewTuples.size() +
      //" size = " + size);
      HashSet tupleSubsets = UserLib.genSubsets(viewTuples, size);
      //System.out.println("# of tupleSubsets = " + tupleSubsets.size());

      for (Iterator iter = tupleSubsets.iterator(); iter.hasNext();) 
      {
	// for this "i"^th around, we consider only subsets with size i 
	HashSet tupleSubset = (HashSet) iter.next();
	if (tupleSubset.size() != size) 
	  UserLib.myerror("CoreCover.coverQuerySubgoals(), error!");

	if (unionCoverSubgoals(tupleSubset, query))  {// found one
	  rewritings.add(new Rewriting(tupleSubset, query));
	  sizeGMR = size;
	  numGMR ++;
	  numMR ++;
	  /*System.out.println("\n tupleSubset " + tupleSubset +
			     " query = " + query + "\n");*/
	  
	  // time when the 1st GMR is generated
	  if (!found) {
	    endTime = System.currentTimeMillis();
	    timeFirstGMR = endTime - startTime;
	  }

	  found = true;
	}
      }

      if (found) {
	endTime = System.currentTimeMillis();
	timeAllGMR = endTime - startTime;
      }
    }

    return rewritings;
  }

  // checks if the union of tuple cores covers all query subgoals
  static boolean unionCoverSubgoals(HashSet tupleSubset, Query query) {
    HashSet coreUnion = new HashSet();
    for (Iterator iter2 = tupleSubset.iterator(); iter2.hasNext();) {
      Tuple viewTuple = (Tuple) iter2.next();
      HashSet core = viewTuple.getCore();
      coreUnion.addAll(core);
    }

    return coreUnion.containsAll(query.getBody());
  }

  /**
   * Checks if a rewriting has a used a subset of view tuples in tupleSubset.
   */
  static boolean containsTupleSubset(HashSet rewritings, HashSet tupleSubset) {
    for (Iterator iter = rewritings.iterator(); iter.hasNext();) {
      Rewriting rewriting = (Rewriting) iter.next();
      HashSet viewTuples = rewriting.getViewTuples();
      if (tupleSubset.containsAll(viewTuples))
	return true;
    }
    return false;
  }


  /**
   * groups different views into equivalence classes
   */
  static HashSet groupViewTuples(HashSet viewTuples) {
    HashSet groups = new HashSet();

    for (Iterator iter = viewTuples.iterator(); iter.hasNext();) {
      Tuple viewTuple = (Tuple) iter.next();
      
      // search for view tuples with the same utple-core
      boolean found = false;
      for (Iterator iter2 = groups.iterator(); iter2.hasNext();) {
	Tuple group = (Tuple) iter2.next();
	if ((group != viewTuple) && group.sameCore(viewTuple)) {
	  found = true;
	  group.addEquTuple(viewTuple);
	  break;
	}
      }

      if (!found) { // new equivalence class
	viewTuple.setEquSelf();
	groups.add(viewTuple);
      }
    }
    
    return groups;
  }

  // computes the number of view tuples by considering the equivalence
  // view classes: computes the total.
  static int computeNumVTs(HashSet viewTuples) {
    int num = 0;
    for (Iterator iter = viewTuples.iterator(); iter.hasNext();) {
      Tuple tuple = (Tuple) iter.next();
      num += tuple.getQuery().getCount();  
    }

    return num;
  }
}
