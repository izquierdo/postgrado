/* 
 * Query.java
 * ----------
 * Records a query on relations
 * $Id: Query.java,v 1.38 2000/11/28 00:48:08 chenli Exp $
 */

import java.util.*;
import java.io.*;
import java.lang.*;

class Query {
  public static int renameCounter = 0;

  String  name = null; // query name
  Subgoal head = null; // the head is also a subgoal
  Vector  body = null; // a set of subgoals
  int     queryType = GoodPlan.randomQuery;

  int     count = 0; // number of equivalent views in the same class

  /**
   * Creates a query given an id.
   */
  Query(String name, Vector relations, int queryType, int bodySubgoalNum) {
    this.name = name;
    this.queryType = queryType;
    genBody(relations, queryType, bodySubgoalNum);
    genHead(relations, queryType, bodySubgoalNum);
  }

  /**
   * Constructor
   */
  Query(String name, Subgoal head, Vector body) {
    this.name = name;
    this.head = head;
    this.body = body;
  }

  /**
   * generates the body of the query
   */
  void genBody(Vector relations, int queryType, int bodySubgoalNum) {

    // random query
    if (queryType == GoodPlan.randomQuery) {
      genBodyRandom(relations, bodySubgoalNum);
      return;
    }

    // chain query
    if (queryType == GoodPlan.chainQuery) {
      genBodyChain(relations, bodySubgoalNum);
      return;
    }

    // chain query
    if (queryType == GoodPlan.chainQueryFix) {
      genBodyChainFix(relations, bodySubgoalNum);
      return;
    }


    // star query
    if (queryType == GoodPlan.starQuery) {
      genBodyStar(relations, bodySubgoalNum);
      return;
    }

    if (queryType == GoodPlan.ccQuery) {
      genBodyCC(relations, bodySubgoalNum);
      return;
    }
  }


  void genBodyCC(Vector relations, int bodySubgoalNum) {

     if ( (bodySubgoalNum % 2) != 0)
        UserLib.myerror("Query.genBodyCC(): bodySubGoalNum not even");


    // generates the subgoals
    body = new Vector(); 
    int argCounter = 1;
    int relSize = relations.size();
    int relSubgoalNum = bodySubgoalNum/2;

    //    for (int i = 0; i < bodySubgoalNum; i ++) {
    for (int i = 1; i <= relSubgoalNum; i ++) {

      Vector args = new Vector();
      args.add(new Argument("Z0"));
      args.add(new Argument("Z"+i));
      body.add(new Subgoal("a"+i, args));

      args = new Vector();
      args.add(new Argument("Z"+i));
      args.add(new Argument("Z"+(relSubgoalNum+1)));
      body.add(new Subgoal("b"+i, args));

    }
  }

  /**
   * generates the body of the query
   */
  void genBodyRandom(Vector relations, int bodySubgoalNum) {
    // populates the domain of arguments
      GoodPlan.queryArgDomSize = bodySubgoalNum*3;
    Vector argDom = new Vector();
    for (int i = 1; i <= GoodPlan.queryArgDomSize; i ++) {
      String argName = UserLib.getChar(i);
      argDom.add(new Argument(argName));
    }

    body = new Vector(); 
    //    System.out.println();
    // generates the subgoals
    //    int subgoalNum = bodySubgoalNum;
    int subgoalNum = GoodPlan.random.nextInt(bodySubgoalNum)+1;
    Vector rel2 = (Vector)relations.clone();

    for (int i = 0; i < bodySubgoalNum; i ++) {
      //int relIndex = Math.abs(GoodPlan.random.nextInt(relations.size()); 
      int relIndex = GoodPlan.random.nextInt(rel2.size()); 
      body.add(new Subgoal(rel2, relIndex, argDom));
      rel2.removeElementAt(relIndex);
    }
  }

  Vector getBodyArgs() {
    Vector bodyArgs = new Vector(); // arguments used in the body
    for (int k = 0; k < body.size(); k ++) {
      Subgoal subgoal = (Subgoal) body.elementAt(k);
      
      Vector args = subgoal.getArgs();
      for (int i = 0; i < args.size(); i ++) {
	// make sure we add the same arg only once
	Argument subgoalArg = (Argument) args.elementAt(i);
	if (bodyArgs.indexOf(subgoalArg) == -1 && !subgoalArg.isConst())
	  bodyArgs.add(subgoalArg);
      }
    }
    return bodyArgs;
  }

