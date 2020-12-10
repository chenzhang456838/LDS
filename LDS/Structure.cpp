#include "Structure.h"


//======GeneralFunction======
void GeneralFunction::show_lint( lint p ) {
	printf( lint_fmt, p );
}

//======DBLPPreProcessor======
int DBLPPreProcessor::get_node_id( const string &name ) {
	if( map_id.find( name ) != map_id.end() )
		return map_id[name];
	fprintf( fout_author_id, "%d&%s\n", n_author, name.c_str() );
	map_id[name] = n_author++;
	return n_author-1;
}

int DBLPPreProcessor::get_new_node_id( const string &name, int old_node_id, double pagerank ) {
	if( map_new_id.find(old_node_id) != map_new_id.end() )
		return map_new_id[old_node_id];
	fprintf( fout_author_id, "%d&%s\n", n_author, name.c_str() );
	fwrite( &pagerank, sizeof(double), 1, fout_pagerank );
	map_new_id[old_node_id] = n_author++;
	return n_author-1;
}

void DBLPPreProcessor::parse_author( char *line, vector<int> &v_author ) {
	v_author.clear();
	vector<char *> v_author_st;
	int len = strlen( line );
	if( len == 0 )
		return;
	if( line[len-1] == '\n' )
		line[--len] = '\0';
	if( len == 0 )
		return;
	int cnt = 0;
	for( int i = 0; i < len; ++i )
		if( line[i] == '&' ) {
			line[i] = '\0';
			++cnt;
		}
	if( cnt == 0 )
		return;

	for( int i = 0; i < len; ++i )
		if( line[i] )
			if( i == 0 || (line[i-1] == '\0' ) ) {
				bool dup = false;
				for( int j = 0; j < (int)v_author_st.size(); ++j )
					if( strcmp( line+i, v_author_st[j] ) == 0 )
						dup = true;
				if( dup )
					continue;
				v_author_st.push_back( line+i );
			}

	if( (int) v_author_st.size() >= 2 )
		for( int i = 0; i < (int) v_author_st.size(); ++i ) {
			string name = string(v_author_st[i]);
			v_author.push_back( get_node_id(name) );
		}
}

void DBLPPreProcessor::process_dblp( string path ) {
	char *line = new char[MAX_LINE_CHAR];
	string f_path = path + "author.txt";

	vector<int> v_author;

	int m = 0;
	FILE *fin = fopen( f_path.c_str(), "r" );

	fout_author_id = fopen( (path + "author_id.txt").c_str(), "w" );
	fout_graph = fopen( (path + "graph.txt").c_str(), "w" );

	int cnt = 0;
	while( fgets( line, MAX_LINE_CHAR, fin ) ) {
		if( ++cnt % 100000 == 0 )
			printf( "%d/%d:%0.3lf%%,m=%d\n", cnt, 3910991, cnt * 100.0/3910991, m );
		parse_author( line, v_author );
		for( int i = 0; i < (int) v_author.size(); ++i )
			for( int j = i+1; j < (int) v_author.size(); ++j ) {
				int vi = v_author[i];
				int vj = v_author[j];
				fprintf( fout_graph, "%d,%d\n", vi, vj );
				++m;
			}
	}

	fclose( fin );
	fclose( fout_author_id );
	fclose( fout_graph );
	delete[] line;
}

void DBLPPreProcessor::init_domain( string path ) {
	fout_author_id = fopen( (path + "author_id.txt").c_str(), "w" );
	fout_graph = fopen( (path + "graph.txt").c_str(), "w" );
	fout_pagerank = fopen( (path + "pagerank.dat" ).c_str(), "wb" );
}

void DBLPPreProcessor::clear_domain() {
	fclose( fout_author_id );
	fclose( fout_graph );
	fclose( fout_pagerank );
}

void DBLPPreProcessor::process_domain( string path ) {
	init_domain( path );
	process_domain( 0, path, "graph.net" );
	clear_domain();
}

void DBLPPreProcessor::process_domain( int domain, string path, string dataset ) {
	domain *= 10000000;
	char *line = new char[MAX_LINE_CHAR];
	vector<string> v_name;
	vector<int> v_w;

	string f_path = path + dataset;

	FILE *fin = fopen( f_path.c_str(), "r" );

	int n;

	fscanf( fin, "%s %d", line, &n );
	fgets( line, MAX_LINE_CHAR, fin );
	for( int i = 0; i < n; ++i ) {
		fgets( line, MAX_LINE_CHAR, fin );
		int p1 = 0;
		int p2 = strlen(line);
		while( line[p1] != '"' && p1 < p2 )
			++p1;
		while( line[p2] != '"' && p2 > p1 )
			--p2;
		if( p1 >= p2 ) {
			printf( "error %s\n", line );
		}
		line[p1++] = '\0';
		line[p2++] = '\0';

		int w;
		v_name.push_back( string(line+p1) );
		sscanf( line+p2, "%d", &w );
		v_w.push_back( w );
	}


	fgets( line, MAX_LINE_CHAR, fin );

	while( fgets( line, MAX_LINE_CHAR, fin ) ) {
		if( line[0] == '*' )
			break;
		int id1, id2, w;
		sscanf( line, "%d %d %d", &id1, &id2, &w );
		--id1;
		--id2;
		int new_id1 = get_new_node_id( v_name[id1], domain+id1, (double)v_w[id1] );
		int new_id2 = get_new_node_id( v_name[id2], domain+id2, (double)v_w[id2] );
		fprintf( fout_graph, "%d,%d\n", new_id1, new_id2 );
	}

	fclose( fin );

	delete[] line;
}


//======PreProcessor======

PreProcessor::PreProcessor() {
	min_edge_support = 0;
	max_id = -1;
	fout = NULL;
}

void PreProcessor::set_fout( FILE *fout ) {
	this->fout = fout;
}

int PreProcessor::get_node_id( int id ) {
	if( map_id.find(id) == map_id.end() ) {
		map_id[id] = max_id;
		con.push_back(new vector<int>());
		org_id.push_back( id );
		return max_id++;
	}
	return map_id[id];
}

unsigned PreProcessor::hash( int val ) {
	return (unsigned) (val * 157 + 2131);
}

void PreProcessor::process_graph( string input_path, string output_dir, int my_min_edge_support,
		int node_percent, int edge_percent, string sep, int start_pos ) {
	min_edge_support = my_min_edge_support;
	max_id = 0;
	con.clear();
	org_id.clear();
	map_id.clear();
	mylong cnt_edge_s = 0;

	mylong n_diff = 0;
	FILE *fin = fopen( input_path.c_str(), "r" );
	int a, b;
	int cnt = 0;

	char line[1024];
	string fmt = "%d" + sep + "%d";

	while( fgets( line, 1024, fin ) ) {
		if( line[start_pos] < '0' || line[start_pos] > '9' )
			continue;
		sscanf( line+start_pos, fmt.c_str(), &a, &b );

		if( a == b )
			continue;
		//printf( "line:%s\n", line );

		if( node_percent > 0 ) {
			if( (hash(a) % 100) >= node_percent )
				continue;
			if( (hash(b) % 100) >= node_percent )
				continue;
		}

		if( edge_percent > 0 ) {
			++cnt;
			if( (hash(cnt) % 100) >= edge_percent )
				continue;
		}

		int u = get_node_id(a);
		int v = get_node_id(b);

		con[u]->push_back(v);
		con[v]->push_back(u);
		++cnt_edge_s;
		if( cnt_edge_s < 100 )
			printf( "(%d,%d)->(%d,%d)\n", a, b, u, v );
		if( a != u || b != v )
			++n_diff;
		if( cnt_edge_s % 1000000 == 0 )
			printf( "i:%ld\n", cnt_edge_s );
	}

	fclose( fin );

	printf( "n_diff=%ld\n", n_diff );
	if( fout )
		fprintf( fout, "n_diff=%ld\n", n_diff );

	mylong cnt_edge_t = 0;

	for( int i = 0; i < max_id; ++i ) {
		sort( con[i]->begin(), con[i]->end() );
		int len_s = con[i]->size();
		int len_t = 0;
		int now_cnt = 1;
		for( int j = 1; j <= len_s; ++j ) {
			bool reset = true;
			if( j < len_s )
				if( (*con[i])[j] == (*con[i])[j-1] ) {
					++now_cnt;
					reset = false;
				}
			if( reset ) {
				if( now_cnt >= min_edge_support )
					(*con[i])[len_t++] = (*con[i])[j-1];
				now_cnt = 1;
			}
		}
		con[i]->resize( len_t );
		cnt_edge_t += len_t;
		if( i < 100 )
			printf( "%d->%d\n", len_s, len_t );
	}

	printf( "creating index\n" );
	string st_idx = output_dir + "graph.idx";
	string st_dat = output_dir + "graph.dat";

	FILE *fout_idx = fopen( st_idx.c_str(), "wb" );
	FILE *fout_dat = fopen( st_dat.c_str(), "wb" );

	fwrite( &max_id, sizeof(int), 1, fout_idx );
	fwrite( &cnt_edge_t, sizeof(mylong), 1, fout_dat );

	mylong now_pos = 0;
	for( int i = 0; i < max_id; ++i ) {
		int len = con[i]->size();
		for( int j = 0; j < len; ++j ) {
			int now = (*con[i])[j];
			fwrite( &now, sizeof(int), 1, fout_dat );
		}
		now_pos += len;
		fwrite( &now_pos, sizeof(mylong), 1, fout_idx );
	}

	fclose( fout_idx );
	fclose( fout_dat );

	printf( "cnt_edge_s=%ld, cnt_edge_t=%ld, now_pos=%ld, memory=%0.3lfMB\n", cnt_edge_s,
			cnt_edge_t, now_pos, cnt_edge_t*4.0/(1024*1024) );
	if( fout )
		fprintf( fout, "cnt_edge_s=%ld, cnt_edge_t=%ld, now_pos=%ld, memory=%0.3lfMB\n", cnt_edge_s,
				cnt_edge_t, now_pos, cnt_edge_t*4.0/(1024*1024) );
	for( int i = 0; i < max_id; ++i )
		delete con[i];

	FILE *fout_name = fopen( (output_dir + "node_name.txt").c_str(), "w" );
	for( int i = 0; i < max_id; ++i )
		fprintf( fout_name, "%d&%d\n", i, org_id[i] );
	fclose( fout_name );
}


//======BinNode======
BinNode::BinNode() {
	prev = NULL;
	next = NULL;
	id = -1;
	deg = 0;
}

void BinNode::clear() {
	prev = NULL;
	next = NULL;
	id = -1;
	deg = 0;
}

//======DegNode
DegNode::DegNode() {
	id = -1;
	core = -1;
}

bool DegNode::operator < (const DegNode &dn ) const {
	return core > dn.core;
}


