javac *.java &&
echo $'m\tn\t#MCD\t#Rew' > ../satcomp/expTipo$1/TiemposMCTipo$1.txt &&
for a in 2 4 6 8 10 12 14 16 18 20
do
 echo "empieza $a"

  java -Xmx512M GoodPlan $a $1 $2 >> ../satcomp/expTipo$1/TiemposMCTipo$1.txt

done
