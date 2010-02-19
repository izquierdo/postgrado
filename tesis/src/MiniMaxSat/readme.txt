Format files of MiniMaxSat:
---------------------------

MiniMaxSat handles cnf and wcnf files following the format specifications of the Max-SAT Evaluation 2007:
http://www.maxsat07.udl.es/


To execute MiniMaxSat:
----------------------
Files "minimaxsat_64-bit_static" and "ubcsat" must be on the same directory. Note that "ubcsat" is a local search
engine that gives an initial upper bound.


- With a cnf file:
./minimaxsat1.0 -F=1 file.cnf

- With a wcnf file:
./minimaxsat1.0 -F=2 file.wcnf

Output:
-------
It follows the output format specifications of the Max-SAT Evaluation 2007:
http://www.maxsat07.udl.es/

IMPORTANT: If all the MANDATORY CLAUSES cannot be satisfied simultaneously by any assignment, MiniMaxSat will return UNSAT within a comment line.