//======Graph======
Graph::Graph() {
	n = 0;
	m = 0;
	maxd = 0;
	max_core = 0;
	sum_core = 0;

	min_rho = 0;

	now_time = 0;
	now_node = NULL;
	n_now_node = 0;
	m_now_node = 0;

	cnt = 0;
	my_cnt = 0;

	con = NULL;
	len = NULL;
	dat = NULL;
	pos = NULL;

	core = NULL;
	bin_pool = NULL;
	bin = NULL;

	timestamp = NULL;

	lrho_m = NULL;
	lrho_n = NULL;
	urho_m = NULL;
	urho_n = NULL;
	ureach = NULL;

	node_name = NULL;

	export_path = NULL;
	fout = NULL;

	valid = NULL;
	heap = NULL;

	tmp_node = NULL;
	n_tmp_node = 0;
	m_tmp_node = 0;

	valid_densest = NULL;
	in_lds = NULL;
	tmp_info = NULL;

	max_rho_n = 0;
	max_rho_m = 0;

	nowd = NULL;
	hash = NULL;
	tcnt = NULL;
	used = NULL;

	output_factor = false;
	start_clock = clock();
}

void Graph::clear() {
	if( con != NULL ) {
		delete[] con;
		con = NULL;
	}

	if( len != NULL ) {
		delete[] len;
		len = NULL;
	}

	if( dat != NULL ) {
		delete[] dat;
		dat = NULL;
	}

	if( pos != NULL ) {
		delete[] pos;
		pos = NULL;
	}

	if( core != NULL ) {
		delete[] core;
		core = NULL;
	}

	if( bin_pool != NULL ) {
		delete[] bin_pool;
		bin_pool = NULL;
	}

	if( bin != NULL ) {
		delete[] bin;
		bin = NULL;
	}

	if( node_name != NULL ) {
		delete[] node_name;
		node_name = NULL;
	}

	if( valid != NULL ) {
		delete[] valid;
		valid = NULL;
	}

	if( now_node != NULL ) {
		delete[] now_node;
		now_node = NULL;
	}

	if( timestamp != NULL ) {
		delete[] timestamp;
		timestamp = NULL;
	}

	if( lrho_n != NULL ) {
		delete[] lrho_n;
		lrho_n = NULL;
	}

	if( lrho_m != NULL ) {
		delete[] lrho_m;
		lrho_m = NULL;
	}

	if( urho_n != NULL ) {
		delete[] urho_n;
		urho_n = NULL;
	}

	if( urho_m != NULL ) {
		delete[] urho_m;
		urho_m = NULL;
	}

	if( ureach != NULL ) {
		delete[] ureach;
		ureach = NULL;
	}

	if( heap != NULL ) {
		delete heap;
		heap = NULL;
	}

	if( tmp_node != NULL ) {
		delete[] tmp_node;
		tmp_node = NULL;
	}

	if( valid_densest != NULL ) {
		delete[] valid_densest;
		valid_densest = NULL;
	}

	if( in_lds != NULL ) {
		delete[] in_lds;
		in_lds = NULL;
	}

	export_path = NULL;
	fout = NULL;

	if( tmp_info != NULL ) {
		delete[] tmp_info;
		tmp_info = NULL;
	}

	if( nowd != NULL ) {
		delete[] nowd;
		nowd = NULL;
	}

	if( hash != NULL ) {
		delete hash;
		hash = NULL;
	}

	if( tcnt != NULL ) {
		delete[] tcnt;
		tcnt = NULL;
	}

	if( used != NULL ) {
		delete[] used;
		used = NULL;
	}

	printf( "finished clearing memory\n" );
}

Graph::~Graph() {
	clear();
}

void Graph::update_binnode( BinNode *bn ) {
	int d = bn->deg;
	if( bn->prev != NULL )
		bn->prev->next = bn->next;
	if( bn->next != NULL )
		bn->next->prev = bn->prev;

	BinNode *head = bin+d;
	bn->next = head->next;
	bn->prev = head;
	if( head->next != NULL )
		head->next->prev = bn;
	head->next = bn;
}


mylong Graph::get_cnt() {
	return cnt;
}

mylong Graph::get_my_cnt() {
	return my_cnt;
}

void Graph::set_fout( FILE *fout ) {
	this->fout = fout;
}

void Graph::set_export_path( char *export_path ) {

	this->export_path = export_path;
}

void Graph::init( string path ){
	string st_idx = path + "graph.idx";
	string st_dat = path + "graph.dat";

	FILE *fin_idx = fopen( st_idx.c_str(), "rb" );
	FILE *fin_dat = fopen( st_dat.c_str(), "rb" );

	fread( &n, sizeof(int), 1, fin_idx );
	fread( &m, sizeof(mylong), 1, fin_dat );
	maxd = 0;

	con = new int*[n];
	len = new int[n];
	dat = new int[m];
	pos = new mylong[n];

	fread( pos, sizeof(mylong), n, fin_idx );
	fread( dat, sizeof(int), m, fin_dat );

	fclose( fin_idx );
	fclose( fin_dat );

	for( int i = 0; i < n; ++i ) {
		int nowlen = (int)(((i==0) ? pos[i] : pos[i]-pos[i-1]));
		len[i] = nowlen;
		con[i] = dat + ((i==0)?0:pos[i-1]);
		maxd = ((nowlen>maxd)? nowlen : maxd);
	}

	printf( "n=%d, m=%ld, maxd=%d\n", n, m, maxd );
	if( fout )
		fprintf( fout, "n=%d, m=%ld, maxd=%d\n", n, m, maxd );

	delete[] pos;
	pos = NULL;

	//for( int i = 0; i < 100; ++i ) {
	//	for( int j = 0; j < len[i]; ++j )
	//		printf( "(%d,%d)", i, con[i][j] );
	//	printf( "\n" );
	//}

	timestamp = new int[n];
	memset( timestamp, 0, sizeof(int) * n );

	now_time = 0;
	now_node = new int[n];
	tmp_node = new int[n];
	tmp_info = new int[n];
	printf( "init finish\n" );
}

void Graph::compute_core() {

	printf( "computing core...\n" );
	if( fout )
		fprintf( fout, "computing core...\n" );

	core = new int[n];

	memset( core, -1, sizeof(int) * n );

	bin = new BinNode[maxd+1];
	bin_pool = new BinNode[n];

	for( int i = 0; i < n; ++i ) {
		bin_pool[i].id = i;
		bin_pool[i].deg = len[i];
		update_binnode( bin_pool+i );
	}

	max_core = 0;
	sum_core = 0;
	for( int i = 0; i <= maxd; ++i ) {
		BinNode *bn = bin+i;
		while( bn->next != NULL )
			bn = bn->next;

		int cnt = 0;
		while( bn != bin+i ) {
			int u = bn->id;

			core[u] = i;
			++cnt;
			if( i > max_core )
				max_core = i;
			sum_core += i;
			for( int j = 0; j < len[u]; ++j ) {
				int v = con[u][j];
				if( bin_pool[v].deg > i ) {
					--bin_pool[v].deg;
					update_binnode( bin_pool + v );
				}
			}
			bn = bn->prev;
		}
		if( cnt > 0 )
			printf( "%d:%d\n", i, cnt );
	}

	//for( int i = 0; i < 100 && i < n; ++i )
	//	printf( "%d:%d\n", i, core[i] );

	printf( "max_core=%d,sum_core=%ld\n", max_core,sum_core );
	if( fout )
		fprintf( fout, "max_core=%d,sum_core=%ld\n", max_core,sum_core );

	//delete[] bin;
	//bin = NULL;

	//delete[] bin_pool;
	//bin_pool = NULL;

	sort_neighbors();
}

void Graph::sort_neighbors() {
	printf( "sorting neighbors...\n" );

	DegNode *list_dn = new DegNode[maxd];
	for( int u = 0; u < n; ++u ) {
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			list_dn[j].id = v;
			list_dn[j].core = core[v];
		}
		sort( list_dn, list_dn+len[u] );
		for( int j = 0; j < len[u]; ++j ) {
			con[u][j] = list_dn[j].id;
			//printf( "(%d,%d)\n", u, con[u][j] );
		}
	}

	delete[] list_dn;
}


void Graph::save_core( string path ) {
	printf( "saving core...\n" );
	path = path + "core.dat";
	FILE* fout = fopen( path.c_str(), "wb" );
	fwrite( core, sizeof(int), n, fout );
	fclose( fout );
}

void Graph::load_core( string path, bool is_sort_neighbors ) {
	printf( "loading core...\n" );
	core = new int[n];
	path = path + "core.dat";
	FILE* fout = fopen( path.c_str(), "rb" );
	fread( core, sizeof(int), n, fout );
	fclose( fout );

	max_core = 0;
	sum_core = 0;

	for( int i = 0; i < n; ++i ) {
		sum_core += core[i];
		max_core = core[i] > max_core ? core[i] : max_core;
	}

	printf( "max_core=%d,sum_core=%ld\n", max_core,sum_core );

	if( is_sort_neighbors )
		sort_neighbors();
}

void Graph::load_node_name( string path ) {
	printf( "loading node name...\n" );
	char *line = new char[1024];
	node_name = new string[n];
	FILE *fin = fopen( path.c_str(), "r" );

	while( fgets( line, 1024, fin ) ) {
		int len = strlen( line );
		if( len == 0 )
			continue;
		if( line[len-1] == '\n' )
			line[--len] = '\0';
		if( len == 0 )
			continue;
		for( int i = 0; i < len; ++i )
			if( line[i] == '&' ) {
				line[i] = '\0';
				int id = atoi(line);
				node_name[id] = line+i+1;
				break;
			}
	}

	fclose( fin );
	delete[] line;
}

//int Graph::get_f( int p, int *f ) {
//	if( f[p] != p )
//		f[p] = get_f( f[p], f );
//	return f[p];
//}

bool Graph::better( lint n1, lint m1, lint n2, lint m2 ) {
	return m1 * n2 > m2 * n1;
}

void Graph::set_min_rho( int now_min_rho ) {
	min_rho = now_min_rho;
}

bool Graph::is_valid( int u, int v ) {
	if( better( lrho_n[v], lrho_m[v], urho_n[u], urho_m[u] ) )
		return false;
	if( !ureach[u] && (lint) lrho_n[v] * (lint) urho_m[u] == (lint) lrho_m[v] * (lint) urho_n[u] )
		return false;
	return true;
}

bool Graph::is_valid( int u ) {
	if( better( lrho_n[u], lrho_m[u], urho_n[u], urho_m[u] ) )
		return false;
	if( !ureach[u] && (lint) lrho_n[u] * (lint) urho_m[u] == (lint) lrho_m[u] * (lint) urho_n[u] )
		return false;
	if( min_rho > 0 && (lint) urho_m[u] < min_rho * (lint) urho_n[u] )
		return false;
	return true;
}

