/* 
 * GoodPlan.java
 * -------------
 * http://java.sun.com/j2se/1.3/docs/api/index.html
 * $Id: GoodPlan.java,v 1.49 2000/11/28 00:48:08 chenli Exp $
 */

// Sample gnu files: ~/Papers/1999/clindex/VLDB99-clindex/v2.3/*.com

import java.io.*;
import java.util.*;


class GoodPlan {

  //---------------------------------------------------------------
  final public static int randomQuery  = 0;
  final public static int chainQuery   = 1;
  final public static int starQuery    = 2;
  final public static int chainQueryFix   = 3;
  final public static int ccQuery    = 4;

  //final public static int simQueryType = randomQuery;
  //final public static int simQueryType = chainQuery;
   public static int simQueryType = ccQuery;
  //---------------------------------------------------------------
     public static boolean debug       = false;
    //public static boolean debug     = true;
  //---------------------------------------------------------------

  public static int viewNum            ;  // # of views
  public static int relationNum     = 8;  // # of relations
  public static int viewSubgoalNum  = 3;  // # of subgoals in a view

  public static int relAttrNum      ;  // # of rel attrs 
  public static int querySubgoalNum ;  // # of subgoals in a query

  public static int dropHeadArgNum  = 0;  // # of args dropped a head
  public static int queryArgDomSize = 5;  // size of arg domain in a query
  public static int simQueryNum     = 5; // # of queries
  public static int simQueryStep    = 5;  // # of queries in a step
  public static int validQueryNum      ;  // # of queries with rewritings

  public static int groupedViewNum     = 0; // real view number after grouping
  public static int oldGroupedViewNum  = 0; // old number

  public static int repetitionNum      = 1;  // # of repetition of experiment
  
  //---------------------------------------------------------------
  // statistics
  public static int  totalNumVTs        = 0;    // equivalence class
  public static int  totalGroupedNumVTs = 0; // without shrinking
  public static int  totalNumMR         = 0;
  public static int  totalNumGMR        = 0;
  public static int  totalSizeGMR       = 0;
  public static long totalTimeMR        = 0;
  public static long totalTimeFirstGMR  = 0;
  public static long totalTimeAllGMR    = 0;  
  //---------------------------------------------------------------
  public static Random random = new Random(18461);
  public static boolean testMiniCon = false;
  

    public static boolean isView;
    
  public static void main(String[] args) {
      String type = args[0];
      String viewFile = args[1];
      String queryFile = args[2];

      doOneTestYoli2(type, queryFile, viewFile);
    
  }



    public static void main2(String[] args) {
     int m = Integer.parseInt(args[0]);
     String tipo = args[1];
     int repeticion = Integer.parseInt(args[2]);
     
     if (simQueryType == starQuery) {
      querySubgoalNum = m;  // required by star queries
      viewSubgoalNum  = m;
      relAttrNum      = m;  // # of rel attrs 
      relationNum     = 10;
      dropHeadArgNum  = (m*(m - 1)+1)/2;


//       querySubgoalNum = m;  required by star queries
//       viewSubgoalNum  = m;
//       relAttrNum      = 2;  # of rel attrs 
//       relationNum     = 10;
//       dropHeadArgNum  = 0;
    }

    if (simQueryType == chainQueryFix) {
      querySubgoalNum = m;  // required by star queries
      viewSubgoalNum  = m;
      relAttrNum      = 2;  // # of rel attrs 
      relationNum     = 10;
      dropHeadArgNum  = 0;
    }

    if (simQueryType == randomQuery) {
      querySubgoalNum = m;  // required by star queries
      viewSubgoalNum  = m;
      relAttrNum      = 5;  // # of rel attrs 
      relationNum     = 10;
      dropHeadArgNum  = 0; // all distinguished
    }

    if (simQueryType == ccQuery) {
      querySubgoalNum = m;  // required by star queries
      viewSubgoalNum  = m;
      relAttrNum      = 5;  // # of rel attrs 
      relationNum     = 10;
      dropHeadArgNum  = 0; // all distinguished
    }
    
    //unitTest();
    //simulation();
    for (int i = 0; i < repeticion; i++) {
        repetitionNum = i;
        doOneTestYoli(tipo); 
    }
        
    }
    


