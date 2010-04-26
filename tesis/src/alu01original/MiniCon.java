/* 
 * MiniCon.java
 * -------------
 * Implements the MiniCon algorithm.
 * $Id: MiniCon.java,v 1.33 2000/11/17 01:35:39 chenli Exp $
 *
 */

import java.util.*;

class MiniCon {

   public static double timeFormMCD;
   public static double timeCombineMCD;
   public static int numMCDs;
   public static int numRews;
    public static Prof prof;
      
  // calls the MiniCon algorithm to generate rewritings for the query
  static HashSet genPlan(Query query, Vector views) {
      long startTime = 0;
      long endTime   = 0;     
      
      prof = new Prof("MCDs");
      prof.start();
      HashSet mcds = formMCDs(query, views);
      prof.stop();
      //HashSet rewritings = new HashSet(); 
      HashSet rewritings = combineMCDs(query, mcds);
      prof.stop2();
      timeFormMCD    = (prof.stopcputime  - prof.startcputime)/1000.0;
      
      timeCombineMCD = (prof.stopcputime2 - prof.stopcputime)/1000.0 ;
      //minimizeRews();
      numMCDs = mcds.size();
      numRews = rewritings.size();
      return rewritings;
    
  }


  // calls the MiniCon algorithm to generate rewritings for the query
    static HashSet genPlan2(String type, Query query, Vector views) {
      long startTime = 0;
      long endTime   = 0;     
      
      prof = new Prof("MCDs");
      prof.start();
      HashSet mcds = formMCDs(query, views);
      System.out.println( mcds);
      System.out.println("query "+ query);
      prof.stop();

      numMCDs = mcds.size();

      HashSet rewritings = new HashSet(); 
      if (type.equals("RW")){
          rewritings = combineMCDs(query, mcds);
          prof.stop2();
          timeCombineMCD = (prof.stopcputime2 - prof.stopcputime)/1000.0 ;
          numRews = rewritings.size();
      }

      timeFormMCD    = (prof.stopcputime  - prof.startcputime)/1000.0;
      
      //minimizeRews();
      return rewritings;
    
  }


  // generates all MCDs for a query and views
  static HashSet formMCDs(Query query, Vector views) {
    UserLib.myprintln("calling formMCDs()...");
    HashSet mcds = new HashSet();

    for (int i = 0; i < views.size(); i ++) {
      Query view = (Query) views.elementAt(i);
      mcds.addAll(formMCDs(query, view));
    }

    return mcds;
  }

  // generates all MCDs for a query and views
  static HashSet formMCDs(Query query, Query view) {
    UserLib.myprintln("calling formMCDs(), view = " + view);
    HashSet mcds = new HashSet();

    Vector querySubgoals = (Vector) query.getBody();

    // generates all subsets of the query subgoals except the given one
    HashSet querySubsets = UserLib.genSubsets(querySubgoals);

    // scans the subsets from the smallest one to the largest one
    for (int i = 1; i <= querySubgoals.size(); i ++) 
    {
      for (Iterator iter = querySubsets.iterator(); iter.hasNext();) 
      {
	// for this "i"^th around, we consider only subsets with size i 
	HashSet querySubgoalSubset = (HashSet) iter.next();
	if (querySubgoalSubset.size() != i) 
	  continue;
	//System.out.println("query-subgoal subset = " + querySubgoalSubset);
	
	// tries to extend this subset ONLY IF it is not a superset of
	// another existing subset in the found MCDs
	if (containsQuerySubgoalSubset(mcds, querySubgoalSubset)) 
	  continue;

	// we should rename the view and adjust the PhiHH
	Query renamedView = view.rename();
	//System.out.println("after renaming, view = " + renamedView);
	HashSet newMCDs = extend(query, querySubgoalSubset, renamedView);
	// it IS extendable.  finally, we got some mcds
	mcds.addAll(newMCDs);  
      }
    }

    return mcds;
  }

  /**
   * Given a query subgoal and a view subguoa, tries to find an HH of the
   * view, under which there is a mapping phi from the query subgoal to
   * the view subgoal.
   */
  private static PhiHH unifiable(Query query, int querySubgoalIndex,
				 Query view, int viewSubgoalIndex) {

    Subgoal querySubgoal = query.getSubgoal(querySubgoalIndex);
    Subgoal viewSubgoal  =  view.getSubgoal(viewSubgoalIndex);
    
    // they must use the same name
    if (!querySubgoal.isSameName(viewSubgoal))
      return null;

    Vector queryArgs = querySubgoal.getArgs();
    Vector viewArgs  = viewSubgoal.getArgs();
    if (queryArgs.size() != viewArgs.size()) 
      UserLib.myerror("MiniCon.unifiable(), wrong args!");

    Mapping phi = new Mapping();
    Mapping hh  = new Mapping();
    // checks if a query arg is unifiable with the corresponding view arg
    for (int i = 0; i < queryArgs.size(); i ++) {
      Argument queryArg = (Argument) queryArgs.elementAt(i);
      Argument viewArg  = (Argument)  viewArgs.elementAt(i);

      if (!queryArg.unifiable(viewArg, phi, hh, view.getDistVars()))
	return null;
    }

    return (new PhiHH(phi, hh));
  }