bool Graph::update_l( int u, int l_n, int l_m ) {
	if( lrho_n == NULL )
		return false;

	if( better( l_n, l_m, lrho_n[u], lrho_m[u] ) ) {
		lrho_n[u] = l_n;
		lrho_m[u] = l_m;
		return true;
	}
	return false;
}

bool Graph::update_u( int u, int u_n, int u_m, bool reach ) {
	if( better( urho_n[u], urho_m[u], u_n, u_m ) ) {
		urho_n[u] = u_n;
		urho_m[u] = u_m;
		ureach[u] = reach;
		return true;
	}
	if( !reach && ureach[u] )
		if( (lint) urho_n[u] * (lint) u_m == (lint) urho_m[u] * (lint) u_n ) {
			urho_n[u] = u_n;
			urho_m[u] = u_m;
			ureach[u] = reach;
			return true;
		}
	return false;
}

void Graph::save_tmp_node() {
	n_now_node = n_tmp_node;
	m_now_node = m_tmp_node;
	memcpy( now_node, tmp_node, sizeof(int) * n_now_node );
}

void Graph::save_now_node() {
	n_tmp_node = n_now_node;
	m_tmp_node = m_now_node;
	memcpy( tmp_node, now_node, sizeof(int) * n_tmp_node );
}

void Graph::prune_by_core() {
	printf( "pruning by core...\n" );
	lrho_m = new int[n];
	lrho_n = new int[n];
	urho_m = new int[n];
	urho_n = new int[n];
	ureach = new bool[n];

	for( int u = 0; u < n; ++u ) {
		lrho_m[u] = core[u];
		lrho_n[u] = 2;
		urho_m[u] = core[u];
		urho_n[u] = 1;
		ureach[u] = true;
	}

	valid = new bool[n];

	int cnt_valid = 0;
	for( int u = 0; u < n; ++u ) {
		valid[u] = true;
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			if( !is_valid( u, v ) ) {
				valid[u] = false;
				break;
			}
		}
		if( !is_valid( u ) )
			valid[u] = false;
		if( valid[u] )
			++cnt_valid;
	}

	printf( "cnt_valid=%d,n=%d\n", cnt_valid, n );

	++now_time;
	int cur_time = now_time;
	for( int u = 0; u < n; ++u )
		if( valid[u] && timestamp[u] < cur_time )
			compute_invalid( u );

	//n_now_node = 0;
	//for( int u = 0; u < n; ++u )
	//	if( valid[u] )
	//		now_node[n_now_node++] = u;
	//for( int i = 1; i <= 100; ++i ) {
	//	int cnt_change = compute_core_by_valid();
	//	printf( "[%d]:cnt_change=%d\n", i, cnt_change );
	//	if( cnt_change == 0 )
	//		break;
	//}


	//++now_time;
	//for( int u = 0; u < n; ++u )
	//	bfs( u, -1, true );

	//++now_time;
	//bfs( 0, -1, false, 0 );

	cnt_valid = 0;
	for( int u = 0; u < n; ++u )
		if( valid[u] )
			++cnt_valid;

	printf( "cnt_valid=%d\n", cnt_valid );
}

void Graph::init_heap() {
	printf( "init_heap...\n" );
	int n_comp = 0;
	++now_time;

	heap = new EntryHeap();
	for( int u = 0; u < n; ++u )
		if( timestamp[u] != now_time && valid[u]) {
			++n_comp;
			add_to_heap( u );
		}


	printf( "n_comp=%d\n", n_comp );

	valid_densest = new bool[n];
	in_lds = new bool[n];

	memset( in_lds, 0, sizeof(bool) * n );
	memset( valid_densest, 0, sizeof(bool) * n );

	++now_time;

	/*
	for( int i = 0; i < 300; ++i ) {
		Entry* e = heap->deheap();
		if( e != NULL ) {
			bfs( e->v, -1, true );
			printf( "%d:%0.3lf:n=%d,m=%d\n", i+1, e->rho_m*1.0/e->rho_n, n_tmp_node, m_tmp_node );
		}
	}
	*/
}

int Graph::compute_core_by_valid() {
	if( n_now_node == 0 )
		return 0;

	//memset( bin, 0, sizeof(BinNode) * (maxd+1) );
	//memset( bin_pool, 0, sizeof(BinNode) * n );

	int now_maxd = 0;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( valid[u] ) {
			bin_pool[u].clear();
			now_maxd = max(len[u], now_maxd);
		}
	}

	memset( bin, 0, sizeof(BinNode) * (now_maxd+1) );

	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( valid[u] ) {
			bin_pool[u].id = u;
			for( int j = 0; j < len[u]; ++j ) {
				int v = con[u][j];
				if( valid[v] )
					++bin_pool[u].deg;
			}
			update_binnode( bin_pool+u );
		}
	}

	int cnt_change = 0;
	for( int i = 0; i <= now_maxd; ++i ) {
		BinNode *bn = bin+i;
		while( bn->next != NULL )
			bn = bn->next;

		while( bn != bin+i ) {
			int u = bn->id;

			if( i * (lint) urho_n[u] < (lint) urho_m[u] ) {
				urho_m[u] = i;
				urho_n[u] = 1;
				ureach[u] = true;
			}

			for( int j = 0; j < len[u]; ++j ) {
				int v = con[u][j];
				if( valid[v] && bin_pool[v].deg > i ) {
					--bin_pool[v].deg;
					update_binnode( bin_pool + v );
				}
				if( !is_valid( u, v ) )
					valid[u] = false;
			}

			if( !is_valid( u ) )
				valid[u] = false;

			if( !valid[u] )
				++cnt_change;

			bn = bn->prev;
		}
	}

	return cnt_change;
}

int Graph::bfs( int s, int min_core, bool only_valid, int print_min_size, bool update_valid ) {
	n_tmp_node = 0;
	m_tmp_node = 0;

	if( only_valid && !valid[s] )
		return -1;

	if( core[s] < min_core )
		return -1;

	//if( timestamp[s] == now_time )
	//	return -1;

	int top = 0;
	int tail = 1;
	timestamp[s] = now_time;
	tmp_node[top] = s;

	while( top < tail ) {
		int u = tmp_node[top++];
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			if( core[v] < min_core )
				continue;
			if( only_valid && !valid[v] )
				continue;

			++m_tmp_node;

			if( timestamp[v] == now_time )
				continue;
			tmp_node[tail++] = v;
			timestamp[v] = now_time;
		}
	}

	n_tmp_node = tail;
	m_tmp_node /= 2;

	int cnt_c = 0;
	if( update_valid ) {
		for( int i = 0; i < tail; ++i ) {
			int u = tmp_node[i];
			if( valid[u] && (lint) lrho_n[u] * (lint) (n_tmp_node-1) < (lint) lrho_m[u] * 2 ) {
				++cnt_c;
				valid[u] = false;
			}
		}
	}

	return cnt_c;

	/*
	int cnt_c = 0;

	if( only_valid ) {
		for( int i = 0; i < tail; ++i ) {
			int u = now_node[i];
			if( valid[u] && lrho_n[u] * (n_now_node-1) < lrho_m[u] * 2 ) {
				++cnt_c;
				valid[u] = false;
			}
		}
	}
	*/

	/*
	int now_min_core = n;
	int now_max_core = 0;


	double mind = n;
	double maxd = 0;

	for( int i = 0; i < tail; ++i ) {
		int u = now_node[i];
		now_min_core = min( urho[u], now_min_core );
		now_max_core = max( urho[u], now_max_core );
		mind = min( 1.0*lrho_m[u]/lrho_n[u], mind );
		maxd = max( 1.0*lrho_m[u]/lrho_n[u], maxd );
	}


	if( print_min_size == -1 )
		return;

	if( n_now_node >= print_min_size )
		printf( "n=%d,d=%0.2lf,m=%d,nc=%d,minc=%d,maxc=%d,mind=%0.2lf,maxd=%0.3lf\n",
			n_now_node,  1.0*m_now_node/n_now_node, m_now_node,
			cnt_c, now_min_core, now_max_core, mind, maxd );
	*/
}


int Graph::compute_invalid( int u ) {
	++now_time;
	bfs( u, -1, true );
	save_tmp_node();

	int n_iter = 0;
	for( int i = 1; i <= MAX_ITER; ++i ) {
		++n_iter;
		int cnt_change = compute_core_by_valid();
		++now_time;
		int cnt_change2 = 0;
		for( int j = 0; j < n_now_node; ++j ) {
			int u = now_node[j];
			if( valid[u] && timestamp[u] != now_time )
				cnt_change2 += bfs( u, -1, true, -1, true );
		}

		//if( cnt_change > 1000 )
		//	printf( "[%d]:cnt_change=%d\n", i, cnt_change );
		if( cnt_change + cnt_change2 == 0 )
			break;
	}
	//if( n_iter > 30 )
	//	printf( "n_iter:%d\n", n_iter );
	return n_iter;
}


/*
void Graph::prune_by_core() {
	printf( "pruning by core...\n" );
	lrho_m = new lint[n];
	lrho_n = new lint[n];
	int* fa = new int[n];
	int* f = new int[n];
	int* stk = new int[n];

	memset( fa, -1, sizeof(int) * n );
	for( int i = 0; i < n; ++i )
		f[i] = i;

	printf( "building tree...\n" );
	for( int i = n-1; i >= 0; --i ) {
		int u = del_list[i];
		lrho_n[u] = 1;
		lrho_m[u] = 0;
		fa[u] = u;

		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( fa[v] == -1 )
				continue;
			int nowf = get_f( v, f );
			if( nowf != u ) {
				f[nowf] = u;
				fa[nowf] = u;
				lrho_n[u] += lrho_n[nowf];
				lrho_m[u] += lrho_m[nowf] + 1;
			} else
				++lrho_m[u];
		}
	}

	memset( f, -1, sizeof(int) * n );

	printf( "updating lrho...\n" );
	for( int u = 0; u < n; ++u ) {
		int top = 0;
		int nowu = u;
		while( true ) {
			stk[top++] = nowu;
			if( f[nowu] != -1 || fa[nowu] == nowu )
				break;
			nowu = fa[nowu];
		}

		lint best_n = lrho_n[nowu];
		lint best_m = lrho_m[nowu];

		for( int j = top-2; j >= 0; --j ) {
			nowu = stk[j];
			f[nowu] = 0;
			if( better(lrho_n[nowu], lrho_m[nowu], best_n, best_m) ) {
				best_n = lrho_n[nowu];
				best_m = lrho_m[nowu];
			}
			lrho_n[nowu] = best_n;
			lrho_m[nowu] = best_m;
		}
	}

	printf( "computing valid nodes...\n" );
	valid = new bool[n];
	memset(valid, 0, sizeof(bool) * n);

	int cnt_valid = 0;
	int min_core = n;

	for( int u = 0; u < n; ++u )
		if ( lrho_n[u] * core[u] >= lrho_m[u] ) {
			valid[u] = true;
			++cnt_valid;
			if( core[u] < min_core )
				min_core = core[u];
		}

	printf( "cnt_valid=%d, min_core=%d\n", cnt_valid, min_core );

	delete[] fa;
	delete[] f;
	delete[] stk;

	int n_comp = 0;

	//++now_time;
	//for( int u = 0; u < n; ++u )
	//	if( timestamp[u] != now_time) {
	//		++n_comp;
	//		printf( "%d:", n_comp );
	//		bfs( u, -1, false );
	//}
	//return;


	++now_time;
	for( int u = 0; u < n; ++u )
		bfs( u, -1, true, false );

	++now_time;
	bfs( 0, -1, false );

	n_comp = 0;
	++now_time;
	for( int u = 0; u < n; ++u )
		if( timestamp[u] == now_time-1 && valid[u]) {
			++n_comp;
			printf( "%d:", n_comp );
			bfs( u, -1, true );
		}
}
*/