  /**
   * Does the simulation and collects data
   */
  static void simulation() {

    System.out.println("nView\tnDiView\tProb\tnGMRs\ttGMRs\tVTs\tgVTs ");

    for (viewNum = 1; viewNum <= 1000; viewNum += simQueryStep)
      //for (viewNum = 20; viewNum <= 20; viewNum += 3)
      doOneTest();
  }


    static void doOneTestYoli(String tipo) {
    Vector relations = genRelations();


    Query query = genQuery(relations, simQueryType);
    Vector dummyquery = new Vector();
    dummyquery.add(query);
    String directory = "../satcomp/expTipo"+tipo+"/expM";
    //    System.out.println(directory+querySubgoalNum);
    
    printViews(dummyquery, directory+querySubgoalNum+"/consExpM"+querySubgoalNum+".txt");
    // simQueryType = chainQuery;
    Vector views = null;

    //   for (int n = 2; n <= 10; n = n + 2) {
     viewNum = querySubgoalNum/2;
     
      views = genViewsCC(relations, simQueryType);
      printViews(views, directory+querySubgoalNum+"/vistasExpM"+querySubgoalNum+"N"+viewNum+".txt");
      //printViews(views, directory+querySubgoalNum+"/vistasExpM"+querySubgoalNum+"N"+viewNum+"Rep"+repetitionNum+".txt");
      genPlan(query, views);
      //}
    
    showSetting(relations, views);

 
    // consider a larger number of views
    showStat();
  }


    static void doOneTestYoli2(String type, String queryFile, String viewFile) {

    Vector dummyquery = readViews(queryFile);
    Query query = (Query)dummyquery.elementAt(0);
    Vector views = readViews(viewFile);
    genPlan2(type, query, views);
    showStat();
    }



  static void doOneTest() {
    Vector relations = genRelations();
    Vector views     = genViews(relations, simQueryType);

    int oldNum = views.size();
    views = groupViews(views);
    groupedViewNum = views.size();

    // consider a larger number of views
    if (groupedViewNum < oldGroupedViewNum) return;

    //System.out.println(views);
    //System.out.println("oldViewNum = " + oldNum + " newViewNum = " + groupedViewNum);

    showSetting(relations, views);

    totalNumVTs        = 0;
    totalGroupedNumVTs = 0; // grouped
    totalNumMR         = 0;
    totalNumGMR        = 0;
    totalSizeGMR       = 0;
    totalTimeMR        = 0;
    totalTimeFirstGMR  = 0;
    totalTimeAllGMR    = 0;  
    validQueryNum      = 0; // # of queries with rewritings

    for (int queryNum = 1; queryNum <= simQueryNum; queryNum ++) {
      Query query = genQuery(relations, simQueryType);
      genPlan(query, views); // calls the algorithms
    }

    // consider a larger number of views
    if (groupedViewNum < oldGroupedViewNum)
      return;

    oldGroupedViewNum = groupedViewNum;
    showStat();
  }

  /**
   * groups different views into equivalence classes
   */
  static Vector groupViews(Vector views) {
    Vector groups = new Vector();
    for (int i = 0; i < views.size(); i ++) {
      Query view = (Query) views.elementAt(i);
      
      // search for equivalent views
      boolean found = false;
      for (int j = 0; j < groups.size(); j ++) {
	Query group = (Query) groups.elementAt(j);

	if ((group != view) && group.equivalent(view)) {
	  group.inreaseCount();
	  found = true;
	  debug  = false;
	  break;
	}

	debug  = false;
      }

      if (!found) { // new equivalence class
	view.setCount(1);
	groups.add(view);
      }
    }

    return groups;
  }