    // for (i = 1; i < number of query's subgoals; i ++)
    // {
    //   for each superset querySuperset of {querySubgoal} with size i 
    //   {
    //    if querySuperset can be mapped to some view subgoals while
    //       the mapping is consistent with phihh, and the mapping satisfies
    //       the PROPERTY
    //     {
    //      if this superset is not a superset of another existing 
    //        superset in moreMCDs
    //         {
    //           form an MCD and add
    //         }
    //     }
    //   }
    // }
  /**
   * checks if a subset of query's subgoals can be mapped to some view
   * subgoals while the mapping is consistent with a given phihh from a
   * given query subgoal to a given view subgoal, and the mapping
   * satisfies the PROPERTY.
   * If so, returns the set of new MCDs.
   */
  private static HashSet extend(Query query, HashSet querySubgoalSubset,
				Query view) {
    
    HashSet mcds = new HashSet();

    // translates the querySubgoalSubset to a vector
    Vector querySubgoalSeq = new Vector();
    for (Iterator iter = querySubgoalSubset.iterator(); iter.hasNext();) {
      querySubgoalSeq.add(iter.next());
    }

    // clones the view's body
    Vector viewSubgoals = (Vector) view.getBody();
    // generates all subsets of the view subgoals except the given one
    HashSet viewSubsets = UserLib.genSubsets(viewSubgoals);
    //System.out.println("viewSubsets = " + viewSubsets);
    
    // scans the subsets from the smallest one to the largest one
    for (int i = 1; i <= viewSubgoals.size(); i ++) 
    {
      if (i > querySubgoalSubset.size()) 
	break;

      for (Iterator iter = viewSubsets.iterator(); iter.hasNext();) 
      {
	// for this "i"^th around, we consider only subsets with size i 
	HashSet viewSubgoalSubset = (HashSet) iter.next();
	if (viewSubgoalSubset.size() != i) 
	  continue;
	//System.out.println("view-subgoal subset = " + viewSubgoalSubset);

	// tries to extend the view-subgoal subset ONLY IF it is not a
	// superset of another existing view-subgoal subset in the found MCDs
	if (containsViewSubgoalSubset(mcds, viewSubgoalSubset)) continue;

	// considers this pair of query-subgoal subset and view-subgoal subset
	MCD mcd = formOneMCD(query, querySubgoalSeq,
			     view,  viewSubgoalSubset);

	if (mcd != null) mcds.add(mcd);
      }
    } 

    return mcds;
  }

  /**
   * checks if one query-subgoal subset is a superset of another existing 
   * query-subgoal subset in a given set of MCDs.
   */
  private static boolean containsQuerySubgoalSubset(HashSet mcds, 
						    HashSet querySubgoals) {
    for (Iterator iter = mcds.iterator(); iter.hasNext();) {
      MCD                  mcd = (MCD) iter.next();
      HashSet mcdQuerySubgoals = mcd.getMappedQuerySubgoals();
      // checks if the given set of subgoals is a super set of the
      // subgoals in this MCD. !!! make sure the sets are using references
      // to the same set of query subgoals
      if (querySubgoals.containsAll(mcdQuerySubgoals))
	return true;
    }
    
    return false;
  }

  /**
   * checks if one query-subgoal subset is a superset of another existing 
   * query-subgoal subset in a given set of MCDs.
   */
  private static boolean containsViewSubgoalSubset(HashSet mcds, 
						   HashSet viewSubgoals) {
    for (Iterator iter = mcds.iterator(); iter.hasNext();) {
      MCD                 mcd = (MCD) iter.next();
      HashSet mcdViewSubgoals = mcd.getViewSubgoals();
      // checks if the given set of subgoals is a super set of the
      // subgoals in this MCD. !!! make sure the sets are using references
      // to the same set of query subgoals
      if (viewSubgoals.containsAll(mcdViewSubgoals)) // <== !!! renamed
	return true;
    }
    
    return false;
  }
  
