/* 
 * UserLib.java
 * -------------
 * $Id: UserLib.java,v 1.19 2000/11/28 00:48:09 chenli Exp $
 */

import java.util.*;

class UserLib {

  public static void myerror(String str) {
    System.out.println(str);
    System.exit(1);
  }

  public static void myprintln(String str) {
    if ( GoodPlan.debug == true )
      System.out.println(str);
  }

  public static String getChar(int index, boolean isConstant) {
      if (isConstant)
          return "" + index;

      return "X" + index;

      /*
      if (GoodPlan.isView)
	  return "X" + index;
      else
	  return "X" + index;
      */
  }

  /**
   * Gets the i^th character.
   */
  public static String getChar(int index) {
      return getChar(index, false);
  }

  /**
   * generates all possible mappings from one set of args to another set
   * of args. Here is the design.  Suppose we have A, B, C, D as the
   * distinguished variables.  For A, it can be mapped to A, B, C, or D.
   * For each of them, we expand it (recursively) by consider the four
   * possible mapping targets of B, and so on.  
   */
  public static Vector genAllMappings(Vector srcArgs, Vector dstArgs) {
    Map partialMap  = new HashMap();
    Vector mappings = new Vector();
    buildHeadHomos(mappings, srcArgs, dstArgs, partialMap, 0); 
    return mappings;
  }
  
  /**
   * recursively build mappings
   */
  private static void buildHeadHomos(Vector headHomos, 
				     Vector srcArgs, Vector dstArgs,
				     Map partialMap, int srcArgIndex) {
    
    if (srcArgIndex == srcArgs.size()) { // we reach the last arg
      // we need to clone one so that we can operate on the partial one later
      Map completeMap = (Map) ((HashMap) partialMap).clone(); 
      headHomos.add(new Mapping(completeMap));
      return;
    }

    for (int dstArgIndex = 0; dstArgIndex < dstArgs.size(); dstArgIndex ++) {
      // adds "src arg -> dst arg" to the partial mapping
      partialMap.put(srcArgs.elementAt(srcArgIndex), 
		     dstArgs.elementAt(dstArgIndex));
      // recursively builds a mapping
      buildHeadHomos(headHomos, srcArgs, dstArgs, partialMap, srcArgIndex + 1); 
      // removes "src arg -> dst arg" from the partial mapping
      partialMap.remove(srcArgs.elementAt(srcArgIndex));
    }
  }

  /**
   * given args, finds all their partitions.  For each partition, equates
   * the args in each group to any arg in the group.
   */
  public static Vector genPartitionEquMappings(Vector args) {
    HashSet partitions = (HashSet) genPartitions(args);
    Vector mappings = new Vector();

    for (Iterator iter = partitions.iterator(); iter.hasNext();) {
      HashSet onePartition = (HashSet) iter.next(); // one partition

      // scan the groups in this partition
      Map map = new HashMap();
      for (Iterator iterGroup = onePartition.iterator(); 
	   iterGroup.hasNext();) {
	HashSet group = (HashSet) iterGroup.next(); // one group

	if (group.size() <= 0 )
	  myerror("genPartitionEquMappings() " +
		  "Wrong partitions.  A group should have " +
		  "an argument.");

	boolean firstOne = true;
	Argument dstArg = null;
	// scan the arg in a group
	for (Iterator iterArg = group.iterator(); iterArg.hasNext();) {
	  Argument srcArg = (Argument) iterArg.next(); // one argument
	  if (firstOne) {
	    // picks the first argument as the dst argument
	    dstArg = srcArg;
	    firstOne = false;
	  }

	  // map them
	  map.put(srcArg, dstArg);
	}
      }
      
      mappings.add(new Mapping(map));
    }
    
    return mappings;
  }

  /**
   * Generates all partitions of a set, represented as a vector
   */
  public static Set genPartitions(Vector v) {
    Set partitions = new HashSet(); // all partitions
    buildPartitions(partitions, v, 0);
    return partitions;
  }