/*
void Graph::dfs_invalid( int u ) {
	valid[u] = false;
	for( int i = 0; i < len[u]; ++i ) {
		int v = con[u][i];
		if( valid[v] ) {
			--tmp_info[v];
			if( tmp_info[v] * lrho_n[v] < lrho_m[v] )
				dfs_invalid( v );
		}
	}
}
*/

/*
void Graph::bfs( int s, int min_core, bool only_valid, bool print_info ) {
	if( only_valid && !valid[s] )
		return;
	if( core[s] < min_core )
		return;
	if( timestamp[s] == now_time )
		return;

	int top = 0;
	int tail = 1;
	timestamp[s] = now_time;
	now_node[top] = s;
	m_now_node = 0;
	tmp_info[s] = 0;

	while( top < tail ) {
		int u = now_node[top++];
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			if( core[v] < min_core )
				continue;
			if( only_valid && !valid[v] )
				continue;

			++tmp_info[u];
			++m_now_node;

			if( timestamp[v] == now_time )
				continue;
			now_node[tail++] = v;
			timestamp[v] = now_time;
			tmp_info[v] = 0;
		}
	}

	n_now_node = tail;
	m_now_node /= 2;

	int now_min_core = n;
	int now_max_core = 0;
	int cnt_c = 0;
	int cnt_d = 0;
	int cnt_e = 0;

	if( only_valid ) {

		for( int p = 0; p < 5; ++p ) {
			cnt_c = 0;
			cnt_d = 0;
			cnt_e = 0;
			for( int i = 0; i < tail; ++i ) {
				int u = now_node[i];
				if( valid[u] && better(n_now_node, m_now_node, lrho_n[u], lrho_m[u] ) ) {
					++cnt_e;
					lrho_n[u] = n_now_node;
					lrho_m[u] = m_now_node;
				}
			}

			for( int i = 0; i < tail; ++i ) {
				int u = now_node[i];
				if( valid[u] && tmp_info[u] * lrho_n[u] < lrho_m[u] ) {
					++cnt_d;
					dfs_invalid(u);
				}
			}

			for( int i = 0; i < tail; ++i ) {
				int u = now_node[i];
				if( valid[u] && lrho_n[u] * (n_now_node-1) < lrho_m[u] * 2 ) {
					++cnt_c;
					valid[u] = false;
				}
			}

			if( cnt_c == 0 && cnt_d == 0 && cnt_e == 0 )
				break;
		}
	}

	double mind = n;
	double maxd = 0;

	for( int i = 0; i < tail; ++i ) {
		int u = now_node[i];
		now_min_core = min( core[u], now_min_core );
		now_max_core = max( core[u], now_max_core );
		mind = min( 1.0*lrho_m[u]/lrho_n[u], mind );
		maxd = max( 1.0*lrho_m[u]/lrho_n[u], maxd );
	}


	if( !print_info )
		return;

	printf( "n=%d,d=%0.2lf,m=%d,s=%d,nc=%d,nd=%d,ne=%d,minc=%d,maxc=%d,mind=%0.2lf,maxd=%0.3lf\n",
		n_now_node,  1.0*m_now_node/n_now_node, m_now_node,
		s, cnt_c, cnt_d, cnt_e, now_min_core, now_max_core, mind, maxd );
}
*/

void Graph::ready(string path, int min_rho) {
	init( path );
	if( min_rho > 0 )
		set_min_rho( min_rho );
	compute_core();
	prune_by_core();
	init_heap();
}

int Graph::next() {
	while( !heap->empty() ) {
		Entry *e = heap->deheap();
		printf( "\n%s:n=%d,m=%d,density=%0.3lf\n", e->is_exact?"Exact":"InExact", e->rho_n,
				e->rho_m, e->rho_m*1.0/e->rho_n );
		max_rho_n = e->rho_n;
		max_rho_m = e->rho_m;
		if( e->is_exact ) {

			if( is_lds(e->v) ) {
				for( int i = 0; i < n_now_node; ++i )
					in_lds[now_node[i]] = true;
				return e->v;
			} else {
				for( int i = 0; i < n_now_node; ++i )
					valid[now_node[i]] = false;
			}
		} else
			compute_densest( e->v );
	}

	n_now_node = 0;
	m_now_node = 0;
	return -1;
}

void Graph::test() {
	/*
	for( int i = 0; i < 20; ++i ){
		printf( "\nIteration %d:\n", i+1 );
		Entry * e = heap->deheap();
		if( e == NULL )
			break;
		if( !e->is_exact ) {
			printf( "udensity=%0.3lf\n", e->rho_m * 1.0 / e->rho_n );
			compute_densest( e->v );
		}
		else
			printf( "Exact:n=%d,m=%d,density=%0.3lf\n", e->rho_n, e->rho_m, e->rho_m * 1.0 / e->rho_n );
	}
	*/
	for( int i = 1; i <= 10; ++i ) {
		printf( "\n======Top %d======\n", i );
		next();
	}
}

void Graph::lds_topk( int k ) {
	for( int i = 1; i <= k; ++i ) {
		printf( "\n======Top %d======\n", i );
		next();
		output_result_factor( i );
	}
}

bool Graph::is_lds_in_core( int v ) {
	int max_core = (m_now_node+n_now_node-1)/n_now_node;

	++now_time;
	bfs( v, max_core, false );
	printf( "max_core=%d,n_core=%d,m_core=%d\n", max_core, n_tmp_node, m_tmp_node );

	bool is_max_in_core = true;

	for( int i = 0; i < n_tmp_node; ++i ) {
		int u = tmp_node[i];
		if( in_lds[u] ) {
			is_max_in_core = false;
			break;
		}
	}

	if( is_max_in_core ) {
		printf( "True: is max in core\n" );
		return true;
	}

	Subgraph *s = new Subgraph();
	s->init( this );
	s->find_rho_compact( n_now_node, m_now_node );

	++now_time;
	for( int i = 0; i < s->n_now_node; ++i ) {
		int u = s->org_id[s->now_node[i]];
		timestamp[u] = now_time;
	}

	++now_time;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( timestamp[u] != now_time - 1 ) {
			printf( "Error, u=%d\n", u );
			return false;
		}
		timestamp[u] = now_time;
	}

	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( timestamp[v] == now_time - 1 ) {
				printf( "False!\n" );
				return false;
			}
		}
	}

	printf( "True!\n" );
	return true;
}

bool Graph::is_lds( int v ) {
	printf( "Is LDS:\n" );

	++now_time;
	bfs( v, -1, true );
	save_tmp_node();

	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( better( lrho_n[u], lrho_m[u], n_now_node, m_now_node ) ) {
			printf( "False:lower bound\n" );
			return false;
		}
		if( better( n_now_node, m_now_node, urho_n[u], urho_m[u] ) ) {
			printf( "False:upper bound\n" );
			return false;
		}
	}

	return is_lds_in_core( v );
}

void Graph::compute_udensity( int &u_m, int &u_n ) {
	++now_time;
	int now_maxd = 0;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		bin_pool[u].clear();
		now_maxd = max(len[u], now_maxd);
	}

	memset( bin, 0, sizeof(BinNode) * (now_maxd+1) );

	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		bin_pool[u].id = u;
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( valid[v] )
				++bin_pool[u].deg;
		}
		update_binnode( bin_pool+u );
	}

	int now_m = u_m;
	int now_n = u_n;

	for( int i = 0; i <= now_maxd; ++i ) {
		BinNode *bn = bin+i;
		while( bn->next != NULL )
			bn = bn->next;

		while( bn != bin+i ) {
			int u = bn->id;
			--now_n;
			timestamp[u] = now_time;
			tmp_info[u] = i;
			for( int j = 0; j < len[u]; ++j ) {
				int v = con[u][j];
				if( valid[v] && bin_pool[v].deg > i ) {
					--bin_pool[v].deg;
					update_binnode( bin_pool + v );
				}
				if( timestamp[v] == now_time - 1 )
					--now_m;
			}

			if( better(now_n, now_m, u_n, u_m) ) {
				u_n = now_n;
				u_m = now_m;
			}

			bn = bn->prev;
		}
	}

	if( now_m != 0 || now_n != 0 )
		printf( "Error:now_m=%d,now_n=%d\n", now_m, now_n );

	int n_valid_densest = 0;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		valid_densest[u] = (tmp_info[u] * (lint) u_n >= (lint) u_m);
		if( valid_densest[u] )
			++n_valid_densest;
	}

	printf( "u_m=%d,u_n=%d,density=%0.3lf\n", u_m, u_n, u_m*1.0/u_n );
	printf( "n_valid_densest=%d\n", n_valid_densest );
}