  /**
   * generates the body of the chain query (con longitud variable) 
   */
  void genBodyChain(Vector relations, int bodySubgoalNum) {

    // generates the subgoals
    body = new Vector(); 
    int argCounter = 1;
    int relSize = relations.size();
    int relSubgoalNum = GoodPlan.random.nextInt(bodySubgoalNum-1) + 2;

    //    for (int i = 0; i < bodySubgoalNum; i ++) {
    for (int i = 0; i < relSubgoalNum; i ++) {
      int relIndex = GoodPlan.random.nextInt(relSize); 
      //int relIndex = random.nextInt(relSize);

      Relation relation = ((Relation) relations.elementAt(relIndex));
      String subgoalName = relation.getName();
      Vector args = new Vector();
      for (int j = 0; j < relation.getAttrNum(); j ++) {
	String argName = UserLib.getChar(argCounter + j);
	args.add(new Argument(argName));
      }

      // TODO
      body.add(new Subgoal(subgoalName, args));
      argCounter += args.size() - 1; // chained
    }
  }


/**
   * generates the body of the chain query (con longitud fija) 
   */
  void genBodyChainFix(Vector relations, int bodySubgoalNum) {

    // generates the subgoals
    body = new Vector(); 
    int argCounter = 1;
    int relSize = relations.size();
    int relSubgoalNum = GoodPlan.random.nextInt(bodySubgoalNum-1) + 2;

    Map<Integer,Boolean> isConstant = new HashMap<Integer,Boolean>();

    for (int i = 0; i < bodySubgoalNum; i ++) {
        //for (int i = 0; i < relSubgoalNum; i ++) {
      int relIndex = GoodPlan.random.nextInt(relSize); 
      //int relIndex = random.nextInt(relSize);

      Relation relation = ((Relation) relations.elementAt(relIndex));

      String subgoalName = relation.getName();
      Vector args = new Vector();
      for (int j = 0; j < relation.getAttrNum(); j ++) {
          int number = argCounter + j;

          if (!isConstant.containsKey(number)) {
              /* TODO request constant percentage from user */
              int constantPercentage = 0;
              boolean setConstant = (GoodPlan.random.nextInt(100) < constantPercentage);

              isConstant.put(number, setConstant);
          }

          String argName = UserLib.getChar(argCounter + j, isConstant.get(number));
          args.add(new Argument(argName, isConstant.get(number)));
      }

      // TODO
      body.add(new Subgoal(subgoalName, args));
      argCounter += args.size() - 1; // chained
    }
  }

  /**
   * generates the body of the star query !!!!!
   */
  void genBodyStar(Vector relations, int bodySubgoalNum) {

    if (GoodPlan.querySubgoalNum > GoodPlan.relAttrNum + 1)
      UserLib.myerror("Query.genBodyStar(): querySubgoalNum <= relAttrNum!");

    body = new Vector(); 

    // creates the center subgoal
    int argCounter = 1;
    int relSize = relations.size();
    int relIndex = GoodPlan.random.nextInt(relSize); 
    Relation relation = ((Relation) relations.elementAt(relIndex));
    String subgoalName = relation.getName();

    // the arguments
    Vector centerArgs = new Vector();
    for (int j = 0; j < relation.getAttrNum(); j ++) {
      String argName = UserLib.getChar(argCounter);
      centerArgs.add(new Argument(argName));
      argCounter ++;	
    }
    Subgoal centerSubgoal = new Subgoal(subgoalName, centerArgs);
    body.add(centerSubgoal);

    int relSubgoalNum = GoodPlan.random.nextInt(bodySubgoalNum) + 1;
    
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // all the rest subgoals that join with the center subgoal
    // generates the subgoals
    for (int i = 0; i < bodySubgoalNum - 1; i ++) {
        //for (int i = 0; i < relSubgoalNum - 1; i ++) {
      // the relation
      relIndex = GoodPlan.random.nextInt(relSize);
      relation = ((Relation) relations.elementAt(relIndex));
      subgoalName = relation.getName();

      // the arguments
      Vector args = new Vector();
      // the argument joining with the center subgoal
      args.add(centerArgs.elementAt(i)); 

      // the rest
      for (int j = 1; j < relation.getAttrNum(); j ++) {
	String argName = UserLib.getChar(argCounter);
	args.add(new Argument(argName));
	argCounter ++;	
      }

      body.add(new Subgoal(subgoalName, args));
    }
  }

