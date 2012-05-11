#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define RANGE 256
#define DRE2_INFINITE -1
#define DRE2_LEFT 0
#define DRE2_RIGHT 1
#define DRE2_GROUP_OPEN -1
#define DRE2_GROUP_CLOSE -2

// Character classes and predifined classes.
#define DRE2_CHAR_CLASS -3
#define DRE2_EOF -4
#define DRE2_ALPHA -5
#define DRE2_BORDER -6
#define DRE2_DIGIT -7
#define DRE2_SIM_DIGIT -8
#define DRE2_OBF_DIGIT -9
#define DRE2_HEX -10
#define DRE2_URL -11
#define DRE2_DOMAIN -12
#define DRE2_WORD -13
#define DRE2_WHITE_SPACE -14
#define DRE2_8BIT -15

// Matching methods.
#define DRE2_SN_SC 0
#define DRE2_SN_MC 1
#define DRE2_SN_SC_H 2
#define DRE2_SN_MC_H 3
#define DRE2_MN 4


#ifndef true
  #define true 1
#endif
#ifndef false
  #define false 0
#endif

struct
dre2_path {
  int *nodes;                    // Nodes in this path.
  int count;                     // Number of nodes within the path.
};

struct dre2_cost {
  int c_count;                   // Number of possible characters.
  int frequency;                 // Sum of the frequency of each possible character.
};

struct
dre2_fl_cost {
  int f_n_count;                 // Number of reachable nodes from the first node.
  int f_c_count;                 // Number of characters possible in the first reachable nodes.
  int f_frequency;               // Total frequency rating for first possible characters.
  int l_n_count;                 // Number of reachable nodes from the last node.
  int l_c_count;                 // Number of characters possible in the last reachable nodes.
  int l_frequency;               // Total frequency rating for the last possible characters.
};

struct
dre2_node {
  int c;                         // This node's data.
  int *n;                        // Neighbors of this node.
  int n_count;                   // Number of neighbors of this node.
  int *p;                        // Parents of this node.
  int p_count;                   // Number of parents of this node.
  int min_id;                    // New id assigned during the min process.
  int *possible;                 // Range of possible characters when using character classes.
  int *min_n;                    // Neighbors of this node in the minimal graph.
  int min_n_count;               // Number of neighbors in the minimal graph.
};

struct
dre2_range_return {
  int min;                       // Min size.
  int max;                       // Max size.
  int valid;                     // Whether or not the range is valid.
  int pos;                       // Position in the regex string where range terminates.
};

struct
dre2_parse_return {
  int open;                      // Opening node of the group.
  int close;                     // Closing node of the group.
  int pos;                       // Position in the regex.
};

struct
dre2 {
  struct dre2_node *v;           // Vertices within the graph.
  int count;                     // Number of nodes in the graph.
  int skip_table[RANGE];         // Horspool style skip table.
  int *state;                    // State table.
  int *reachable;                // Un-lookup table version of the state table.
  int *r_temp;                   // Temp reachable table.
  int min_length;                // Minimum string length that can possibly match this RE.
  int starting_point;            // Best starting point to reduce number of checks.
  int *starting_chars;           // Characters possible at our starting point.
  int single;                    // Whether or not the starting point is a single char.
  int match_method;              // Method that will be used to match this regex.
  unsigned char c;               // Single char to search for if there's a single starting char.
  int *starting_points;          // If all else fails, match from multiple nodes using best node in each path.
  int starting_count;            // Number of nodes for above.
};


// Function definitions.
int dre2_char_matches( struct dre2_node *node, unsigned char c );
unsigned char * dre2_matcher( struct dre2 *graph, unsigned char *begin_ptr, unsigned char *input, int start, int direction, int length );
int dre2_sn_sc_horspool( struct dre2 *graph, unsigned char *input, int length );
int dre2_sn_sc( struct dre2 *graph, unsigned char *input, int length );
int dre2_sn_mc_horspool( struct dre2 *graph, unsigned char *input, int length );
int dre2_sn_mc( struct dre2 *graph, unsigned char *input, int length );
int dre2_mn( struct dre2 *graph, unsigned char *input, int length );
int dre2_match( struct dre2 *graph, unsigned char *input );
unsigned char * dre2_escaped( unsigned char *re );
int dre2_binsearch( int *values, int min, int max, int key );
int dre2_largest( int *values, int length );
int dre2_contains_int( int *values, int length, int key );
int dre2_contains_char( unsigned char *string, unsigned char c );
void dre2_predefined_class( struct dre2_node *node, unsigned char *c, int action, int part_of_class );
int dre2_character_class( struct dre2_node *node, unsigned char *re, int s );
int string_to_int( unsigned char *s );
struct dre2_range_return dre2_range( unsigned char *re, int length, int pos );
void cleanup_nodes( struct dre2_node **v, int node_count );
void cleanup_dre2( struct dre2 *graph );
void dre2_find_paths_recursive( struct dre2 *graph, int id, int *path_count, struct dre2_path **paths );
int dre2_node_cost( struct dre2 *graph, int id );
int dre2_best_choice( struct dre2 *graph, int *required, int count );
void dre2_set_chars( struct dre2 *graph, int id );
void dre2_starting_chars( struct dre2 *graph, int *minimal );
int dre2_starting_point( struct dre2 *graph, int *minimal, int *minimal_id, int minimal_count );
void dre2_min_reachable( struct dre2 *graph, int **reachable, int **visited, int id );
void dre2_reachable( struct dre2 *graph, int **reachable, int **visited, int id );
void dre2_strip_groups( struct dre2 *graph, struct dre2 *new_graph, struct dre2_node **new_nodes, int *minimal, int **new_minimal, int *new_minimal_count, int **minimal_id );
struct dre2_cost dre2_single_cost( struct dre2 *graph, int id );
struct dre2_fl_cost dre2_first_or_last_cost( struct dre2 *graph, int *minimal );
int dre2_use_paths( struct dre2 *graph, int best, struct dre2_fl_cost *cost );
int dre2_first_or_last( struct dre2 *graph, struct dre2_fl_cost *cost );
void dre2_remove_minimal( int **minimal, int id );
void dre2_add_minimal( int **minimal, int *node_count );
void dre2_add_node( struct dre2_node **v, int *node_count, int c, int **minimal );
void dre2_add_neighbor( struct dre2_node **v, int origin, int dest );
void dre2_add_parents( struct dre2 *graph );
void dre2_skip_table( struct dre2 *graph );
void dre2_duplicate_group( struct dre2_node **v, int *node_count, int *last_node, struct dre2_parse_return *res, int **minimal );
void dre2_duplicate_node( struct dre2_node **v, int *node_count, int last_node, int **minimal );
void dre2_make_range( struct dre2_node **v, int *node_count, int *last_node, struct dre2_parse_return *res, int min, int max, int **minimal );
struct dre2_parse_return dre2_parse_recursive( struct dre2_node **v, int *node_count, unsigned char *re, int length, int pos, int **minimal );
struct dre2 dre2_parse( unsigned char *re );
void print_dre2( struct dre2 *graph );
void print_reverse_dre2( struct dre2 *graph );