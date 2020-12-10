#include "Structure.h"

using namespace std;

string data_path = "./";

void preprocess( string dataset, string sep = "," ) {
	if( sep.compare( "space" ) == 0 )
		sep = " ";

	PreProcessor* processor = new PreProcessor();
	string input_path = data_path + dataset + "/graph.txt";
	string output_dir = data_path + dataset + "/";
	string path_out = data_path + dataset + "_info_preprocess.txt";
	FILE *fout = fopen( path_out.c_str(), "w" );
	processor->set_fout( fout );
	processor->process_graph(input_path, output_dir, 1, -1, -1, sep );
	delete processor;
	fclose( fout );
}

void precompute( string dataset ) {
	Graph *g = new Graph();

	string path = data_path + dataset + "/";
	string path_out =  data_path + dataset + "_info_precompute.txt";
	FILE *fout = fopen( path_out.c_str(), "w" );

	g->init( path );
	g->set_fout( fout );
	g->compute_core();
	g->save_core( path );
	delete g;

	fclose( fout );
}

void process_quality( string dataset, string alg, string st_topk, bool output_graph = true ) {
	string path = data_path + dataset + "/";
	string path_out =  data_path + "quality_" + dataset +
			"_" + alg + "_" + st_topk  + ".txt";
	int topk = atoi(st_topk.c_str());
	Graph* g = new Graph();

	FILE* fout = fopen( path_out.c_str(), "w" );
	g->set_fout( fout );

	if( alg.compare( "lds" ) == 0 )
		g->ready( path );
	else
		g->init( path );

	g->set_output_factor( true );

	if( output_graph )
		g->load_node_name( path + "node_name.txt" );

	if( alg.compare( "lds") == 0 )
		g->lds_topk( topk );
	else if( alg.compare( "greedy" ) == 0 )
		g->greedy_no_verify_topk( topk );
	else if( alg.compare( "qls" ) == 0 )
		g->local_topk( topk );
	else if( alg.compare( "ldsnaive" ) == 0 )
		g->greedy_topk( topk );
	else if( alg.compare( "approx" ) == 0 )
		g->approx_topk( topk );

	fclose( fout );
	delete g;
}

void process_efficiency( string dataset, string alg, string st_topk ) {
	string path = data_path + dataset + "/";
	string path_out =  data_path + "efficiency_" + dataset +
			"_" + alg + "_" + st_topk  + ".txt";
	int topk = atoi(st_topk.c_str());
	Graph* g = new Graph();

	FILE* fout = fopen( path_out.c_str(), "w" );
	g->set_fout( fout );

	if( alg.compare( "lds" ) == 0 )
		g->ready( path );
	else
		g->init( path );

	if( alg.compare( "lds") == 0 )
		g->lds_topk( topk );
	else if( alg.compare( "greedy" ) == 0 )
		g->greedy_no_verify_topk( topk );
	else if( alg.compare( "qls" ) == 0 )
		g->local_topk( topk );
	else if( alg.compare( "ldsnaive" ) == 0 )
		g->greedy_topk( topk );
	else if( alg.compare( "approx" ) == 0 )
		g->approx_topk( topk );

	fclose( fout );
	delete g;
}

int main(int argc, char *argv[]) {
	printf( "argc=%d\n", argc );
	for( int i = 0; i < argc; ++i )
		printf( "argv[%d]=%s\n", i, argv[i] );

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	printf( "start\n" );
	long t = clock();

	if( argc > 1 ) {
		if( strcmp( argv[1], "preprocess" ) == 0 ) { //preprocess uk-2002
			string sep = ",";
			if( argc > 3 )
				sep = argv[3];
			if( argc <= 2 )
				printf( "error parameter!\n" );
			else
				preprocess( argv[2], sep );
		} else if( strcmp( argv[1], "precompute" ) == 0 ) { //precompute uk-2002
			if( argc <= 2 )
				printf( "error parameter!\n" );
			else
				precompute( argv[2] );
		} else if( strcmp( argv[1], "quality" ) == 0 ) { //quality uk-2002 lds 10
			if( argc <= 4 )
				printf( "error parameter!\n" );
			else
				process_quality( argv[2], argv[3], argv[4] );
		} else if( strcmp( argv[1], "efficiency" ) == 0 ) { //efficiency uk-2002 lds 10
			if( argc <= 4 )
				printf( "error parameter!\n" );
			else
				process_efficiency( argv[2], argv[3], argv[4] );
		}
	}

	t = clock() - t;
	printf( "end\n" );
	printf( "total time=%0.3lf sec\n", t/CLOCK_SECOND);

	return 0;
}