    static void genPlan(Query query, Vector views) {
    HashSet rewritings = null;

    UserLib.myprintln("\nThe query is:");
    UserLib.myprintln(query.toString() + "\n");
    
    // MiniCon algorithm
    /*if (testMiniCon) {*/
      long startTime = 0;
      long endTime   = 0;
      long timeMiniCon = 0;
      startTime   = System.currentTimeMillis();
      rewritings  = MiniCon.genPlan(query, views);
      endTime     = System.currentTimeMillis();
      timeMiniCon = endTime - startTime;
      UserLib.myprintln("rewritings generated by MiniCon():\n");
      for (Iterator iter = rewritings.iterator(); iter.hasNext();) {
          UserLib.myprintln("rewriting:\n" + (Rewriting) iter.next());
      }
      
      UserLib.myprintln("MiniCon time = " + timeMiniCon);
//       System.out.print(querySubgoalNum+"\t");
//       System.out.print(viewNum+"\t");
      System.out.print(MiniCon.numMCDs+"\t");
      System.out.print(MiniCon.numRews+"\t");
      System.out.print(MiniCon.timeFormMCD+"\t");
      System.out.println(MiniCon.timeCombineMCD);
      
      /*}*/
      
      /*
    // CoreCover algorithm
    rewritings = CoreCover.genPlan(query, views);
    //for (Iterator iter = rewritings.iterator(); iter.hasNext();) 
    //UserLib.myprintln("rewriting:\n" + (Rewriting) iter.next());
    if (!rewritings.isEmpty()) {
      collectStat();
      validQueryNum ++;
      }*/
  }

    static void genPlan2(String type, Query query, Vector views) {
    HashSet rewritings = null;

    UserLib.myprintln("\nThe query is:");
    UserLib.myprintln(query.toString() + "\n");
    
    // MiniCon algorithm
    /*if (testMiniCon) {*/
      long startTime = 0;
      long endTime   = 0;
      long timeMiniCon = 0;
      rewritings  = MiniCon.genPlan2(type, query, views);
      

      System.out.print(MiniCon.numMCDs+"\t");
      System.out.print(MiniCon.numRews+"\t");
      System.out.print(MiniCon.timeFormMCD+"\t");
      System.out.println(MiniCon.timeCombineMCD);
      
  }


  static void collectStat() {
    totalNumVTs        += CoreCover.numVTs;       // ungrouped
    totalGroupedNumVTs += CoreCover.groupedNumVTs; // grouped
    totalNumMR         += CoreCover.numMR;
    totalNumGMR        += CoreCover.numGMR;
    //System.out.println("CoreCover.numGMR = " + CoreCover.numGMR);
    totalSizeGMR       += CoreCover.sizeGMR;
    totalTimeMR        += CoreCover.timeMR;
    totalTimeFirstGMR  += CoreCover.timeFirstGMR;
    totalTimeAllGMR    += CoreCover.timeAllGMR;  

    UserLib.myprintln("# of ungrouped View Tuples   = " + CoreCover.numVTs);
    UserLib.myprintln("# of MRs          = " + CoreCover.numMR);
    UserLib.myprintln("# of GMRs          = " + CoreCover.numGMR);
    UserLib.myprintln("subgoal # of a GMR = " + CoreCover.sizeGMR);
    UserLib.myprintln("time of MR        = " + CoreCover.timeMR + " ms");
    UserLib.myprintln("time of first GMR  = " + CoreCover.timeFirstGMR + " ms");
    UserLib.myprintln("time of all GMRs   = " + CoreCover.timeAllGMR + " ms");
    UserLib.myprintln("------------------------------------------------");
  }