void Graph::compute_densest( int v ) {
	++now_time;
	bfs( v, -1, true );
	printf( "compute_densest:n=%d,m=%d,now_time=%d\n", n_tmp_node, m_tmp_node, now_time );
	if( m_tmp_node == 0 )
		return;
	save_tmp_node();
	int u_m = m_now_node;
	int u_n = n_now_node;
	compute_udensity( u_m, u_n );

	++now_time;
	int cur_time = now_time;

	Subgraph *best = NULL;

	int n_comp = 0;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( valid_densest[u] && timestamp[u] < cur_time ) {
			bfs_densest( u );
			if( better( u_n, u_m, 2, n_tmp_node ) )
				continue;
			if( best != NULL )
				if( better( best->n_now_node, best->m_now_node, 2, n_tmp_node ) )
					continue;

			++n_comp;
			printf( "comp %d:n=%d,m=%d,density=%0.3lf\n", n_comp, n_tmp_node,
					m_tmp_node, m_tmp_node*1.0/n_tmp_node );

			Subgraph *s = new Subgraph();
			s->init( this );
			s->find_densest(max_rho_n, max_rho_m);
			printf( "densest:n=%d,m=%d,density=%0.3lf\n", s->n_now_node, s->m_now_node,
					s->m_now_node*1.0/s->n_now_node );
			if( best == NULL )
				best = s;
			else if( better( s->n_now_node, s->m_now_node, best->n_now_node, best->m_now_node ) ) {
				delete best;
				best = s;
			}
		}
	}

	if( best != NULL ) {
		printf( "Best:n=%d,m=%d,density=%0.3lf\n", best->n_now_node, best->m_now_node,
				best->m_now_node * 1.0 / best->n_now_node );

		bool now_valid = true;
		++now_time;
		for( int i = 0; i < best->n_now_node; ++i ) {
			int u = best->org_id[best->now_node[i]];
			timestamp[u] = now_time;
			if( better( lrho_n[u], lrho_m[u], best->n_now_node, best->m_now_node ) )
				now_valid = false;
		}

		for( int i = 0; i < n_now_node; ++i ) {
			int u = now_node[i];
			if( timestamp[u] != now_time )
				update_u( u, best->n_now_node, best->m_now_node, false );
		}

		printf( "computing invalid...\n" );
		int n_iter = compute_invalid( v );
		printf( "compute invalid: iter = %d\n", n_iter );

		++now_time;
		int u = best->org_id[best->now_node[0]];
		bfs( u, -1, true );
		if( !now_valid || n_tmp_node != best->n_now_node ) {
			printf( "Best is Invalid:n_tmp_node=%d,best_node=%d\n", n_tmp_node, best->n_now_node );
			for( int i = 0; i < best->n_now_node; ++i ) {
				int u = best->org_id[best->now_node[i]];
				valid[u] = false;
			}
		} else
			heap->enheap( u, best->n_now_node, best->m_now_node, true );

		int n_comp = 0;
		int n_invalid = 0;

		for( int i = 0; i < n_now_node; ++i ) {
			int u = now_node[i];
			if( valid[u] && timestamp[u] != now_time ) {
				++n_comp;
				add_to_heap( u );
			}
			if( !valid[u] )
				++n_invalid;
		}
		printf( "n_new_comp=%d,n_new_invalid=%d\n", n_comp, n_invalid );

		delete best;
	}
}

void Graph::add_to_heap( int u ) {
	bfs( u, -1, true );

	int max_rho_n = urho_n[u];
	int max_rho_m = urho_m[u];
	for( int i = 0; i < n_tmp_node; ++i ) {
		int v = tmp_node[i];
		if( better( urho_n[v], urho_m[v], max_rho_n, max_rho_m ) ) {
			max_rho_n = urho_n[v];
			max_rho_m = urho_m[v];
		}
	}
	heap->enheap( u, max_rho_n, max_rho_m, false );
}

void Graph::bfs_now_time( int v ) {
	int pre_time = now_time;
	++now_time;
	int top = 0;
	int tail = 1;
	timestamp[v] = now_time;
	tmp_node[top] = v;
	m_tmp_node = 0;

	while( top < tail ) {
		int u = tmp_node[top++];
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];

			if( timestamp[v] < pre_time )
				continue;

			++m_tmp_node;

			if( timestamp[v] == now_time )
				continue;
			tmp_node[tail++] = v;
			timestamp[v] = now_time;
		}
	}

	n_tmp_node = tail;
	m_tmp_node /= 2;
}

void Graph::bfs_densest( int v ) {
	++now_time;
	int top = 0;
	int tail = 1;
	timestamp[v] = now_time;
	tmp_node[top] = v;
	m_tmp_node = 0;

	while( top < tail ) {
		int u = tmp_node[top++];
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];

			if( !valid[v] || !valid_densest[v] )
				continue;

			++m_tmp_node;

			if( timestamp[v] == now_time )
				continue;
			tmp_node[tail++] = v;
			timestamp[v] = now_time;
		}
	}

	n_tmp_node = tail;
	m_tmp_node /= 2;
}

ResultGraph* Graph::create_result_graph() {
	ResultGraph *rg = new ResultGraph();
	rg->n = n_now_node;
	rg->m = m_now_node;
	rg->con = new vector<int>[n_now_node];
	rg->org_id = new int[n_now_node];
	rg->len = new int[n_now_node];
	memset( rg->len, 0, sizeof(int) * n_now_node );

	++now_time;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		tmp_info[u] = i;
		timestamp[u] = now_time;
		rg->org_id[i] = u;
	}

	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		int pu = tmp_info[u];
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			int pv = tmp_info[v];
			if( timestamp[v] == now_time ) {
				++rg->len[pu];
				rg->con[pu].push_back( pv );
			}
		}
	}
	rg->g = this;

	return rg;
}

void Graph::set_output_factor( bool output_factor ) {
	this->output_factor = output_factor;
}

void Graph::output_result_factor( int topk ) {
	if( fout && !output_factor ) {
		fprintf( fout, "%d %0.5lf\n",  topk, (clock() - start_clock) / CLOCK_SECOND );
		fflush( fout );
		return;
	}

	if( !output_factor )
		return;

	if( n_now_node == 0 )
		return;
	ResultGraph *rg = create_result_graph();
	rg->compute_all_factor();

	if( fout ) {
		fprintf( fout, "======\n" );
		fprintf( fout, "topk=%d\n", topk );
		fprintf( fout, "n=%d\n", rg->n );
		fprintf( fout, "m=%d\n", rg->m );
		fprintf( fout, "density=%0.5lf\n", rg->density );
		fprintf( fout, "edge_density=%0.5lf\n", rg->edge_density );
		fprintf( fout, "triangle_density=%0.5lf\n", rg->triangle_density );
		fprintf( fout, "cluster_coefficient=%0.5lf\n", rg->cluster_coefficient );
		fprintf( fout, "min_degree=%d\n", rg->min_degree );
		fprintf( fout, "connectivity=%d\n", rg->connectivity );
		fprintf( fout, "diameter=%d\n", rg->diameter );
		fprintf( fout, "avg_distance=%0.5lf\n", rg->avg_distance );
		fprintf( fout, "conductance=%0.5lf\n", rg->conductance );
		fprintf( fout, "relative_density=%0.5lf\n", rg->relative_density );
		fprintf( fout, "now_time=%0.5lf sec\n",  (clock() - start_clock) / CLOCK_SECOND );
		fflush( fout );

		if( node_name != NULL ) {
			fprintf( fout, "Graph:\n" );
			for( int i = 0; i < rg->n; ++i ) {
				for( int j = 0; j < rg->len[i]; ++j ) {
					int k = rg->con[i][j]; {
						int id_i = rg->org_id[i];
						int id_k = rg->org_id[k];
						if( id_k > id_i )
							fprintf( fout, "(%s,%s) ", node_name[id_i].c_str(), node_name[id_k].c_str() );
					}
				}
			}
			fprintf( fout, "\n" );
		}
	}

	if( node_name != NULL ) {
		/*
		printf( "\n======Graph======\n" );
		for( int i = 0; i < rg->n; ++i ) {
			int id = rg->org_id[i];
			printf( "%d:%s\n", i, node_name[id].c_str() );
		}

		for( int i = 0; i < rg->n; ++i ) {
			printf( "%d(%d):", i, rg->len[i] );
			for( int j = 0; j < rg->len[i]; ++j )
				printf( " %d", rg->con[i][j] );
			printf( "\n" );
		}

		printf( "\n=================\n" );
		*/
	}

	delete rg;
}

void Graph::fill_subgraph( Subgraph *s ) {
	s->init( n_tmp_node, m_tmp_node );
	int nowm = 0;

	++now_time;
	for( int i = 0; i < n_tmp_node; ++i ) {
		int u = tmp_node[i];
		tmp_info[u] = i+1;
		timestamp[u] = now_time;
		s->org_id[i+1] = u;
	}

	for( int i = 0; i < n_tmp_node; ++i ) {
		int u = tmp_node[i];
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( v > u && timestamp[v] == now_time ) {
				++nowm;
				s->px[nowm] = tmp_info[u];
				s->py[nowm] = tmp_info[v];

				++s->in[s->px[nowm]];
				++s->in[s->py[nowm]];
			}
		}
	}

	if( nowm != m_tmp_node )
		printf( "Error:nowm=%d,m_tmp_node=%d\n", nowm, m_tmp_node );
}


void Graph::init_greedy() {
	printf( "Init greedy..." );

	compute_core();

	if( valid == NULL )
		valid = new bool[n];
	for( int i = 0; i < n; ++i )
		valid[i] = true;

	if( bin == NULL )
		bin = new BinNode[maxd+1];
	if( bin_pool == NULL )
		bin_pool = new BinNode[n];

	if( valid_densest == NULL )
		valid_densest = new bool[n];
	if( in_lds == NULL )
		in_lds = new bool[n];

	memset( in_lds, 0, sizeof(bool) * n );
	memset( valid_densest, 0, sizeof(bool) * n );

	printf( "Done\n" );
}

void Graph::init_greedy_no_verify() {
	printf( "Init greedy no verify..." );
	if( valid == NULL )
		valid = new bool[n];
	for( int i = 0; i < n; ++i )
		valid[i] = true;

	if( bin == NULL )
		bin = new BinNode[maxd+1];
	if( bin_pool == NULL )
		bin_pool = new BinNode[n];

	if( valid_densest == NULL )
		valid_densest = new bool[n];
	if( in_lds == NULL )
		in_lds = new bool[n];

	memset( in_lds, 0, sizeof(bool) * n );
	memset( valid_densest, 0, sizeof(bool) * n );

	printf( "Done\n" );
}

void Graph::init_quasi_clique_local() {
	printf( "Init quasi clique local..." );
	if( valid == NULL )
		valid = new bool[n];
	for( int i = 0; i < n; ++i )
		valid[i] = true;
	nowd = new int[n];
	for( int i = 0; i < n; ++i )
		nowd[i] = len[i];

	used = new bool[n];

	/*
	hash = new EntryHash();
	for( int u = 0; u < n; ++u )
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			if( v > u )
				hash->add( u, v );
		}
	*/

	tcnt = new int[n];
	printf( "Done\n" );
}

bool Graph::greater( int u, int v ) {
	if( nowd[u] > nowd[v] )
		return true;
	if( nowd[u] < nowd[v] )
		return false;
	return u > v;
}

