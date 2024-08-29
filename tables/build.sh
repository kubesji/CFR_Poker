g++ -c descriptions.cpp dp.cpp hashtable.cpp hashtable5.cpp hashtable6.cpp hashtable7.cpp pairs.cpp possible_straights.cpp suits.cpp
ar rvs tables.a descriptions.o dp.o hashtable.o hashtable5.o hashtable6.o hashtable7.o pairs.o possible_straights.o suits.o

mkdir objects
mv *.o objects/