  /**
   * recursively build partitions
   */
  private static void buildPartitions(Set partitions, Vector v, int index) {

    if (index == v.size() - 1) {
      // creates one partition with one group with 1 element
      Set newPartition = new HashSet();
      HashSet group = new HashSet();
      group.add(v.elementAt(index));  
      newPartition.add(group);
      partitions.add(newPartition);
      return;
    }
    
    // recursively builds subsets
    buildPartitions(partitions, v, index + 1);

    // expanding the partitions

    // iterator doesn't allow us to add new partitions, so let's use another
    // set to keep the new partitions
    HashSet newPartitions = new HashSet();
    for (Iterator iter = partitions.iterator(); iter.hasNext();) {
      HashSet onePartition = (HashSet) iter.next(); // one partition
      
      // 1) for each partition, for each group, add the current element to
      //    form a new partition;
      for (Iterator iterGroup = onePartition.iterator(); 
	   iterGroup.hasNext();) {
	HashSet currentGroup = (HashSet) iterGroup.next();
	
	// adds the currents element to this group	
	HashSet newGroup = (HashSet) currentGroup.clone();
	newGroup.add(v.elementAt(index));

	// creates a new partition and adds this group
	HashSet newPartition = new HashSet();
	newPartition.add(newGroup);

	// copies all other groups in this partition
	for (Iterator iterOtherGroup = onePartition.iterator(); 
	     iterOtherGroup.hasNext();) {
	  HashSet otherGroup = (HashSet) iterOtherGroup.next();
	  if (otherGroup != currentGroup)
	    newPartition.add(otherGroup.clone());
	}

	// adds this partition to the new set
	newPartitions.add(newPartition);
      }      
      
      // 2) for each partition, expands it by adding a group with the 
      //    current element.
      HashSet newPartition = new HashSet();
      HashSet group = new HashSet();
      group.add(v.elementAt(index));  
      newPartition.add(group);
      for (Iterator iterGroup = onePartition.iterator(); 
	   iterGroup.hasNext();) {
	group = (HashSet) iterGroup.next();
	newPartition.add(group.clone());
      }
      newPartitions.add(newPartition);

      // remove the old partition
      iter.remove();
    }

    partitions.addAll(newPartitions);
  }



  /**
   * Generates all subsets of a set, represented as a hashset.
   */
  public static HashSet genSubsets(HashSet h) {
    // translates it to a vector
    Vector v = new Vector();
    for (Iterator iter = h.iterator(); iter.hasNext();) {
      v.add(iter.next());
    }
    return genSubsets(v);
  }



  /**
   * Generates all subsets of a set, represented as a vector
   */
  public static HashSet genSubsets(Vector v) {
    //UserLib.myprintln("In genSubsets(): v = " + v);
    HashSet subsets = new HashSet(); // all subsets
    buildSubsets(subsets, v, 0);
    return subsets;
  }

  /**
   * recursively build subsets
   */
  private static void buildSubsets(HashSet subsets, Vector v, int index) {
    /*UserLib.myprintln("subsets.size() = " + subsets.size() +
		      "index = " + index);*/

    if (index == v.size()) {// we reach the last element
      subsets.add(new HashSet());  // creates an empty subset
      return;
    }
    
    // recursively builds subsets
    buildSubsets(subsets, v, index + 1);

    // expanding the subsets by adding the current element to each subset
    // while keeping the old ones
    
    // iterator doesn't allow us to add new subsets, so let's use another
    // set to keep the new subsets

    //UserLib.myprintln("before: subsets() = " + subsets);
    //UserLib.myprintln("before: subsets.size() = " + subsets.size());

    HashSet newSubsets = new HashSet();
    for (Iterator iter = subsets.iterator(); iter.hasNext();) {
      HashSet subset = (HashSet) iter.next();
      HashSet newSubset = (HashSet) subset.clone();
      newSubset.add(v.elementAt(index)); // add the current element
      newSubsets.add(newSubset);
    }
    subsets.addAll(newSubsets);

    //UserLib.myprintln("after: subsets.size() = " + subsets.size());
    //UserLib.myprintln("after: subsets() = " + subsets);
  }

  /** 
   * Generates all sequences of length "length" for a set of objects
   * in which each object should appear at least once.
   */
  public static Vector getAllSeqs(Vector v, int length) {
    HashSet hs = new HashSet(v);
    return getAllSeqs(hs, length);
  }

  /** 
   * Generates all sequences of length "length" for a set of objects
   * in which each object should appear at least once.
   */
  public static Vector getAllSeqs(HashSet objectSet, int length) {
    Vector allSeqs = new Vector();
    Vector oneSeq  = new Vector();

    if (length == 0) 
      allSeqs.add(oneSeq);
    else
      buildAllSeq(objectSet, length, oneSeq, allSeqs);

    return allSeqs;
  }