  static void showStat() {
    /*System.out.println("# of queries       = " + simQueryNum );
    System.out.println("# queries w/ rews  = " + validQueryNum );
    if (validQueryNum == 0) {
      System.out.println("------------------------------------------------");
      return;
    }

    System.out.println("# of View Tuples   = " + totalNumVTs / validQueryNum );
    System.out.println("# of MRs          = " + totalNumMR / validQueryNum);
    System.out.println("# of GMRs          = " + totalNumGMR / validQueryNum);
    System.out.println("subgoal # of a GMR = " + totalSizeGMR / validQueryNum);
    System.out.println("time of MR        = " + totalTimeMR / validQueryNum + " ms");
    System.out.println("time of first GMR  = " + totalTimeFirstGMR / validQueryNum + " ms");
    System.out.println("time of all GMRs   = " + totalTimeAllGMR / validQueryNum + " ms");
    System.out.println("------------------------------------------------");
    */

    // probability of having a rewriting in terms of number of views
    //System.out.println(" " + viewNum + " " + 
    //((float) validQueryNum) / ((float) simQueryNum));

    if (validQueryNum != 0) {
      // real number of views
      System.out.print(" " + viewNum);

      // number of distinct views
      System.out.print("\t " + groupedViewNum);

      // probability of having a rewriting
      System.out.print("\t " + (float) validQueryNum / ((float) simQueryNum));

      // number of GMRs
      System.out.print("\t " + totalNumGMR / validQueryNum);
      
      //number of MRs
      //System.out.print("\t " + totalNumMR / validQueryNum);
      
      // time of generating GMRs
      System.out.print("\t " + totalTimeAllGMR / validQueryNum);
      
      // time of generating MRs
      //System.out.print("\t " + totalTimeMR / validQueryNum);
      
      // # of ungrouped view tuples
      System.out.print("\t " + totalNumVTs / validQueryNum);

      // # of grouped view tuples
      System.out.print("\t " + totalGroupedNumVTs / validQueryNum);

      //System.out.print("\t " + validQueryNum);

      System.out.println();
    } else {
      /*System.out.println(" " + viewNum + 
			 "\t " + groupedViewNum +
			 "\t " + 0 +
			 "\t " + 0 +
			 "\t " + 0 +
			 "\t " + 0 +
			 "\t " + 0);*/
    }
  }
  
  static void unitTest() {
    test1();
    test2();
    test3();
  }
  
  // create a query: q(M,C)     :- car(M,d0), loc(d0,C,C)
  // create a view:  v(M,D,C,E) :- car(M,D),  loc(D,C,E)
  // ONLY one rewriting: q(M,C) :- v(M_1,d0,C,C),v(M,d0,C_2,E_2)
  static void test1() {
    // create a relation
    Vector relations = new Vector();
    String  name   = "r";  // relation name
    Vector  schema = new Vector();  // a vector of attributes
    schema = new Vector();
    schema.add(new Attribute("M"));
    schema.add(new Attribute("D"));
    schema.add(new Attribute("C"));
    relations.add(new Relation(name, schema));
    
    // create a view:  v(M,D,C,E) :- car(M,D), loc(D,C,E)
    Vector views = new Vector();
    String viewName = "v";

    Vector viewHeadArgs = new Vector();  // head: v(M,D,C,E)
    viewHeadArgs.add(new Argument("M"));
    viewHeadArgs.add(new Argument("D"));
    viewHeadArgs.add(new Argument("C"));
    viewHeadArgs.add(new Argument("E"));
    Subgoal viewHead = new Subgoal(viewName, viewHeadArgs);

    Vector viewBody     = new Vector();
    Vector subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("M")); // car(M,D)
    subgoalArgs.add(new Argument("D"));
    viewBody.add(new Subgoal("car", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("D")); // loc(D,C,E)
    subgoalArgs.add(new Argument("C"));
    subgoalArgs.add(new Argument("E"));
    viewBody.add(new Subgoal("loc", subgoalArgs));
    
    views.add(new Query(viewName, viewHead, viewBody));
    
    // create a query: q(M,C) :- car(M,d0), loc(d0,C,C)
    String queryName = "q";

    Vector queryHeadArgs  = new Vector();
    queryHeadArgs.add(new Argument("M"));  // head: q(M,C)
    queryHeadArgs.add(new Argument("C"));
    Subgoal queryHead = new Subgoal(queryName, queryHeadArgs);
    
    Vector queryBody      = new Vector();
    subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("M")); // car(M,d0)
    subgoalArgs.add(new Argument("d0"));
    queryBody.add(new Subgoal("car", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("d0")); // loc(d0,C,C)
    subgoalArgs.add(new Argument("C"));
    subgoalArgs.add(new Argument("C"));
    queryBody.add(new Subgoal("loc", subgoalArgs));
    Query query = new Query(queryName, queryHead, queryBody);

    showSetting(relations, views);
    
    genPlan(query, views);
    return;
  }

