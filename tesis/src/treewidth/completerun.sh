FILE=$1
ELG=`basename ${FILE} .cnf`.primal.elg
REDUCED=$ELG.reduced

./cnf2elg.py ${FILE} primal
./get_graph_stats/get_graph_stats ${ELG} -g ${REDUCED}
./TWSearch/TWSearch -m 1536 -e ${REDUCED}
