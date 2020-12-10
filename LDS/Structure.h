#ifndef STRUCTURE_H_
#define STRUCTURE_H_

#include<cstring>
#include<string>
#include<vector>
#include<cstdlib>
#include<cstdio>
#include<cmath>
#include<map>
#include<algorithm>

typedef long long lint;
typedef long mylong;

#define lint_fmt "%lld"
#define MAX_LINE_CHAR 100000
#define CLOCK_SECOND 1000000.0
#define MAX_ITER 1
#define alpha 0.3333333333333

#define TMAX 50

const lint LINT_INF=(lint)2000000000*(lint)2000000000;
const int INT_INF = 2000000000;

using namespace std;

class GeneralFunction {
public:
	static void show_lint( lint p );
};

class DBLPPreProcessor{
private:
	map<string,int> map_id;
	map<int,int> map_new_id;
	int n_author;

	FILE *fout_author_id;
	FILE *fout_graph;
	FILE *fout_pagerank;

private:
	int get_node_id( const string &name );
	int get_new_node_id( const string &name, int old_node_id, double pagerank );
	void parse_author( char *line, vector<int> &v_author );

public:
	void init_domain( string path );
	void clear_domain();
	void process_dblp( string path );
	void process_domain( int domain, string path, string dataset );
	void process_domain( string path );
};

class PreProcessor{
private:
	map<int,int> map_id;
	vector<vector<int>*> con;
	int max_id;
	int min_edge_support;
	vector<int> org_id;

	FILE *fout;

private:
	unsigned hash( int val );
	int get_node_id( int id );

public:
	PreProcessor();

public:
	void set_fout( FILE *fout );
	void process_graph( string input_path, string output_dir, int my_min_edge_support = 0,
			int node_percent = -1, int edge_percent = -1, string sep = ",", int start_pos = 0 );
};

class BinNode {
public:
	BinNode *next;
	BinNode *prev;
	int id;
	int deg;

public:
	BinNode();
	void clear();
};

class DegNode {
public:
	int id;
	int core;

public:
	DegNode();
	bool operator < (const DegNode &dn ) const;
};

class HEntry{
public:
	lint val;
	HEntry* next;
public:
	HEntry();
};

class EntryHash{
private:
	static const int n_block_entry = 1000000;
	static const int init_hash_len = 1111111;

private:
	vector<HEntry*> v_block;
	vector<int> v_pos;

private:
	HEntry* get_entry();

private:
	HEntry **hash;
	int len_hash;
	int n_hash;

public:
	EntryHash();
	~EntryHash();

public:
	lint get_val( int u, int v );
	int h( lint val );
	int h( int u, int v );
	int h( HEntry *e );
	void add( int u, int v );
	HEntry* find( int u, int v );
};

class Entry {
public:
	int v;
	int rho_m;
	int rho_n;
	bool is_exact;

public:
	Entry();
};

class EntryHeap {
private:
	static const int n_block_entry = 1000000;

private:
	vector<Entry*> v_block;
	vector<int> v_pos;

public:
	EntryHeap();
	~EntryHeap();

private:
	Entry* get_entry();

private:
	vector<Entry*> heap;
	int tot;

	void up( int p );
	void down( int p );

public:
	void enheap( int v, int rho_n, int rho_m, bool is_exact );
	Entry *deheap();
	bool empty();
};

class Subgraph;
class ResultGraph;

class Graph {
public:
	int n;
	mylong m;
	int maxd;
	int **con;
	int *len;
	int *dat;
	mylong *pos;

	char* export_path;
	FILE *fout;
	mylong cnt;
	mylong my_cnt;

	string *node_name;

	int *core;
	int max_core;
	BinNode *bin_pool;
	BinNode *bin;
	mylong sum_core;

public:

	int *timestamp; 		//n
	int now_time;

	//tmp structures
	int *now_node;			//n
	int n_now_node;
	int m_now_node;

	int *tmp_node;			//n
	int n_tmp_node;
	int m_tmp_node;

	int max_rho_n;
	int max_rho_m;

	int *tmp_info;			//n

	bool *valid;			//n
	bool *valid_densest;	//n
	bool *in_lds;			//n
	int *lrho_m;   			//n
	int *lrho_n;   			//n
	int *urho_m;			//n
	int *urho_n;			//n
	bool *ureach;			//n; whether the upper bound can be reached
	int min_rho;

	EntryHeap *heap;

public:
	long start_clock;
public:
	void clear();
	Graph();
	~Graph();

private:
	void update_binnode( BinNode *bn );
	//int get_f( int p, int *f );

public:
	static bool better( lint n1, lint m1, lint n2, lint m2 );

public:
	mylong get_cnt();
	mylong get_my_cnt();
	int get_max_core();

