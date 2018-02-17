all:	stringServer	stringClient
stringServer:	stringServer.cc	
	g++ -o stringServer stringServer.cc
stringClient:	stringClient.cc
	g++ -o stringClient stringClient.cc