  /**
   * Given a pair of query-subgoal subset and view-subgoal subset, 
   * tries to extend them to form an MCD.  Note that ALL the subgoals in
   * the two subsets must be used.
   */
  private static MCD formOneMCD(Query   query, 
				Vector  querySubgoalSeq,
				Query   view,  
				HashSet viewSubgoalSubset) {
    
    /*System.out.println("formOneMCD(): querySubgoalSeq = " + 
		       querySubgoalSeq + " viewSubgoalSubset " +
		       viewSubgoalSubset);*/
    // the view-subgoal subest cannot have more subgoals than the
    // query-subgoal subset
    if (viewSubgoalSubset.size() > querySubgoalSeq.size())
      return null;
    
    // generates all the sequences of length querySubgoalSubset.size() for
    // the view subgoals  which each view subgoal should appear at least
    // once.
    Vector viewSubgoalSeqs = 
      UserLib.getAllSeqs(viewSubgoalSubset, querySubgoalSeq.size());

    //System.out.println("formOneMCD(): viewSubgoalSeqs = " + viewSubgoalSeqs);
    if (viewSubgoalSeqs == null) return null;
    
    for (int i = 0; i < viewSubgoalSeqs.size(); i ++) {
      // tries to create an MCD for this pair of sequences of query
      // subgoals and view subgoals
      Vector viewSubgoalSeq = (Vector) viewSubgoalSeqs.elementAt(i);
      MCD mcd = buildMCD(query, querySubgoalSeq, view, viewSubgoalSeq);

      // returns once if we find one MCD
      if (mcd != null) return mcd;
    }

    return null;
  }

  private static MCD buildMCD(Query   query, 
			      Vector  querySubgoalSeq,
			      Query   view,  
			      Vector  viewSubgoalSeq) {

    // two vectors should have the same size
    if (querySubgoalSeq.size() != viewSubgoalSeq.size()) 
      UserLib.myerror("MiniCon.buildMCD(): error.  The two " +
		      "vectors should have the same size.");

    // clones a new phihh
    Mapping phi = new Mapping();
    Mapping hh  = new Mapping();

    // scans the subgoals to match
    for (int i = 0; i < querySubgoalSeq.size(); i ++) {
      Subgoal querySubgoal = (Subgoal) querySubgoalSeq.elementAt(i);
      Subgoal viewSubgoal  = (Subgoal) viewSubgoalSeq.elementAt(i);
      
      // they should have the same name
      // !!! make sure "equals" works
      if (!querySubgoal.getName().equals(viewSubgoal.getName()))
	return null;
      
      Vector queryArgs = querySubgoal.getArgs();
      Vector viewArgs  = viewSubgoal.getArgs();
      
      // they should have the same number of subgoals
      if (viewArgs.size() != queryArgs.size()) 
	UserLib.myerror("MiniCon.buildmCD(): error.  The two " +
			"subgoals should have the same size.");
      
      // checks if a query arg is unifiable with the corresponding view arg
      for (int j = 0; j < queryArgs.size(); j ++) {
	Argument queryArg = (Argument) queryArgs.elementAt(j);
	Argument viewArg  = (Argument)  viewArgs.elementAt(j);
	
	if (!queryArg.unifiable(viewArg, phi, hh, view.getDistVars()))
	  return null;
      }
    }

    // transforms vectors to sets
    HashSet querySubgoalSubset = new HashSet(querySubgoalSeq);
    HashSet viewSubgoalSubset  = new HashSet(viewSubgoalSeq);

    // checks if the mapping satisfies the PROPERTY
    if (satisfyProperty(phi, hh, query, querySubgoalSubset, view)) 
      { // finally we got one!
	MCD mcd = new MCD(query, 
			  querySubgoalSubset, 
			  hh.apply(view),   // applies hh here!
			  viewSubgoalSubset, 
			  phi, hh); // hh has been extended

	UserLib.myprintln("WE FOUND ONE MCD! ==> \n" + mcd);
	return mcd;
      }

    return null;
  }