  /**
   * Recursively builds the sequences.
   */
  private static void buildAllSeq(HashSet objectSet, int length, 
				  Vector oneSeq, Vector allSeqs) {

    // if there are more objects than the length, do nothing
    if (objectSet.size() > length) return;

    if (objectSet.size() == length) {
      // generates all the permutations of the objects and returns
      HashSet permutations = UserLib.genPermutations(objectSet);
      for (Iterator iter = permutations.iterator(); iter.hasNext();) {
	Vector permutation = (Vector) iter.next();
	Vector newSeq = (Vector) oneSeq.clone();
	newSeq.addAll(permutation);  // concates the two sequences
	allSeqs.add(newSeq); // generates one complete sequence
      }

      return;
    }

    // iteratively adds an object to the vector
    for (Iterator iter = objectSet.iterator(); iter.hasNext();) {
      Object obj = iter.next();

      HashSet newObjectSet = (HashSet) objectSet.clone();
      // if the object doesn't appear in the existing sequence
      if (!oneSeq.contains(obj)) {
	newObjectSet.remove(obj);
      }

      Vector  newSeq = (Vector) oneSeq.clone();
      newSeq.add(obj); 	// adds this object to the vector
      
      // recursively calls the function
      buildAllSeq(newObjectSet, length - 1,  newSeq, allSeqs);
    }
  }

  /**
   * Creates a set of all permutations (represented as vectors) for a
   * given vector of objects.
   */
  public static HashSet genPermutations(Vector v) {
    HashSet hs = new HashSet(v);
    return genPermutations(hs);
  }

  /**
   * Creates a set of all permutations (represented as vectors) for a
   * given set of objects.
   */
  public static HashSet genPermutations(HashSet objectSet) {
    HashSet allPermutations = new HashSet();
    Vector  onePermutation = new Vector();

    buildPermutations(objectSet, onePermutation, allPermutations);
    
    return allPermutations;
  }
  
  private static void buildPermutations(HashSet objectSet, 
					Vector  onePermutation, 
					HashSet allPermutations) {

    if (objectSet.isEmpty()) {
      allPermutations.add(onePermutation);
      return;
    }
    
    // iteratively adds an object to the permutation
    for (Iterator iter = objectSet.iterator(); iter.hasNext();) {
      Object obj = iter.next();
      
      // adds this object to the vector
      Vector newPermutation = (Vector) onePermutation.clone();
      newPermutation.add(obj);
      
      HashSet newObjectSet = (HashSet) objectSet.clone();
      newObjectSet.remove(obj);

      // recursively calls the function
      buildPermutations(newObjectSet, newPermutation, allPermutations);
    }
  }


  /**
   * Creates all the subsets with size k of a given hashset
   */
  public static HashSet genSubsets(HashSet objectSet, int k) {
    if (objectSet.size() < k )
      return (new HashSet()); // empty
    
    return buildSubsets(objectSet, k);
  }

  /**
   * recursive helper.
   */
  static HashSet buildSubsets(HashSet objectSet, int k) {
    HashSet result = new HashSet();

    if (k == 0) {
      result.add(new HashSet()); // empty set
      return result;
    }

    if (objectSet.size() == k ) {
      result.add(objectSet.clone());
      return result;
    }

    if (objectSet.size() == 0)
      UserLib.myerror("UserLib.buildSubsets(): error()!");

    // randomly pick an object
    Object obj = objectSet.iterator().next();

    // removed the current object
    HashSet tmpObjSet = (HashSet) objectSet.clone();
    tmpObjSet.remove(obj);

    // part 1: buildSubsets(objectSet - o, k); // without o
    HashSet result1 = buildSubsets(tmpObjSet, k); 
    result.addAll(result1);

    // part 2: buildSubsets(objectSet - o, k - 1); // with o
    HashSet result2 = buildSubsets(tmpObjSet, k - 1); 

    // add the current object to each subset in result 2
    for (Iterator iter = result2.iterator(); iter.hasNext();) {
      HashSet oneSet = (HashSet) iter.next(); // one partition
      oneSet.add(obj);
      result.add(oneSet);
    }
    
    return result;
  }

  /**
   * Checks if two sets are disjoint
   */
  public static boolean disjoint(HashSet set1, HashSet set2) {
    HashSet tmpSet = (HashSet) set1.clone();
    tmpSet.retainAll(set2);
    return (tmpSet.isEmpty());
  }

  /**
   * Unit test.
   */
  public static void main(String[] args) {

    Vector v = new Vector();
    v.add(new Integer(1));
    v.add(new Integer(2));
    v.add(new Integer(3));
    v.add(new Integer(4));
    System.out.println("vector = " + v);
    
    // tests genSubsets
    HashSet subsets = (HashSet) genSubsets(v);
    System.out.println("Subsets are:" + subsets);

    // tests genPartitions
    HashSet partitions = (HashSet) genPartitions(v);
    System.out.println("partitions are:" + partitions);

    // tests vector.clone()
    Vector newV = (Vector) v.clone();
    System.out.println("New vector = " + newV);

    newV.remove(0);
    System.out.println("After the deletion, New vector = " + newV);
    System.out.println("After the deletion, old vector = " + v);    
  }

}