  // generates the head of the query
  void genHead(Vector relations, int queryType, int bodySubgoalNum) {

    // random query head
    if (queryType == GoodPlan.randomQuery) {
      genHeadRandom(relations);
      return;
    }

    // chain query head
    if (queryType == GoodPlan.chainQuery || queryType == GoodPlan.chainQueryFix) {
      genHeadChain(relations);
      return;
    }

    // star query head
    if (queryType == GoodPlan.starQuery) {
      genHeadStar(relations);
      return;
    }

    if (queryType == GoodPlan.ccQuery) {
      genHeadCC(bodySubgoalNum);
      return;
    }

  }

  void genHeadCC(int bodySubgoalNum) {
    Vector headArgs = new Vector();
    // randomly drop arguments
    int relSubgoalNum = bodySubgoalNum/2;
    
    headArgs.add(new Argument("Z0"));
    headArgs.add(new Argument("Z"+(relSubgoalNum+1)));

    // sets the head, with the query's name as the subgoal's name
    head = new Subgoal("q", headArgs);
  }

  // generates the head of the random query
  void genHeadRandom(Vector relations) {
    Vector headArgs = getBodyArgs();
    // randomly drop arguments

    if (GoodPlan.dropHeadArgNum == -3)
	GoodPlan.dropHeadArgNum = headArgs.size()/2;

    for (int i = 0; i < GoodPlan.dropHeadArgNum; i ++) {
      int removeIndex = GoodPlan.random.nextInt(headArgs.size());
      headArgs.removeElementAt(removeIndex);
    }
    
    // sets the head, with the query's name as the subgoal's name
    head = new Subgoal(name, headArgs);
  }

  // generates the head of the chain query
  void genHeadChain(Vector relations) {
    Vector headArgs = getBodyArgs();
    // uncomment this to generate only head and tail arguments
    if (GoodPlan.dropHeadArgNum == -1) {
        GoodPlan.dropHeadArgNum = headArgs.size() / 2;
    }
    
    

    // randomly drop arguments, while keeping the head and tail arguments
    if (body.size() > 1)
      for (int i = 0; i < GoodPlan.dropHeadArgNum; i ++) {
	int removeIndex = GoodPlan.random.nextInt(headArgs.size());
	// we want to keep the head and tail args
	if (removeIndex == 0 || removeIndex == headArgs.size() - 1)
	  removeIndex = 1;
	if (headArgs.size() == 2)
	  UserLib.myerror("Query.genHeadChain(): too few arguments to drop.");
	headArgs.removeElementAt(removeIndex);
      }
    
    // sets the head, with the query's name as the subgoal's name
    head = new Subgoal(name, headArgs);
  }

  // generates the head of the star query
  void genHeadStar(Vector relations) {
    Vector headArgs = getBodyArgs();
    // randomly drop arguments except the "center" variable
    if (GoodPlan.dropHeadArgNum == -1)
	headArgs = dropJoined(headArgs);

    else if (GoodPlan.dropHeadArgNum == -2)
	headArgs = ((Subgoal)body.elementAt(0)).getArgs();

    else if (GoodPlan.dropHeadArgNum == -3)
	GoodPlan.dropHeadArgNum = headArgs.size()/2;

        for (int i = 0; i < GoodPlan.dropHeadArgNum; i ++) {
	    int removeIndex = GoodPlan.random.nextInt(headArgs.size());
	    // we want to keep the root
	    if (headArgs.size() == 1)
		UserLib.myerror("Query.genHeadStar(): too few arguments to drop.");
	    headArgs.removeElementAt(removeIndex);
	}
    
    // sets the head, with the query's name as the subgoal's name
    head = new Subgoal(name, headArgs);
  }


    Vector dropJoined(Vector headArgs){
      Subgoal subgoal = (Subgoal) body.elementAt(0);
      Vector args = subgoal.getArgs();
      Vector result = new Vector();
      for (int i = 0; i < headArgs.size(); i ++) {
	// make sure we drop the same arg only once
	Argument subgoalArg = (Argument) headArgs.elementAt(i);
	int ind = headArgs.indexOf(subgoalArg);
	if (ind == -1)
	    result.add(subgoalArg);
      }
      return result;
    }