  /**
   * Checks if a mapping satisfies the PROPERTY.
   */
  private static boolean satisfyProperty(Mapping phi, Mapping hh,
					 Query query, 
					 HashSet querySubgoalSubset, 
					 Query view) {
    /*System.out.println("In  satisfyProperty()!");
    System.out.println("phi = " + phi + "\n" + " hh = " + hh + 
		       "query = " + query + "\n" +
		       "querySubgoalSubset = " + querySubgoalSubset + "\n" +
		       "view = " + view);*/

    Map map = phi.getMap();
    Set entrySet = (Set) map.entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument  queryArg = (Argument) mapEntry.getKey();
      Argument  viewArg  = (Argument) mapEntry.getValue();

      /*System.out.println("queryArg = " + queryArg + 
			 " viewArg = " + viewArg );*/

      // it's OK if the view arg is distinguished
      if (view.isDistVar(viewArg)) continue;

      // the view arg is not distinguished,
      // then the query arg must be distinguished 
      if (query.isDistVar(queryArg)) {
	UserLib.myprintln("Case 1 failed.");
	return false;
      }

      // queryArg is not distinguished, and viewArg is not either, then
      // all the query subgoals that use this var should be mapped, i.e.,
      // they are in querySubgoalSubset
      for (int i = 0; i < query.getSubgoalNum(); i ++) {
	Subgoal querySubgoal = query.getSubgoal(i);
	if (querySubgoal.contains(queryArg)  // this subgoal uses this var
	    && !querySubgoalSubset.contains(querySubgoal)) {
	  UserLib.myprintln("Case 2 failed.");
	  UserLib.myprintln("querySubgoal = " + querySubgoal +
			     "querySubgoalSubset = " + querySubgoalSubset);
	  return false;
	}
      }
    }
    