	void set_export_path( char *export_path );
	void set_fout(FILE *fout);

	void init( string path );
	void compute_core();
	void sort_neighbors();

	void save_core( string path );
	void load_core( string path, bool is_sort_neighbors = true );

	void load_node_name( string path );

public:
	void set_min_rho( int now_min_rho );
	void prune_by_core();
	int compute_core_by_valid();

	int bfs( int s, int min_core = -1, bool only_valid = true, int print_min_size = -1,
			bool update_valid = false );
	//void dfs_invalid( int u );
	int compute_invalid( int u );

	void init_heap();
	void test();

	bool is_valid( int u, int v ); //whether u is valid because of v
	bool is_valid( int u );
	void save_tmp_node();
	void save_now_node();
	void compute_udensity( int &u_m, int &u_n );
	bool update_l( int u, int l_n, int l_m );
	bool update_u( int u, int u_n, int u_m, bool reach );
	void add_to_heap( int u );

public:
	void fill_subgraph( Subgraph *s ); //fill the subgraph using tmp_node
	void bfs_densest( int v );
	void bfs_now_time( int v );

public:
	bool is_lds( int v );
	bool is_lds_in_core( int v );
	void compute_densest( int v );
	//void update_heap_with_densest( int v );

public:
	void ready( string path, int min_rho = -1 );
	int next();
	void lds_topk( int k );

public://for greedy algorithm with/without verification
	int compute_densest_greedy();
	void init_greedy();
	void init_greedy_no_verify();
	void next_greedy();
	void next_greedy_no_verify();
	void greedy_topk( int k );
	void greedy_no_verify_topk( int k );

public: // for approx
	double f_density( int n, int m );
	void init_approx();
	void approx_next();
	void approx_topk( int k );

public:
	void show_shortest_path( int s, int t );

public://for quasi-cliques local
	int *nowd;					//the directed degree of every node
	int *tcnt;					//the number of triangles for each node
	bool *used;
	EntryHash *hash;

	bool greater( int u, int v );
	void count_triangle();		//count the triangle for all valid nodes
	void init_quasi_clique_local();
	void remove_triangle_node( int u );
	void print_tcnt();
	void local_next(); //save to now_node
	void local_topk( int k );
	bool local_add();
	bool local_del();

public:
	bool output_factor;
	ResultGraph* create_result_graph(); //create result graph by now nodes
	void output_result_factor( int topk );
	void set_output_factor( bool output_factor );
};

class Subgraph{ 		//n nodes, id start from 1
public:
	int n, m;
	int tot;
	int s, t;			//s=0, t=n+1

	int *px;			//m+1
	int *py;			//m+1
	int *in;			//n+2

	int *a;				//2m+4n+2
	int *next;			//2m+4n+2
	lint *c;			//2m+4n+2

	int *start;			//n+2
	int *level;			//n+2
	int *now;			//n+2

	int n_now_node;
	int m_now_node;
	int *now_node;		//n+3
	bool *used;			//n+2

	int n_dst_node;
	int m_dst_node;
	int *dst_node;		//n+3

	int *org_id;		//n+2; the id in graph g

	Graph *g;

public:
	Subgraph();
	~Subgraph();

public:
	void init( int nown, int nowm );
	void clear();

public:
	void init( Graph *g ); //initialize subgraph using tmp_node in g
	void add(int x, int y, lint z, bool t);
	void make_map( lint rho );
	bool bfs();
	lint dinic(int u, lint l);
	bool try_it( lint rho );
	void bfs( int u, bool update_g = false );
	void bfs_one( int u );
	void save_to_dst();
	void find_densest( lint max_rho_n, lint max_rho_m );
	void find_rho_compact( lint rho_n, lint rho_m ); //rho=rho_m/rho_n
};

class ResultGraph {
public:
	int n, m;
	int *len;
	vector<int> *con;
	int *org_id;
	Graph *g;

public:
	double density;
	double edge_density;
	double triangle_density;
	int diameter;
	int connectivity;
	int min_degree;
	double conductance;
	double avg_distance;
	double cluster_coefficient;
	double relative_density;

public:
	ResultGraph();
	~ResultGraph();

public:
	void clear();

public:
	bool *used;
	int now_time;
	int *timestamp;
	void compute_all_factor();

public:
	void compute_cut_factor();

public:
	void compute_dis_factor();

public:
	int **adj;
	int *dis;
	int *p;

	void compute_connectivity();

public:
	bool greater( int u, int v );
	void compute_triangle_factor();
};


#endif /* STRUCTURE_H_ */