void Graph::count_triangle() {
	printf( "Counting Triangles...." );
	memset( tcnt, 0, sizeof(int) * n );
	double percent = 0;
	for( int u = 0; u < n; ++u ) {
		double now_percent = u * 1.0 / n;
		if( now_percent - percent > 0.05 ) {
			percent = now_percent;
			printf( "[%0.3lf]", percent );
		}

		++now_time;
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			timestamp[v] = now_time;
		}

		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			if( greater( v, u ) )
				continue;
			for( int j = 0; j < len[v]; ++j ) {
				int w = con[v][j];
				if( !greater( w, u ) || !greater( w, v ) )
					continue;
				//if( hash->find( w, u ) ) {
				if( timestamp[w] == now_time ) {
					++tcnt[u];
					++tcnt[v];
					++tcnt[w];
				}
			}
		}
	}
	printf( "Done\n" );
}

void Graph::remove_triangle_node( int u ) {
	if( !valid[u] )
		return;

	++now_time;
	for( int i = 0; i < len[u]; ++i ) {
		int v = con[u][i];
		if( valid[v] )
			timestamp[v] = now_time;
	}

	for( int i = 0; i < len[u]; ++i ) {
		int v = con[u][i];
		if( !valid[v] )
			continue;
		for( int j = 0; j < len[v]; ++j ) {
			int w = con[v][j];
			if( timestamp[w] != now_time || greater( w, v ) )
				continue;
			--tcnt[u];
			--tcnt[v];
			--tcnt[w];
		}
	}
	/*
	for( int i = 0; i < len[u]; ++i ) {
		int v = con[u][i];
		if( !valid[v] )
			continue;
		if( greater( v, u ) ) {
			for( int j = 0; j < len[u]; ++j ) {
				int w = con[u][j];
				if( !valid[w] || greater( w, v ) )
					continue;
				if( hash->find( w, v ) ) {
					--tcnt[u];
					--tcnt[v];
					--tcnt[w];
				}
			}
		} else {
			for( int j = 0; j < len[v]; ++j ) {
				int w = con[v][j];
				if( !valid[w] || greater( w, v ) )
					continue;
				if( hash->find( w, u ) ) {
					--tcnt[u];
					--tcnt[v];
					--tcnt[w];
				}
			}
		}
	}
	*/

	valid[u] = false;

	nowd[u] = 0;
	for( int i = 0; i < len[u]; ++i ) {
		int v = con[u][i];
		--nowd[v];
	}
}

void Graph::print_tcnt() {
	printf( "Tcnt:n=%d\n", n );
	for( int u = 0; u < n; ++u )
		if( valid[u] )
			printf( "(%d:%d:%d)\n", u, nowd[u], tcnt[u] );
}

void Graph::local_next() {
	printf( "Local next...\n" );
	n_now_node = 0;
	memset( used, 0, sizeof(bool) * n );

	int s = -1;
	for( int u = 0; u < n; ++u )
		if( valid[u] && nowd[u] > 0 ) {
			if( s == -1 )
				s = u;
			else if( tcnt[u] * nowd[s] > tcnt[s] * nowd[u] )
				s = u;
		}
	printf( "s=%d\n", s );

	if( s == -1 )
		return;

	used[s] = true;
	now_node[n_now_node++] = s;

	for( int i = 0; i < len[s]; ++i ) {
		int u = con[s][i];
		if( valid[u] ) {
			used[u] = true;
			now_node[n_now_node++] = u;
		}
	}

	int cnt = 0;
	for( int i = 0; i < TMAX; ++i ) {
		++cnt;
		while( local_add() );
		if( !local_del() )
			break;
	}

	printf( "iter_num=%d\n", cnt );

	m_now_node = 0;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( used[v] && v > u )
				++m_now_node;
		}
	}

	//for( int i = 0; i < n_now_node; ++i )
	//	printf( "[%d]", now_node[i] );
	//printf( "\n" );

	for( int j = 0; j < n_now_node; ++j ) {
		int u = now_node[j];
		remove_triangle_node(u);
	}

	printf( "n_node=%d,n_edge=%d,density=%0.3lf\n", n_now_node,
			m_now_node, m_now_node * 1.0 / n_now_node );
}

bool Graph::local_add() {
	int n_cand = 0;
	++now_time;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( !valid[v] || used[v] )
				continue;
			if( timestamp[v] == now_time )
				++tmp_info[v];
			else {
				timestamp[v] = now_time;
				tmp_info[v] = 1;
				tmp_node[n_cand++] = v;
			}
		}
	}

	bool updated = false;
	for( int i = 0; i < n_cand; ++i ) {
		int u = tmp_node[i];
		if( tmp_info[u] * 1.0 < alpha * n_now_node + (1e-7) )
			continue;
		now_node[n_now_node++] = u;
		used[u] = true;
		updated = true;
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( timestamp[v] == now_time && !used[v] )
				++tmp_info[v];
		}
	}

	return updated;
}

bool Graph::local_del() {
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		tmp_info[u] = 0;
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( valid[v] && used[v] )
				++tmp_info[u];
		}
	}

	int n_tmp_node = 0;
	int n_lft_node = n_now_node;
	bool updated = false;

	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( tmp_info[u] * 1.0 > alpha * (n_lft_node+1) - (1e-7) ) {
			now_node[n_tmp_node++] = u;
			continue;
		}

		used[u] = false;
		updated = true;
		--n_lft_node;
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( used[v] )
				--tmp_info[v];
		}
	}

	n_now_node = n_tmp_node;

	return updated;
}

void Graph::local_topk( int k ) {
	init_quasi_clique_local();
	count_triangle();
	for( int i = 0; i < k; ++i ) {
		printf( "======top %d======\n", i+1 );
		local_next();
		output_result_factor( i + 1 );
	}
}

double Graph::f_density( int n, int m ) {
	if( n == 0 )
		return 0;
	return 1.0*m/n;
}

void Graph::init_approx() {
	printf( "Init approx..." );
	if( valid == NULL )
		valid = new bool[n];
	for( int i = 0; i < n; ++i )
		valid[i] = true;

	if( bin == NULL )
		bin = new BinNode[maxd+1];
	if( bin_pool == NULL )
		bin_pool = new BinNode[n];

	printf( "Done\n" );
}


void Graph::approx_topk( int k ) {
	init_approx();
	for( int i = 0; i < k; ++i ) {
		printf( "======top %d======\n", i+1 );
		approx_next();
		output_result_factor( i + 1 );
	}
}

void Graph::approx_next() {

	memset( bin, 0, sizeof(BinNode) * (maxd+1) );

	int now_m = 0;
	int now_n = 0;
	++now_time;

	for( int u = 0; u < n; ++u ) {
		if( !valid[u] )
			continue;
		bin_pool[u].clear();
		timestamp[u] = now_time;
		++now_n;
		bin_pool[u].id = u;
		for( int j = 0; j < len[u]; ++j ) {
			int v = con[u][j];
			if( valid[v] ) {
				++bin_pool[u].deg;
				if( v > u )
					++now_m;
			}
		}
		update_binnode( bin_pool+u );
	}

	if( now_n == 0 )
		return;

	++now_time;
	n_now_node = 0;
	int bst_m = now_m;
	int bst_n = now_n;
	int pos = 0;

	printf( "now_m=%d,now_n=%d,f_approx=%0.3lf\n", now_m, now_n, f_density(now_n, now_m) );
	for( int i = 0; i <= maxd; ++i ) {
		BinNode *bn = bin+i;
		while( bn->next != NULL )
			bn = bn->next;

		while( bn != bin+i ) {
			int u = bn->id;
			--now_n;
			timestamp[u] = now_time;
			//tmp_info[u] = i;
			now_node[n_now_node++] = u;
			for( int j = 0; j < len[u]; ++j ) {
				int v = con[u][j];
				if( valid[v] && bin_pool[v].deg > i ) {
					--bin_pool[v].deg;
					update_binnode( bin_pool + v );
				}
				if( timestamp[v] == now_time - 1 )
					--now_m;
			}

			if( f_density(now_n, now_m) > f_density(bst_n, bst_m) ) {
				pos = n_now_node;
				bst_n = now_n;
				bst_m = now_m;
			}

			bn = bn->prev;
		}
	}

	if( now_m != 0 || now_n != 0 )
		printf( "Error:now_m=%d,now_n=%d\n", now_m, now_n );

	++now_time;
	for( int i = pos; i < n_now_node; ++i ) {
		int u = now_node[i];
		timestamp[u] = now_time;
	}

	bfs_now_time( now_node[n_now_node-1] );

	for( int i = 0; i < n_tmp_node; ++i ) {
		int u = tmp_node[i];
		valid[u] = false;
	}


	printf( "n_tmp_node=%d,m_tmp_node=%d,density=%0.3lf\n", n_tmp_node, m_tmp_node,
			1.0*m_tmp_node/n_tmp_node );
	save_tmp_node();
}


int Graph::compute_densest_greedy() {
	++now_time;
	n_now_node = 0;
	m_now_node = 0;
	for( int u = 0; u < n; ++u )
		if( valid[u] ) {
			now_node[n_now_node++] = u;
			timestamp[u] = now_time;
			for( int i = 0; i < len[u]; ++i ) {
				int v = con[u][i];
				if( valid[v] && v > u )
					++m_now_node;
			}
		}
	if( n_now_node == 0 )
		return -1;

	printf( "compute_densest:n=%d,m=%d,now_time=%d\n", n_now_node, m_now_node, now_time );
	int u_m = m_now_node;
	int u_n = n_now_node;
	compute_udensity( u_m, u_n );

	++now_time;
	int cur_time = now_time;

	Subgraph *best = NULL;

	int n_comp = 0;
	for( int i = 0; i < n_now_node; ++i ) {
		int u = now_node[i];
		if( valid_densest[u] && timestamp[u] < cur_time ) {
			bfs_densest( u );
			if( better( u_n, u_m, 2, n_tmp_node ) )
				continue;
			if( best != NULL )
				if( better( best->n_now_node, best->m_now_node, 2, n_tmp_node ) )
					continue;

			++n_comp;
			printf( "comp %d:n=%d,m=%d,density=%0.3lf\n", n_comp, n_tmp_node,
					m_tmp_node, m_tmp_node*1.0/n_tmp_node );

			Subgraph *s = new Subgraph();
			s->init( this );
			s->find_densest(1, n_tmp_node);
			printf( "densest:n=%d,m=%d,density=%0.3lf\n", s->n_now_node, s->m_now_node,
					s->m_now_node*1.0/s->n_now_node );
			if( best == NULL )
				best = s;
			else if( better( s->n_now_node, s->m_now_node, best->n_now_node, best->m_now_node ) ) {
				delete best;
				best = s;
			}
		}
	}

	if( best != NULL ) {
		printf( "Best:n=%d,m=%d,density=%0.3lf\n", best->n_now_node, best->m_now_node,
				best->m_now_node * 1.0 / best->n_now_node );

		++now_time;
		n_tmp_node = 0;
		m_tmp_node = best->m_now_node;

		for( int i = 0; i < best->n_now_node; ++i ) {
			int u = best->org_id[best->now_node[i]];
			timestamp[u] = now_time;
			valid[u] = false;
			tmp_node[n_tmp_node++] = u;
		}

		int v = best->org_id[best->now_node[0]];
		delete best;

		save_tmp_node();
		return v;
	}
	return -1;
}