  /**
   * Gets all the distinguished variables in the head
   */
  public Vector getDistVars() {
    return head.getArgs();
  }

  public int getSubgoalNum() {
    return body.size();
  }

  /**
   * Gets all the distinguished variables in the head
   */
  public boolean isDistVar(Argument arg) {
    // a constant is a "dintinguished" arg
    if (arg.isConst()) return true;
    return  head.getArgs().contains(arg);
  }

  /**
   * Checks if the give arg is an arg in this query
   */
  public boolean containsArg(Argument arg) {
    return getBodyArgs().contains(arg);
  }
  
  /**
   * Gets all the head homomorphisms of the variables in the head
   */
  public Vector getHeadHomos() {
    Vector headHomos = new Vector();
    Vector distVars  = getDistVars();

    if (distVars.size() == 0) return headHomos;
    
    Map partialMap = new HashMap();
    //headHomos = UserLib.genAllMappings(distVars, distVars);
    headHomos = UserLib.genPartitionEquMappings(distVars);
    return headHomos;
  }

  public String getName() {
    return name;
  }

  public Subgoal getHead() {
    return head;
  }

  public Vector getBody() {
    return body;
  }

  /**
   * returns the i^th subgoal in the body
   */
  public Subgoal getSubgoal(int index) {
    return (Subgoal) body.elementAt(index);
  }

  /**
   * Indicates whether some other MCD is "equal to" this one.
   * We compare their queries, views, and mapped query subgoals.
   */
  public boolean equals(Object tmpQuery) {
    Query query = (Query) tmpQuery;
    return this.getName().equals(query.getName());
  }

  public int hashCode() {
    return (name.hashCode());
  }
  
  public String toString() {
    // head first
    StringBuffer result = new StringBuffer(name);
    result.append("(");
    for (int i = 0; i < head.size(); i ++) {
      result.append(head.getArgs().elementAt(i).toString());
      if ( i < head.size() - 1)
	result.append(",");
    }
    result.append(") :- ");
    
    // body
    for (int i = 0; i < body.size(); i++) {
      Subgoal subgoal = (Subgoal) body.elementAt(i);
      result.append(subgoal.toString());
      if ( i != body.size() - 1 ) result.append(",");
    }
    
    return result.toString();
  }

  /**
   * Renames the variables in the query.  Returns a new query with the
   * variables renamed.
   */
  public Query rename() {
    renameCounter ++;
    
    // remembers the map between the old args to the new args
    HashMap renameMap = new HashMap();

    // renames the head
    Subgoal newHead = head.rename(renameCounter, renameMap);

    // rename the body
    Vector newBody = new Vector();
    for (int i = 0; i < body.size(); i ++) {
      Subgoal oldSubgoal = (Subgoal) body.elementAt(i);
      Subgoal newSubgoal = oldSubgoal.rename(renameCounter, renameMap);
      newBody.add(newSubgoal);
    }

    return new Query(this.getName(), newHead, newBody);
  }

  /**
   * get the useful variables after the i^th subgoals
   */
  public Vector getUsefulArgs(int index) {
    Vector usefulArgs = (Vector) head.getArgs().clone();
    for (int i = index + 1; i < body.size(); i ++) {
      Subgoal subgoal = (Subgoal) body.elementAt(i);

      Vector subgoalArgs = subgoal.getArgs();
      for (int j = 0; j < subgoalArgs.size(); j ++) {
	Argument arg = (Argument) subgoalArgs.elementAt(j);
	if (!arg.isConst() && !usefulArgs.contains(arg)) // no constant
	  usefulArgs.add(arg);
      }
    }

    return usefulArgs;
  }

  int getNextRandom(Random random, int times) {
    int value = 0;
    for (int i = 0; i < times; i ++)
      value = random.nextInt();
    return value;
  }

  public void setCount(int count) {
    this.count = count;
  }

  public int getCount() {
    return count;
  }

  public void inreaseCount() {
    count ++;
  }

