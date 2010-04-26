/* 
 * PhiHH.java
 * -------------
 * A simple class that includes a mapping \phi and a head homomorphism
 * $Id: PhiHH.java,v 1.3 2000/10/28 04:24:43 chenli Exp $
 */

import java.util.*;

class PhiHH {
  public Mapping phi = null;
  public Mapping hh  = null;
  
  PhiHH(Mapping phi, Mapping hh) {
    this.phi = phi;
    this.hh  = hh;
  }

  public Object clone() {
    return new PhiHH((Mapping) phi.clone(), (Mapping) hh.clone());
  }

  public Mapping getPhi() {
    return phi;
  }

  public Mapping getHH() {
    return hh;
  }

  public String toString() {
    return "Phi = " + getPhi().toString() + ", hh = " + getHH().toString();
  }
}