void Graph::next_greedy() {
	int u = -1;
	while( (u = compute_densest_greedy()) != -1 ) {
		if( is_lds_in_core( u ) ) {
			for( int i = 0; i < n_now_node; ++i )
				in_lds[now_node[i]] = true;
			break;
		}
	}
}

void Graph::next_greedy_no_verify() {
	compute_densest_greedy();
}

void Graph::greedy_topk( int k ) {
	init_greedy();

	for( int i = 1; i <= k; ++i ) {
		printf( "\n======Top %d======\n", i );
		next_greedy();
		output_result_factor( i );
	}
}

void Graph::greedy_no_verify_topk( int k ) {
	init_greedy_no_verify();

	for( int i = 1; i <= k; ++i ) {
		printf( "\n======Top %d======\n", i );
		next_greedy_no_verify();
		output_result_factor( i );
	}
}

void Graph::show_shortest_path( int s, int t ) {
	printf( "==========" );
	++now_time;
	int top = 0;
	int tail = 1;
	timestamp[s] = now_time;
	tmp_node[top] = s;
	now_node[top] = -1;
	tmp_info[top] = 0;

	while( top < tail ) {
		int u = tmp_node[top];
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];

			if( timestamp[v] == now_time )
				continue;
			tmp_node[tail] = v;
			now_node[tail] = top;
			tmp_info[tail] = tmp_info[top] + 1;
			timestamp[v] = now_time;
			if( v == t ) {
				printf( "Dis=%d\n", tmp_info[tail] );
				int pos = tail;
				while( pos != -1 ) {
					int id = tmp_node[pos];
					printf( "%d:%s\n", id, node_name[id].c_str() );
					pos = now_node[pos];
				}
				return;
			}
			++tail;
		}
		++top;
	}
	printf( "No Answer!\n" );
}

//======Entry======
Entry::Entry() {
	v = -1;
	rho_m = 0;
	rho_n = 0;
	is_exact = false;
}

//======EntryHeap======
EntryHeap::EntryHeap() {
	v_block.push_back( new Entry[n_block_entry] );
	v_pos.push_back( 0 );
	tot = 0;
	heap.push_back( NULL );
}

EntryHeap::~EntryHeap() {
	for( int i = 0; i < (int)v_block.size(); ++i )
		delete[] v_block[i];
}

Entry* EntryHeap::get_entry() {
	int t = (int) v_block.size() - 1;
	if( v_pos[t] == n_block_entry ) {
		v_block.push_back( new Entry[n_block_entry] );
		v_pos.push_back( 0 );
		++t;
	}

	return v_block[t] + (v_pos[t]++);
}

void EntryHeap::up( int p ) {
	Entry* tmp = heap[p];
	for( int i = p >> 1; i >= 1; i >>= 1 ) {
		if( Graph::better( heap[i]->rho_n, heap[i]->rho_m, tmp->rho_n, tmp->rho_m ) )
		  break;
		heap[p] = heap[i];
		p = i;
	}

	heap[p] = tmp;
}

void EntryHeap::down( int p ) {
	Entry* tmp = heap[p];
	for( int i = p << 1; i <= tot; i <<= 1 ) {
		if( i + 1 <= tot && Graph::better( heap[i+1]->rho_n, heap[i+1]->rho_m, heap[i]->rho_n, heap[i]->rho_m ) )
		  ++i;
		if( Graph::better( tmp->rho_n, tmp->rho_m, heap[i]->rho_n, heap[i]->rho_m ) )
		  break;
		heap[p] = heap[i];
		p = i;
	}
	heap[p] = tmp;
}


void EntryHeap::enheap( int v, int rho_n, int rho_m, bool is_exact ) {
	Entry *e = get_entry();
	e->v = v;
	e->rho_n = rho_n;
	e->rho_m = rho_m;
	e->is_exact = is_exact;

	++tot;
	if( tot == heap.size() )
		heap.push_back( e );
	else
		heap[tot] = e;
	up(tot);
}

Entry* EntryHeap::deheap() {
	if( tot == 0 )
		return NULL;
	Entry* p = heap[1];
	heap[1] = heap[tot--];
	if( tot > 0 )
		down(1);
	return p;
}

bool EntryHeap::empty() {
	return tot == 0;
}

//======Subgraph======
Subgraph::Subgraph() {
	n = 0;
	m = 0;
	tot = 0;
	s = 0;
	t = 0;

	px = NULL;
	py = NULL;
	in = NULL;
	a = NULL;
	next = NULL;
	c = NULL;
	start = NULL;
	level = NULL;
	now = NULL;

	n_now_node = 0;
	m_now_node = 0;
	n_dst_node = 0;
	m_dst_node = 0;

	now_node = NULL;
	dst_node = NULL;
	used = NULL;

	org_id = NULL;
	g = NULL;
}

Subgraph::~Subgraph() {
	clear();
}

void Subgraph::clear() {
	if( px != NULL ) {
		delete[] px;
		px = NULL;
	}

	if( py != NULL ) {
		delete[] py;
		py = NULL;
	}

	if( in != NULL ) {
		delete[] in;
		in = NULL;
	}

	if( a != NULL ) {
		delete[] a;
		a = NULL;
	}

	if( next != NULL ) {
		delete[] next;
		next = NULL;
	}

	if( c != NULL ) {
		delete[] c;
		c = NULL;
	}

	if( start != NULL) {
		delete[] start;
		start = NULL;
	}

	if( level != NULL ) {
		delete[] level;
		level = NULL;
	}

	if( now != NULL ) {
		delete[] now;
		now = NULL;
	}

	if( now_node != NULL ) {
		delete[] now_node;
		now_node = NULL;
	}

	if( used != NULL ) {
		delete[] used;
		used = NULL;
	}

	if( org_id != NULL ) {
		delete[] org_id;
		org_id = NULL;
	}
}

void Subgraph::init( int nown, int nowm ) {
	n = nown;
	m = nowm;
	s = 0;
	t = n+1;

	px = new int[m+1];
	py = new int[m+1];
	in = new int[n+1];
	memset( in, 0, sizeof(int) * (n+1) );

	a = new int[2*m+4*n+2];
	next = new int[2*m+4*n+2];
	c = new lint[2*m+4*n+2];

	start = new int[n+2];
	level = new int[n+2];
	now = new int[n+2];
	now_node = new int[n+3];
	dst_node = new int[n+3];
	used = new bool[n+2];
	org_id = new int[n+2];

	memset( org_id, -1, sizeof(int) * (n+2) );
}

void Subgraph::init( Graph *g ) {
	g->fill_subgraph( this );
	this->g = g;
}

void Subgraph::add(int x, int y, lint z, bool t) {
	a[++tot] = y;
	next[tot] = start[x];
	start[x] = tot;
	c[tot] = z;

	a[++tot] = x;
	next[tot] = start[y];
	start[y] = tot;
	c[tot] = t ? z: 0;
}

void Subgraph::make_map( lint rho ) {
	memset( start, 0, sizeof(int) * (n+2) );
	tot = 1;
	for( int i = 1; i <= m; ++i )
		add( px[i], py[i], (lint) n * (lint) n, true );

	for( int i = 1; i <= n; ++i ) {
		add( s, i, (lint) n * (lint) n * (lint) n, false );
		add( i, t, 2 * rho + (lint) (n-in[i]) * (lint) n * (lint) n, false );
	}
}

bool Subgraph::bfs() {
	int l=1,r=0;
	memset( level, 0, sizeof(int) * (n+2) );
	memcpy( now, start, sizeof(int) * (n+2) );

	now_node[++r] = s;
	level[s] = 1;

	while( l <= r ) {
		int u = now_node[l];
		for( int i = start[u]; i; i = next[i] ) {
			int v = a[i];
			if( c[i] && level[v] == 0 ) {
				level[v] = level[u] + 1;
				now_node[++r] = v;
				if( v == t )
					return true;
			}
		}
		++l;
	}

	return false;
}

void Subgraph::save_to_dst() {
	n_dst_node = n_now_node;
	m_dst_node = m_now_node;
	memcpy( dst_node, now_node+1, sizeof(int) * n_now_node );
}

lint Subgraph::dinic( int u, lint l ) {
	if( u == t )
		return l;
	lint pre_l = l;
	for( ; now[u]; now[u] = next[now[u]] ) {
		int i = now[u];
		int v = a[i];
		if( c[i] == 0 || level[v] != level[u] + 1 )
			continue;
		lint tmp = dinic( v, min(c[i], l) );
		c[i] -= tmp;
		c[i^1] += tmp;
		l -= tmp;
		if( l == 0 )
			break;
	}
	if( pre_l == l )
		level[u] = -1;
	return pre_l - l;
}

bool Subgraph::try_it( lint rho ) {
	make_map( rho );

	lint ans = 0;
	while( bfs() )
		ans += dinic( s, LINT_INF );

	for( int i = start[s]; i; i = next[i] )
		if( c[i] )
			return true;

	return false;
}

void Subgraph::bfs( int u, bool update_g ) {
	int l=0,r=-1;
	memset( used, 0, sizeof(bool) * (n+2) );

	now_node[++r] = s;
	used[s] = true;

	while( l <= r ) {
		int u = now_node[l++];
		for( int i = start[u]; i; i = next[i] )
			if( c[i] ) {
				int v = a[i];
				if( !used[v]) {
					used[v] = true;
					now_node[++r] = v;
				}
			}
	}

	n_now_node = r;

	m_now_node = 0;
	for( int i = 1; i <=m; ++i )
		if( used[px[i]] && used[py[i]] )
			++m_now_node;

	if( update_g ) {
		for( int i = 1; i <= n_now_node; ++i ) {
			int u = org_id[now_node[i]];
			g->update_l( u, n_now_node, m_now_node );
		}
	}
}

void Subgraph::bfs_one( int u ) {
	int l=0,r=-1;
	memset( now, 0, sizeof(int) * (n+2) );

	now_node[++r] = u;
	now[u] = 1;

	while( l <= r ) {
		int u = now_node[l++];
		for( int i = start[u]; i; i = next[i] ) {
			int v = a[i];
			if( now[v] == 0 && used[v] && v != s && v != t ) {
				now[v] = 1;
				now_node[++r] = v;
			}
		}
	}

	n_now_node = r+1;

	m_now_node = 0;
	for( int i = 1; i <=m; ++i )
		if( now[px[i]] == 1 && now[py[i]] == 1 )
			++m_now_node;
}

