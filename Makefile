
CC = g++ -O2 -Wno-deprecated

tag = -i

ifdef linux
tag = -n
endif

main.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o QueryRunner.o QueryPlan.o RelOpPlanNode.o PathConfig.o Database.o main.o y.tab.latest.o lex.yy.latest.o
	$(CC) -o main.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o Pipe.o BigQ.o RelOp.o Function.o Statistics.o QueryRunner.o QueryPlan.o RelOpPlanNode.o PathConfig.o Database.o main.o y.tab.o lex.yy.o -ll -lpthread

main.o: main.cc
	$(CC) -g -c main.cc

DBFileGTests.o: gtests/DBFileGTests.cc
	$(CC) -g -c gtests/DBFileGTests.cc

BigQGTests.o: gtests/BigQGTests.cc
	$(CC) -g -c gtests/BigQGTests.cc

SortedDBFileGTests.o: gtests/SortedDBFileGTests.cc
	$(CC) -g -c gtests/SortedDBFileGTests.cc

RelOpGTests.o: gtests/RelOpGTests.cc
	$(CC) -g -c gtests/RelOpGTests.cc

StatisticsGTests.o: gtests/StatisticsGTests.cc
	$(CC) -g -c gtests/StatisticsGTests.cc

QueryPlanGTests.o: gtests/QueryPlanGTests.cc
	$(CC) -g -c gtests/QueryPlanGTests.cc

DatabaseGTests.o: gtests/DatabaseGTests.cc
	$(CC) -g -c gtests/DatabaseGTests.cc

QueryRunner.o: QueryRunner.cc
	$(CC) -g -c QueryRunner.cc

QueryPlan.o: QueryPlan.cc
	$(CC) -g -c QueryPlan.cc

RelOpPlanNode.o: RelOpPlanNode.cc
	$(CC) -g -c RelOpPlanNode.cc

Database.o: Database.cc
	$(CC) -g -c Database.cc

PathConfig.o: PathConfig.cc
	$(CC) -g -c PathConfig.cc

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

GenericDBFile.o: GenericDBFile.cc
	$(CC) -g -c GenericDBFile.cc

HeapDBFile.o: HeapDBFile.cc
	$(CC) -g -c HeapDBFile.cc

SortedDBFile.o: SortedDBFile.cc
	$(CC) -g -c SortedDBFile.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc

y.tab.latest.o: ParserLatest.y
	yacc -d ParserLatest.y
	sed $(tag) -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" y.tab.c
	g++ -c y.tab.c

y.tab.o: Parser.y
	yacc -d Parser.y
	#sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c
		
yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	#sed $(tag) yyfunc.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c yyfunc.tab.c

lex.yy.latest.o: LexerLatest.l
	lex  LexerLatest.l
	gcc  -c lex.yy.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

lex.yyfunc.o: LexerFunc.l
	lex -Pyyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.*
	rm -f yyfunc.tab.*
	rm -f lex.yy.*
	rm -f lex.yyfunc*
