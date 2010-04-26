

Implementation of the CoreCover Algorithm

Chen Li (chenli@cs.stanford.edu)

Generating Efficient Plans for Queries Using Views. Foto Afrati, Chen Li, and Jeff Ullman. In the Proc. of the 30th ACM SIGMOD Conference, Santa Barbara, CA, May, 2001

Experiments:

1. Probability of having rewritings:

    - random (small)
    - chain  (small)
    - star   (large)   --> time consuming
        o no variables dropped
        o 1 variable dropped
        o 2 variables dropped

  Choose small number of query/view subgoals to make sure there is a rewriting.

 Setting: 
		 relationNum     = 6;  // # of relations
		 relAttrNum      = 3;  // # of rel attrs 
		 querySubgoalNum = 6;  // # of subgoals in a query
		 viewSubgoalNum  = 2;  // # of subgoals in a view
		 dropHeadArgNum  = 0;  // # of args dropped a head
		 queryArgDomSize = 5;  // size of arg domain in a query
		 simQueryNum     = 10; // # of queries

   Files:

     - prob-random.data: random
     - prob-chain.data:  chain // all vars are distinguished
     - prob-star.data:   star  

   The files with a ".conf" extension has more detailed info.

2. number/time of GMRs, chain queries (also comment on other queries):
 
3. Effects of shrinking views and view tuples, star queries (also comment
   on other queries):

  - # of views;
  - # of view tuples

  Increase the number of query/view subgoals to show the scalability.