  /*
   * containment test
   */
  public boolean contained(Query query) {

    /*    if (this.getName().equals("v2") &&query.getName().equals("v8") ||
	this.getName().equals("v8") && query.getName().equals("v2"))
      GoodPlan.debug = true;*/

    Mapping cm = new Mapping(); // containment mapping

    // maps the head
    Subgoal srcHead = this.getHead();
    Subgoal dstHead = query.getHead();
    if (srcHead.size() != dstHead.size()) // same size
      return false;

    if (!srcHead.unifiable(dstHead, cm, false)) // ignore names
      return false;

    // tries all sequences
    Vector srcSubgoalSeq = this.getBody(); 
    Vector dstSubgoalPermus = UserLib.getAllSeqs(query.getBody(), 
						 srcSubgoalSeq.size());

    for (Iterator iter = dstSubgoalPermus.iterator(); iter.hasNext();) 
    {
      Vector dstSubgoalSeq = (Vector) iter.next();
      if (testMapping(srcSubgoalSeq, dstSubgoalSeq, (Mapping) cm.clone()))
	return true; // found it!
    }

    return false;
  }

  /** 
   * Given two sequences of subgoals and a partial mapping, tired to map
   * the two sequences.
   */
  boolean testMapping(Vector srcSubgoalSeq, Vector dstSubgoalSeq, 
		      Mapping cm) {

    if (srcSubgoalSeq.size() != dstSubgoalSeq.size())
      UserLib.myerror("Query.testMapping(): wrong sequences");
    
    for (int i = 0; i < srcSubgoalSeq.size(); i ++) {
      Subgoal srcSubgoal = (Subgoal) srcSubgoalSeq.elementAt(i);
      Subgoal dstSubgoal = (Subgoal) dstSubgoalSeq.elementAt(i);
      if (srcSubgoal.size() != dstSubgoal.size()) // same size
	return false;
      
      if (!srcSubgoal.unifiable(dstSubgoal, cm, true)) // care names
	return false;
    }
    
    return true;
  }

  /*
   * equivalence test
   */

  public boolean equivalent(Query query) {
    return this.contained(query) && query.contained(this);
  }


  public Object clone() {
    return new Query(name, head, (Vector) body.clone());
  }

  /**
   *
   */
  public Query minimize() {
    Query minQuery = (Query) this.clone();
    Vector subgoals = minQuery.getBody();
    int i = 0;
    while (i < subgoals.size()) {
      // try to remove one subgoal
      Subgoal subgoal = (Subgoal) subgoals.remove(i);
      if (!this.equivalent(minQuery)) {  // put it a back
	subgoals.insertElementAt(subgoal, i);
	i ++;
      }
    }

    return minQuery;
  }


  /**
   * Unit test.
   */
  public static void main(String[] args) {
    //test1();
     int m= Integer.parseInt(args[0]);
     int n= Integer.parseInt(args[1]);
     test2(1, m, n, args[2], args[4], args[3]);
     //test2(20, m, n, args[2], args[4], args[3]);
  }
  static void test1() {
    // generates a conjunctive query
    Vector relations = GoodPlan.genRelations();
    Query query = new Query("testQuery", relations, 
			    GoodPlan.chainQuery, GoodPlan.querySubgoalNum);
    System.out.println("Query is:\n" + query.toString());

    Vector headHomos = query.getHeadHomos();
    System.out.println("There are " + headHomos.size() + " HHs.");
    for (int i = 0; i < headHomos.size(); i ++) {
      Mapping headHomo = (Mapping) headHomos.elementAt(i);
      System.out.println(headHomo.toString());

      // tests "apply()"
      Query newQuery = headHomo.apply(query);
      if (newQuery == null) {
	System.out.println("Query test failed: newQuery shouldn't be null.");
	System.exit(1);
      }
      System.out.println("Under this HH, the new query is:");
      System.out.println(newQuery.toString() + "\n");

      // rename
      newQuery.rename();
      System.out.println("After renaming, the new query is:");
      System.out.println(newQuery.toString() + "\n");
    }
  }