    return true;
  }

    /*
    for each subgoal g in Q
      For view V in \V and every subgoal v_i in V
      Let h be the least restrictive head homomorphism on V
        such that there exists a mapping \phi, s.t., \phi(g) = h(v).
      If h and \phi exist, then add to C any new MCD C 
        that can be constructed where:
         (a) \phi_C (resp. h_C) is an extension of \phi (res. h);
         (b) G_C is the minimal subset of subgoals of Q such that
             G_C,\phi)C and h_C satisfy Peoperty I, and
         (c) it is not possible to extend \phi and h to an MCD that
             covers fewer subgoals than G_C.
      Return C.
    */       

  /** 
   * Step 2: combines MCDs to generate rewritings.
   */


  /*  static HashSet combineMCDs(Query query, HashSet mcds) {

    // computes the EC function
    Mapping ec = new Mapping();
    for (Iterator iter = mcds.iterator(); iter.hasNext();) {
      MCD mcd = (MCD) iter.next();

      Mapping ec = computeECPsi(ec, mcd);
    }

    return genRewritings(query, mcds, ec);
  }
*/
  /**
   * Given a set of MCDs (with their psi functions computed) and an EC,
   * generates rewritings by trying all possible combinations of these MCDs.
   */
  static HashSet combineMCDs(Query query, HashSet mcds) {
    UserLib.myprintln("calling combineMCDs()...");
    HashSet rewritings = new HashSet();
    HashSet mcdSubsets = UserLib.genSubsets(mcds);

    // scans the subsets from the smallest one to the largest one
    for (int i = 1; i <= mcds.size(); i ++) 
    {
      for (Iterator iter = mcdSubsets.iterator(); iter.hasNext();)
      {
	HashSet oneMCDSubset = (HashSet) iter.next();
	if (oneMCDSubset.size() != i) continue;
	
	// tries to build an MCD ONLY IF it is not a superset of
	// another existing subset in the found rewritings
	if (containsMCDSubset(rewritings, oneMCDSubset)) 
	  continue;

	Mapping ec = computeEC(oneMCDSubset); // computes the EC
	Rewriting rewriting = buildRewriting(query, oneMCDSubset, ec);
	if (rewriting != null) rewritings.add(rewriting);
      }
    }

    return rewritings;
  }



  /**
   * computes the EC function for the query and the psi function for a set
   * of MCDs.
   */
  static Mapping computeEC(HashSet mcdSubset) {
    Mapping ec = new Mapping();
    for (Iterator iter = mcdSubset.iterator(); iter.hasNext();) {
      MCD mcd = (MCD) iter.next();
      computeECPsi(ec, mcd);
    }

    // in the case of X1->Y1, Y1->Y2, we need to propogate the mappings to
    // the tail
    Mapping pec = new Mapping();
    Set keySet = (Set) ec.getMap().keySet();
    for (Iterator iter = keySet.iterator(); iter.hasNext();) {
      Argument headArg = (Argument) iter.next();
      Argument tailArg = ec.getTail(headArg);  // finds the tail for this srcArg
      pec.put(headArg, tailArg);
    }

    return pec;  // returns the propogated EC
  }

  /**
   * computes the EC function for the query and the psi function for the
   * MCD given an MCD
   */
  static void computeECPsi(Mapping ec, MCD mcd) {
    // sets the psi
    /*System.out.println("WWWWW: in computeECPsi()");
    System.out.println("WWWWW: ec = " + ec + " mcd = " + mcd);*/

    Mapping phi  = mcd.getPhi();
    Query   view = mcd.getView(); // applies the HH on the view
    Mapping psi  = new Mapping();

    // scans the args in phi
    Set entrySet = phi.getMap().entrySet();
    for (Iterator iter = entrySet.iterator(); iter.hasNext();) {
      Map.Entry mapEntry = (Map.Entry) iter.next();
      Argument queryArg  = (Argument) mapEntry.getKey();
      Argument viewArg   = (Argument) mapEntry.getValue();

      // this view arg should exist in the view
      viewArg = mcd.getHH().applyTryIdentity(viewArg);
      if (!view.containsArg(viewArg)) 
	UserLib.myerror("MiniCon.computeECPsi(), didn't find viewArg " +
			viewArg);
      
      Argument oldQueryArg = psi.apply(viewArg);
      if (oldQueryArg == null) {
	// this view arg hasn't been mapped.
	// puts a pair in psi by reversing this pair
	psi.put(viewArg, queryArg);
      } else { 
	// this view arg already mapped.  extends the ec
	// oldQueryArg    queryArg
	//           \    |
	//            \   |
	//            viewArg
	/*if (ec.get(queryArg) == null) 
	  ec.put(queryArg, oldQueryArg);*/

	if (queryArg.equals(oldQueryArg)) continue; // identical

	if (ec.apply(queryArg) != null) continue; // already mapped

	ec.put(queryArg, oldQueryArg);
      }
    }
    
    // sets the psi
    //System.out.println("WWWWW: computed psi = " + psi);
    mcd.setPsi(psi);
  }

  /**
   * Given a set of MCDs and an EC, build a rewriting
   */  
  static Rewriting buildRewriting(Query query, HashSet oneMCDSubset, 
				  Mapping ec) {
    HashSet coveredQuerySubgoals = new HashSet();

    for (Iterator iter = oneMCDSubset.iterator(); iter.hasNext();) {
      MCD mcd = (MCD) iter.next();
      HashSet querySubgoals = mcd.getMappedQuerySubgoals();

      // all the subgoals in the MCDs should be disjoint
      if (!UserLib.disjoint(coveredQuerySubgoals, querySubgoals)) 
	return null;

      // adds these new subgoals to the whole subgoals
      coveredQuerySubgoals.addAll(querySubgoals);
    }

    // all subgoal subgoals should be covered
    if (!coveredQuerySubgoals.containsAll(query.getBody()))
      return null;

    return createRewriting(query, oneMCDSubset, ec);
  }

  /**
   *  creates a rewriting.
   */
  static Rewriting createRewriting(Query query, HashSet oneMCDSubset, 
				   Mapping ec) {
    /*System.out.println("forming a rewriting using MCDs\n " + 
		       oneMCDSubset);*/
    int    id = 0;
    String name = query.getName();

    Subgoal head = ec.apply(query.getHead()); // head
    
    Vector body = new Vector();  // body
    for (Iterator iter = oneMCDSubset.iterator(); iter.hasNext();) {
      MCD     mcd     = (MCD) iter.next();
      Query   view    = mcd.getView();  // already applies HH !
      Mapping psi     = mcd.getPsi();
      Subgoal subgoal = ec.apply(psi.apply(view.getHead()));
      body.add(subgoal);
    }
    
    Query rew = new Query(name, head, body);
    return new Rewriting(rew, oneMCDSubset, ec);
  }

  /**
   * checks if a mcd subset is a superset of another existing 
   * mcd set in a given set of rewritings.
   */
  private static boolean containsMCDSubset(HashSet rewritings, 
					   HashSet oneMCDSubset) {
    for (Iterator iter = rewritings.iterator(); iter.hasNext();) {
      Rewriting rewriting = (Rewriting) iter.next();
      HashSet   mcds      = rewriting.getMCDs();

      if (oneMCDSubset.containsAll(mcds))
	return true;
    }
    
    return false;
  }

  // minimizes the rwritings
  static void minimizeRews() {
  }



    static class Prof extends JCProf {
    String me;
    long starttime, startcputime;
    long stoptime, stopcputime;
    long stoptime2, stopcputime2;

    Prof(String name) {
      me = name;
    }
    void start() {
      startcputime = getCpuTime();
      // starttime = System.currentTimeMillis();
    }
    void stop() {
      stopcputime = getCpuTime();
      //stoptime = System.currentTimeMillis();
    }

    void stop2() {
      stopcputime2 = getCpuTime();
      //stoptime2 = System.currentTimeMillis();
    }
	void print(){
	    System.out.println(startcputime + " " + stopcputime + " " + stopcputime2);
	}

  }



}
