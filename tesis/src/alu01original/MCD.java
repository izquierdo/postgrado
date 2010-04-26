/* 
 * MCD.java
 * --------
 * $Id: MCD.java,v 1.15 2000/10/30 02:07:48 chenli Exp $
 */

import java.util.*;

class MCD {
  Query    query         = null;
  HashSet  mappedQuerySubgoals = null;
  Query    view          = null;  // hh has been already applied on the view
  HashSet  viewSubgoals  = null;
  Mapping  phi           = null;   // a mapping from query subgoals to 
  Mapping  hh            = null;   // head homomorphism on the view 
  Mapping  psi           = null;   // psi
    
  MCD(Query query, HashSet mappedQuerySubgoals, 
      Query view,  HashSet viewSubgoals, 
      Mapping phi, Mapping hh) {
    this.query         = query;
    this.mappedQuerySubgoals = mappedQuerySubgoals;
    this.view          = view;
    this.viewSubgoals  = viewSubgoals;
    this.phi           = phi;
    this.hh            = hh;
  }

  /**
   * Indicates whether some other MCD is "equal to" this one.
   * We compare their queries, views, mapped query subgoals,
   * and used view subgoals.
   */
  public boolean equals(Object tmpmcd){
    MCD mcd = (MCD) tmpmcd;
    return this.getQuery().equals(mcd.getQuery()) 
      && this.getView().equals(mcd.getView())
      && this.getMappedQuerySubgoals().equals(mcd.getMappedQuerySubgoals())
      && this.getViewSubgoals().equals(mcd.getViewSubgoals());
  }

  public int hashCode() {
    return (query.getName().hashCode() + view.getName().hashCode() );
  }
  
  /**
   * Overrides the hash function Object.hasCode().  As an item in a
   * HashSet collection, called when HashSet.equals() is called.
   */
  //  public int hashCode() {
  // TODO
  //    return 100;
  //}

  public Query getQuery() {
    return query;
  }

  public Query getView() {
    return view;
  }

  // returns the view after applying the hh on it
  /*  public Query getHHedView() { 
    return hh.apply(view);
  }*/

  public Mapping getPhi() {
    return phi;
  }

  public Mapping getHH() {
    return hh;
  }

  public Mapping getPsi() {
    return psi;
  }

  public HashSet getMappedQuerySubgoals() {
    return mappedQuerySubgoals;
  }

  public HashSet getViewSubgoals() {
    return viewSubgoals;
  }

  public void setPsi(Mapping psi) {
    this.psi = psi;
  }

  public String toString() {
    return " v = { " + view.head.name.toString() + " }" +
      " g = { " + mappedQuerySubgoals.toString() + " }" +
      " phi = { " + phi.toString() + " }" +
        " hh = { " + hh.toString() + " }\n" ;
  }
}