    static void test2(int repeticiones, int m, int n, String tipo, String queryFile, String viewFile) {


        
        if (tipo.equals("1") || tipo.equals("7")) {
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo1(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }
            
    
        if (tipo.equals("2")) {
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo2(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }
            

        if (tipo.equals("3") || tipo.equals("8")) {
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo3(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }
            
    
        if (tipo.equals("4")) {
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo4(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }

        if (tipo.equals("5") || tipo.equals("9")) {
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo5(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }

        if (tipo.equals("6")) {
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo6(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }


        if (tipo.equals("11")) {
            GoodPlan.relationNum     = 2;
                for (int i = 0; i < repeticiones; i++) {
                    genExpTipo1(m, n, queryFile+"_"+i+".txt", viewFile+"_"+i+".txt");
                }
        }

  }



    static void genExp(int m, int n, String queryFile, String viewFile, int queryType, int viewType){
    Vector relations = GoodPlan.genRelations();
    //System.out.println("holafd " + GoodPlan.querySubgoalNum);
    Vector views = new Vector();
    Query query;
    int dropHeadArgTemp = GoodPlan.dropHeadArgNum;
    GoodPlan.isView = true;
    for(int i = 0; i < n; i++){
    //query = new Query("v"+i, relations, viewType, GoodPlan.querySubgoalNum);
    query = new Query("v"+i, relations, viewType, 3);
    views.add(query);
    GoodPlan.dropHeadArgNum = dropHeadArgTemp;
    }

    printViews(views, viewFile);
    GoodPlan.isView = false;
    query = new Query("q", relations, queryType, GoodPlan.querySubgoalNum);
    printQuery(query, queryFile);
    }


    static void genExpTipo1(int m, int n, String queryFile, String viewFile){
    // generates a conjunctive query
    GoodPlan.querySubgoalNum = m;
    GoodPlan.relAttrNum = 4;
    GoodPlan.relationNum = 10;
    GoodPlan.dropHeadArgNum = 0;

    genExp(m,n,queryFile,viewFile, GoodPlan.chainQueryFix, GoodPlan.chainQueryFix);
    }


    static void genExpTipo2(int m, int n, String queryFile, String viewFile){
    // generates a conjunctive query
    GoodPlan.querySubgoalNum = m;
    GoodPlan.relAttrNum = 4;
    GoodPlan.relationNum = 10;
    GoodPlan.dropHeadArgNum = -1;

    genExp(m,n,queryFile,viewFile, GoodPlan.chainQueryFix, GoodPlan.chainQueryFix);
    }


    static void genExpTipo3(int m, int n, String queryFile, String viewFile){
    // generates a conjunctive query
    GoodPlan.querySubgoalNum = m;
    GoodPlan.relAttrNum = m;
    GoodPlan.relationNum = 10;
    GoodPlan.dropHeadArgNum = 0;

    genExp(m,n,queryFile,viewFile, GoodPlan.starQuery, GoodPlan.starQuery);
    }



    static void genExpTipo4(int m, int n, String queryFile, String viewFile){
    // generates a conjunctive query
    GoodPlan.querySubgoalNum = m;
    GoodPlan.relAttrNum = m;
    GoodPlan.relationNum = 10;
    GoodPlan.dropHeadArgNum = -3; // la mitad distinguibles

    genExp(m,n,queryFile,viewFile, GoodPlan.starQuery, GoodPlan.starQuery);
    }

    static void genExpTipo5(int m, int n, String queryFile, String viewFile){
    // generates a conjunctive query
    GoodPlan.querySubgoalNum = m;
    GoodPlan.relAttrNum = 5;
    GoodPlan.relationNum = 10;
    GoodPlan.dropHeadArgNum = 0; // todos distinguibles

    genExp(m,n,queryFile,viewFile, GoodPlan.randomQuery, GoodPlan.randomQuery);
    }

    static void genExpTipo6(int m, int n, String queryFile, String viewFile){
    // generates a conjunctive query
    GoodPlan.querySubgoalNum = m;
    GoodPlan.relAttrNum = 5;
    GoodPlan.relationNum = 10;
    GoodPlan.dropHeadArgNum = -3; // la mitad distinguibles

    genExp(m,n,queryFile,viewFile, GoodPlan.randomQuery, GoodPlan.randomQuery);
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


    static void printQuery(Query query, String filename){
        FileOutputStream out; // declare a file output object
        PrintStream p; // declare a print stream object
        try
            {
                out = new FileOutputStream(filename);
                p = new PrintStream( out );
                p.println(query.toString());
                p.close();
            }
        catch (Exception e)
            {
                System.err.println ("Error writing to file" + filename);
            }
    }

}