void Subgraph::find_densest(lint max_rho_n, lint max_rho_m) {
	lint l = max( (lint) n, ((lint) n * (lint) n) / 2);
	lint r = min( (lint) n * (lint) n * (lint) n, ((max_rho_m+max_rho_n)/max_rho_n) * (lint) n * (lint) n );
	lint ans = 0;
	while( l <= r ) {
		lint mid = (l+r) / 2;
		if( try_it( mid ) ) {
			bfs( s, true );
			ans = max(mid, ans);
			l = mid + 1;
		} else
			r = mid - 1;
	}

	try_it( ans );
	bfs( s, true );
	save_to_dst();
	bfs_one( now_node[1] );
}

void Subgraph::find_rho_compact( lint rho_n, lint rho_m ) {
	lint l = n;
	lint r = (lint) n * (lint) n * (lint) n;
	lint ans = 0;
	while( l <= r ) {
		lint mid = (l+r) / 2;
		if( mid * rho_n < rho_m * n * n ) {
			ans = max(mid, ans);
			l = mid + 1;
		} else
			r = mid - 1;
	}

	printf( "find_rho_compact:rho=%0.3lf(%lld/%lld),lrho=%0.3lf(%lld/%lld)\n",
			rho_m*1.0/rho_n, rho_m, rho_n,
			ans*1.0/((lint)n*(lint)n), ans, (lint) n * (lint) n );

	try_it( ans );
	bfs( s, true );
	save_to_dst();
	for( int i = 0; i < n_now_node; ++i )
		now_node[i] = now_node[i+1];
}

//======HEntry
HEntry::HEntry() {
	val = -1;
	next = NULL;
}

//======EntryHash
EntryHash::EntryHash() {
	v_block.push_back( new HEntry[n_block_entry] );
	v_pos.push_back( 0 );
	len_hash = init_hash_len;
	n_hash = 0;
	hash = new HEntry*[len_hash];
	memset( hash, 0, sizeof(HEntry*) * len_hash );
}

EntryHash::~EntryHash() {
	for( int i = 0; i < (int)v_block.size(); ++i )
		delete[] v_block[i];
	if( hash != NULL )
		delete[] hash;
}


HEntry* EntryHash::get_entry() {
	int t = (int) v_block.size() - 1;
	if( v_pos[t] == n_block_entry ) {
		v_block.push_back( new HEntry[n_block_entry] );
		v_pos.push_back( 0 );
		++t;
	}

	return v_block[t] + (v_pos[t]++);
}

lint EntryHash::get_val( int u, int v ) {
	return u < v ? u * (lint) (INT_INF-1) + v : v * (lint) (INT_INF-1) + u;
}

int EntryHash::h( lint val ) {
	return (int) (val % len_hash);
}

int EntryHash::h( int u, int v ) {
	return (int) (get_val( u, v ) % len_hash);
}

int EntryHash::h( HEntry* e ) {
	return (int) (e->val % len_hash);
}

void EntryHash::add( int u, int v ) {
	HEntry *e = get_entry();
	e->val = get_val(u, v);

	if( n_hash == len_hash ) {
		int old_len = len_hash;
		len_hash = len_hash * 2 + 1;
		HEntry **new_hash = new HEntry*[len_hash];
		memset( new_hash, 0, sizeof(HEntry*) * len_hash );
		for( int i = 0; i < old_len; ++i ) {
			for( HEntry *nowe = hash[i]; nowe != NULL; ) {
				HEntry *nexte = nowe->next;
				int nowh = h(nowe);
				nowe->next = new_hash[nowh];
				new_hash[nowh] = nowe;
				nowe = nexte;
			}
		}
		delete[] hash;
		hash = new_hash;
	}

	++n_hash;
	int nowh = h(e);
	e->next = hash[nowh];
	hash[nowh] = e;
}

HEntry* EntryHash::find( int u, int v ) {
	lint val = get_val( u, v );
	int nowh = h(val);
	for( HEntry *e = hash[nowh]; e != NULL; e = e->next )
		if( e->val == val )
			return e;
	return NULL;
}

//======ResultGraph======
ResultGraph::ResultGraph() {
	n = 0;
	m = 0;
	len = NULL;
	con = NULL;
	org_id = NULL;
	g = NULL;

	density = 0;
	edge_density = 0;
	triangle_density = 0;
	diameter = 0;
	connectivity = 0;
	min_degree = 0;
	conductance = 0;
	avg_distance = 0;
	cluster_coefficient = 0;
	relative_density = 0;

	adj = NULL;
	dis = NULL;
	p = NULL;
	used = NULL;

	timestamp = NULL;
	now_time = 0;
}

ResultGraph::~ResultGraph() {
	clear();
}

void ResultGraph::clear() {
	if( len != NULL ) {
		delete[] len;
		len = NULL;
	}

	if( con != NULL ) {
		delete[] con;
		con = NULL;
	}

	if( org_id != NULL ) {
		delete[] org_id;
		org_id = NULL;
	}

	if( adj != NULL ) {
		for( int i = 0; i < n; ++i )
			delete[] adj[i];
		delete[] adj;
		adj = NULL;
	}

	if( dis != NULL ) {
		delete[] dis;
		dis = NULL;
	}

	if( p != NULL ) {
		delete[] p;
		p = NULL;
	}

	if( used != NULL ) {
		delete[] used;
		used = NULL;
	}

	if( timestamp != NULL ) {
		delete[] timestamp;
		timestamp = NULL;
	}
}

void ResultGraph::compute_all_factor() {
	printf( "computing all factors...\n" );
	used = new bool[n];
	density = 1.0 * m / n;

	if( n >= 2 )
		edge_density = 2.0 * m / (n * 1.0 * (n-1) );

	timestamp = new int[n];
	memset( timestamp, 0, sizeof(int) * n );

	min_degree = n;
	for( int u = 0; u < n; ++u )
		min_degree = min(n, len[u]);

	//compute_connectivity();
	compute_triangle_factor();
	compute_cut_factor();
	compute_dis_factor();

	printf( "density=%0.3lf\n", density );
	printf( "edge_density=%0.3lf\n", edge_density );
	printf( "triangle_density=%0.3lf\n", triangle_density );
	printf( "cluster_coefficient=%0.3lf\n", cluster_coefficient );
	printf( "min_degree=%d\n", min_degree );
	printf( "connectivity=%d\n", connectivity );
	printf( "diameter=%d\n", diameter );
	printf( "avg_distance=%0.3lf\n", avg_distance );
	printf( "conductance=%0.3lf\n", conductance );
	printf( "relative_density=%0.3lf\n", relative_density );
}

void ResultGraph::compute_dis_factor() {
	printf( "computing dis factor..." );
	if( dis == NULL )
		dis = new int[n];
	if( used == NULL )
		used = new bool[n];
	if( p == NULL )
		p = new int[n];

	lint sum_dis = 0;
	diameter = 0;

	for( int u = 0; u < n; ++u ) {
		memset( used, 0, sizeof(bool) * n );
		int top = 0;
		int tail = 1;
		p[top] = u;
		dis[u] = 0;
		used[u] = true;

		while( top < tail ) {
			int v = p[top++];
			for( int i = 0; i < len[v]; ++i ) {
				int w = con[v][i];
				if( used[w] )
					continue;
				p[tail++] = w;
				used[w] = true;
				dis[w] = dis[v] + 1;
				diameter = max(diameter, dis[w]);
				sum_dis += dis[w];
			}
		}
		if( top != n ) {
			avg_distance = -1;
			diameter = -1;
			return;
		}
	}

	printf( "sum_dis=%lld\n", sum_dis );
	avg_distance = sum_dis * 1.0 / (n * 1.0 * (n-1));
	printf( "end\n" );
}


void ResultGraph::compute_cut_factor() {
	int n_cut_edge = 0;
	int sum_deg = 0;
	for( int i = 0; i < n; ++i ) {
		int u = org_id[i];
		n_cut_edge += g->len[u] - len[i];
		sum_deg += g->len[u];
	}
	int s = min( sum_deg, (int) g->m - sum_deg );
	if( s > 0 )
		conductance = 1.0 * n_cut_edge / (s * 1.0);
	if( n_cut_edge + m > 0 )
		relative_density = 1.0 * m / (n_cut_edge + m);
}

bool ResultGraph::greater( int u, int v ) {
	if( len[u] > len[v] )
		return true;
	if( len[u] < len[v] )
		return false;
	return u > v;
}

void ResultGraph::compute_triangle_factor() {
	lint n_wedge = 0;
	lint n_triangle = 0;

	for( int u = 0; u < n; ++u ) {
		++now_time;
		n_wedge += (len[u] * (len[u]-1) )/2;
		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			timestamp[v] = now_time;
		}

		for( int i = 0; i < len[u]; ++i ) {
			int v = con[u][i];
			if( greater( v, u ) )
				continue;
			for( int j = 0; j < len[v]; ++j ) {
				int w = con[v][j];
				if( !greater( w, u ) || !greater( w, v ) )
					continue;
				if( timestamp[w] == now_time )
					++n_triangle;
			}
		}
	}

	printf( "n_triangle=%lld\n", n_triangle );

	if( n >= 3 )
		triangle_density = n_triangle * 6.0 / (1.0 * n * (n-1) * (n-2));
	if( n_wedge > 0 )
		cluster_coefficient = n_triangle * 3.0 / n_wedge;
}

void ResultGraph::compute_connectivity() {
	printf( "computing connectivity...\n" );
	adj = new int*[n];
	for( int i = 0; i < n; ++i ) {
		adj[i] = new int[n];
		memset( adj[i], 0, sizeof(int) * n );
	}
	for( int u = 0; u < n; ++u )
		for( int i = 0; i < len[u]; ++i )
			adj[u][con[u][i]] = 1;

	p = new int[n];
	for( int i = 0; i < n; ++i )
		p[i] = i;

	dis = new int[n];

	int nown = n;
	connectivity = n;

	while( nown > 1 ) {
		if( nown % 100 == 0 )
			printf( "[%d/%d]", n-nown, n );
		int pre = 0;
		memset( used, 0, sizeof(bool) * n );
		memset( dis, 0, sizeof(int) * n );
		for( int i = 1; i < nown; ++i ) {
			int k = -1;
			for( int j = 1; j < nown; ++j )
				if( !used[p[j]] ) {
					dis[p[j]] += adj[p[pre]][p[j]];
					if( k == -1 || dis[p[k]] < dis[p[j]] )
						k = j;
				}
			used[p[k]] = true;

			if( i == nown - 1 ) {
				connectivity = min( connectivity, dis[p[k]] );
				for( int j = 0; j < nown; ++j ) {
					adj[p[pre]][p[j]] += adj[p[j]][p[k]];
					adj[p[j]][p[pre]] += adj[p[j]][p[k]];
				}
				p[k] = p[--nown];
			}
			pre = k;
		}
	}
	printf( "\nconnectivity = %d\n", connectivity );
}

