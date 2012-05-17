#include "dre2.h"

// Recursive backtracking function - also sets group open/close positions.
int dre2_backtrack_recursive( struct dre2 *graph, unsigned char *input, int pos, int id, int **state, int **group_open, int **group_close )
{
  int i, j;
  int offset;
  int g_open, g_close;
  unsigned char *tp;
  int *last_state;

  tp = input + pos;

  // If we are at the regex close node, then we've matched as much as possible, return.
  if ( id == graph->count - 1 )
    return true;

  // If we are past the end of input, we can't match, return false.
  if ( *tp == '\0' && graph->v[id].c != DRE2_GROUP_OPEN && graph->v[id].c != DRE2_GROUP_CLOSE )
    return false;

  // Make sure we haven't already tried this node.
  if ( ( *state )[id] )
    return false;

  last_state = ( int * )malloc( sizeof( int ) * graph->count );
  for ( i = 0; i < graph->count; i++ )
    last_state[i] = ( *state )[i];

  ( *state )[id] = true;

  // Match the input character if it's not a group.
  if ( graph->v[id].c != DRE2_GROUP_OPEN && graph->v[id].c != DRE2_GROUP_CLOSE )
  {
    if ( ( graph->v[id].c == DRE2_EOF && ( ( *tp == ' ' && pos >= strlen( input ) - 1 ) || *tp == '\0' ) ) || dre2_char_matches( graph, &graph->v[id], *tp ) )
    {
      offset = 1;
      for ( i = 0; i < graph->count; i++ )
        ( *state )[i] = false;
      pos++;
    } else
    {
      free( last_state );
      last_state = NULL;
      return false;
    }
  } else
  {
    offset = 0;
  }

  // Set the group open and close positions, but also track where it was so we can revert later if needed.
  if ( graph->v[id].c == DRE2_GROUP_OPEN && graph->v[id].group_id > 0 )
  {
    g_open = ( *group_open )[graph->v[id].group_id - 1];
    ( *group_open )[graph->v[id].group_id - 1] = pos;
  } else if ( graph->v[id].c == DRE2_GROUP_CLOSE && graph->v[id].group_id > 0 )
  {
    g_close = ( *group_close )[graph->v[id].group_id - 1];
    ( *group_close )[graph->v[id].group_id - 1] = pos;
  }

  // Check each of this nodes neighbors.
  for ( i = 0; i < graph->v[id].n_count; i++ )
  {
    // If the recursive function returns true, the regex matched.
    if ( dre2_backtrack_recursive( graph, input, pos, graph->v[id].n[i], state, group_open, group_close ) )
    {
      free( last_state );
      last_state = NULL;
      return true;
    }
  }

  // Revert state table on failure as well.
  for ( i = 0; i < graph->count; i++ )
    ( *state )[i] = last_state[i]; //false;

  // Revert group open/close on failure.
  if ( graph->v[id].c == DRE2_GROUP_OPEN && graph->v[id].group_id > 0 )
  {
    ( *group_open )[graph->v[id].group_id - 1] = g_open;
  } else if ( graph->v[id].c == DRE2_GROUP_CLOSE && graph->v[id].group_id > 0 )
  {
    ( *group_close )[graph->v[id].group_id - 1] = g_close;
  }
  free( last_state );
  last_state = NULL;
  return false;
}

// Perform backtracking match, returning submatch info (if any)
void dre2_backtrack_match( struct dre2 *graph, unsigned char *input, unsigned char ***submatches )
{
  int i, j;
  int matched, pos;
  int *state;
  int *group_open, *group_close;

  // Allocate some memory for the state table to ensure we don't ever get in an infinite loop.
  state = ( int * )calloc( graph->original->count, sizeof( int ) );

  // Allocate some memory for extracting submatches.
  if ( graph->original->group_count > 1 )
  {
    group_open = ( int * )calloc( graph->original->group_count, sizeof( int ) );
    group_close = ( int * )calloc( graph->original->group_count, sizeof( int ) );
  } else
  {
    return;
  }

  if ( ( matched = dre2_backtrack_recursive( graph->original, input, 0, 0, &state, &group_open, &group_close ) ) )
  {
    for ( i = 1; i < graph->original->group_count; i++ )
    {
      if ( group_close[i - 1] > group_open[i - 1] )
        sprintf( ( *submatches )[i - 1], "%.*s", group_close[i - 1] - group_open[i - 1], input + group_open[i - 1] );
    }
  } else
  {
    printf( "FAIL!\n" );
  }

  // Free up memory used.
  if ( graph->original->group_count > 1 )
  {
    free( state ); state = NULL;
    free( group_open ); group_open = NULL;
    free( group_close ); group_close = NULL;
  }
}