  /*
    Q(X1):-b(Y5,Y4),b(Y5,Y2),b(Y5,Y3),b(Y3,Y2),b(Y4,Y1),a(Y1,X1),a(Y2,X1),a(Y1,Y2)
   v1(A1,A2):- b(A1,Z1),b(Z1,A2),b(A1,A2)

   v2(B1,B2):- a(B1,B1),a(B1,B2)
   There are 8 rewritings.
   */
  static void test2() {
    // create two relations
    Vector relations = new Vector();
    String  name   = "b";           // b(X,Y)
    Vector  schema = new Vector();  
    schema.add(new Attribute("X"));
    schema.add(new Attribute("Y"));
    relations.add(new Relation(name, schema));

    name   = "a"; 
    schema = new Vector(); 
    schema.add(new Attribute("Z")); // a(Z,W)
    schema.add(new Attribute("W"));
    relations.add(new Relation(name, schema));

    // create a view:  v1(A1,A2):- b(A1,Z1),b(Z1,A2),b(A1,A2)
    Vector views = new Vector();

    String viewName = "v1";
    Vector viewHeadArgs = new Vector();  // head: v1(A1,A2)
    viewHeadArgs.add(new Argument("A1"));
    viewHeadArgs.add(new Argument("A2"));  
    Subgoal viewHead = new Subgoal(viewName, viewHeadArgs);
    
    Vector viewBody     = new Vector();
    Vector subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("A1"));
    subgoalArgs.add(new Argument("Z1")); // b(A1,Z1)
    viewBody.add(new Subgoal("b", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Z1")); // b(Z1,A2)
    subgoalArgs.add(new Argument("A2"));
    viewBody.add(new Subgoal("b", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("A1")); // b(A1,A2)
    subgoalArgs.add(new Argument("A2"));
    viewBody.add(new Subgoal("b", subgoalArgs));

    views.add(new Query(viewName, viewHead, viewBody));

    // create a view:  v2(B1,B2):- a(B1,B1),a(B1,B2)
    viewName = "v2";
    viewHeadArgs = new Vector();  // head: v2(B1,B2)
    viewHeadArgs.add(new Argument("B1"));
    viewHeadArgs.add(new Argument("B2"));
    viewHead = new Subgoal(viewName, viewHeadArgs);
    
    viewBody     = new Vector();
    subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("B1"));
    subgoalArgs.add(new Argument("B1")); // a(B1,B1)
    viewBody.add(new Subgoal("a", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("B1")); // a(B1,B2)
    subgoalArgs.add(new Argument("B2"));
    viewBody.add(new Subgoal("a", subgoalArgs));

    views.add(new Query(viewName, viewHead, viewBody));

    // create a query:    Q(X1):-b(Y5,Y4),b(Y5,Y2),b(Y5,Y3),b(Y3,Y2),b(Y4,Y1),a(Y1,X1),a(Y2,X1),a(Y1,Y2)
    //String queryName  = ((Relation) relations.elementAt(0)).getName();
    String queryName = "q";

    Vector queryHeadArgs  = new Vector();
    queryHeadArgs.add(new Argument("X1"));  // Q(X1)
    //queryHeadArgs.add(new Argument("Y3"));  // Q(Y3) !!!!
    Subgoal queryHead = new Subgoal(queryName, queryHeadArgs);

    Vector queryBody      = new Vector();
    subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("Y5")); // b(Y5,Y4)
    subgoalArgs.add(new Argument("Y4"));
    queryBody.add(new Subgoal("b", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y5"));
    subgoalArgs.add(new Argument("Y2"));  // b(Y5,Y2),
    queryBody.add(new Subgoal("b", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y5"));
    subgoalArgs.add(new Argument("Y3"));  // b(Y5,Y3)
    queryBody.add(new Subgoal("b", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y3"));
    subgoalArgs.add(new Argument("Y2"));  // b(Y3,Y2),
    queryBody.add(new Subgoal("b", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y4"));
    subgoalArgs.add(new Argument("Y1"));  // b(Y4,Y1)
    queryBody.add(new Subgoal("b", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y1"));
    subgoalArgs.add(new Argument("X1"));  // a(Y1,X1)
    queryBody.add(new Subgoal("a", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y2"));
    subgoalArgs.add(new Argument("X1"));  // a(Y2,X1)
    queryBody.add(new Subgoal("a", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("Y1"));
    subgoalArgs.add(new Argument("Y2"));  // a(Y1,Y2)
    queryBody.add(new Subgoal("a", subgoalArgs));

    Query query = new Query(queryName, queryHead, queryBody);

    showSetting(relations, views);
    genPlan(query, views);
    return;
  }

  //  Query: q(U)    :- a(U,U),c(U,X),b(X,X)
  //  Views: v1(A,B) :- a(A,A),       b(B,B)
  //         v2(C,D) :-        c(C,D),b(D,D)
  //  Rewriting:  q(U):- v1(U,X),v2(U,X) 
  static void test3() {
    // create a relation
    Vector relations = new Vector();
    String  name   = "a";  // relation name
    Vector  schema = new Vector();  // a vector of attributes
    schema = new Vector();
    schema.add(new Attribute("A")); // a(A,B)
    schema.add(new Attribute("B"));
    relations.add(new Relation(name, schema));

    name   = "b";  // relation name
    schema = new Vector();  // a vector of attributes
    schema = new Vector();
    schema.add(new Attribute("A")); // b(A,B)
    schema.add(new Attribute("B"));
    relations.add(new Relation(name, schema));

    name   = "c";  // relation name
    schema = new Vector();  // a vector of attributes
    schema = new Vector();
    schema.add(new Attribute("A")); // c(A,B)
    schema.add(new Attribute("B"));
    relations.add(new Relation(name, schema));
    
    // create a view:  v1(A,B) :- a(A,A), b(B,B)
    Vector views = new Vector();
    String viewName = "v1";

    Vector viewHeadArgs = new Vector();  // head: v1(A,B)
    viewHeadArgs.add(new Argument("A"));
    viewHeadArgs.add(new Argument("B"));
    Subgoal viewHead = new Subgoal(viewName, viewHeadArgs);

    Vector viewBody     = new Vector();
    Vector subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("A")); // a(A,A)
    subgoalArgs.add(new Argument("A"));
    viewBody.add(new Subgoal("a", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("B")); // b(B,B)
    subgoalArgs.add(new Argument("B"));
    viewBody.add(new Subgoal("b", subgoalArgs));
    
    views.add(new Query(viewName, viewHead, viewBody));
    
    // create 2nd view:  v2(C,D) :- c(C,D),b(D,D)
    viewName = "v2";
    viewHeadArgs = new Vector();  // head: v2(C,D)
    viewHeadArgs.add(new Argument("C"));
    viewHeadArgs.add(new Argument("D"));
    viewHead = new Subgoal(viewName, viewHeadArgs);

    viewBody     = new Vector();
    subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("C")); // c(C,D)
    subgoalArgs.add(new Argument("D"));
    viewBody.add(new Subgoal("c", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("D")); // b(D,D)
    subgoalArgs.add(new Argument("D"));
    viewBody.add(new Subgoal("b", subgoalArgs));
    
    views.add(new Query(viewName, viewHead, viewBody));

    //  create Query: q(U)    :- a(U,U),c(U,X),b(X,X)
    String queryName = "q";

    Vector queryHeadArgs  = new Vector();
    queryHeadArgs.add(new Argument("U"));  // head: q(U)
    Subgoal queryHead = new Subgoal(queryName, queryHeadArgs);
    
    Vector queryBody      = new Vector();
    subgoalArgs  = new Vector();
    subgoalArgs.add(new Argument("U")); // a(U,U)
    subgoalArgs.add(new Argument("U"));
    queryBody.add(new Subgoal("a", subgoalArgs));
    
    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("U")); // c(U,X),b(X,X)
    subgoalArgs.add(new Argument("X"));
    queryBody.add(new Subgoal("c", subgoalArgs));

    subgoalArgs      = new Vector();
    subgoalArgs.add(new Argument("X")); // b(X,X)
    subgoalArgs.add(new Argument("X"));
    queryBody.add(new Subgoal("b", subgoalArgs));
    Query query = new Query(queryName, queryHead, queryBody);

    showSetting(relations, views);
    genPlan(query, views);
    return;
  }


    static Vector readViews(String filename){
        try {
            parser P = new parser();
            FileInputStream fis = new FileInputStream(filename);
            System.setIn(fis);	
            Vector views = (Vector)(P.parse ()).value;
            return views;
	    }catch(FileNotFoundException fnfe){
		System.err.println("El archivo " + filename + " no existe");
        return null;
	    }
        catch(Exception e){
		e.printStackTrace();
        return null;
	    }  
    }


    static void printViews(Vector views, String filename){
        FileOutputStream out; // declare a file output object
        PrintStream p; // declare a print stream object
        try
            {
                // Create a new file output stream
                        // connected to "myfile.txt"
                out = new FileOutputStream(filename);
                
                // Connect print stream to the output stream
                p = new PrintStream( out );
                
                for (int i = 0; i < views.size(); i ++){
                    //                    System.out.println(views.elementAt(i).toString());
                    p.println(views.elementAt(i).toString());
                }
                p.close();
            }
        catch (Exception e)
            {
                System.err.println ("Error writing to file" + filename);
            }
    }

  static void showSetting(Vector relations, Vector views) {
    // print out the relations
    UserLib.myprintln("------------------------------------------------");
    UserLib.myprintln("The relations are:");
    for (int i = 0; i < relations.size(); i ++)
      UserLib.myprintln(relations.elementAt(i).toString());
    
    UserLib.myprintln("------------------------------------------------");
    // print out the views
    UserLib.myprintln("\nThe views are:");
    for (int i = 0; i < views.size(); i ++)
      UserLib.myprintln(views.elementAt(i).toString());

    UserLib.myprintln("------------------------------------------------");
    UserLib.myprintln("# of views             = " + viewNum);
    UserLib.myprintln("# of relations         = " + relationNum);
    UserLib.myprintln("# of rel attrs         = " + relAttrNum);
    UserLib.myprintln("# of query subgoals    = " + querySubgoalNum);
    UserLib.myprintln("# of view subgoals     = " + viewSubgoalNum);
    UserLib.myprintln("# of dropped head args = " + dropHeadArgNum);
    UserLib.myprintln("size of arg domain     = " + queryArgDomSize + "\n");
    UserLib.myprintln("------------------------------------------------");
  }
  // generate a set of relations
  static Vector genRelations() {
    Vector relations = new Vector();
    for (int i = 1; i <= relationNum; i ++) {
      relations.add(new Relation(i));
    }
    return relations;
  }

  // generate a set of views
  static Vector genViews(Vector relations, int viewType) {
    Vector views = new Vector();
    for (int i = 1; i <= viewNum; i ++) {
      views.add(new Query("v" + i, relations, viewType, viewSubgoalNum));
    }
    return views;
  }
  
  // generate a set of views cc
  static Vector genViewsCC(Vector relations, int viewType) {
    Vector views = new Vector();
    for (int i = 1; i <= viewNum; i ++) {
      views.add(new Query("v" + i, relations, viewType, 2*i));
    }
    return views;
  }

  // generate a conjunctive query
  static Query genQuery(Vector relations, int queryType) {
    return  new Query("q", relations, queryType, querySubgoalNum);
  }
}
