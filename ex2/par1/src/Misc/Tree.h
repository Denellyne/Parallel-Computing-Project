#ifndef __VECTOR__HEADER__
#define __VECTOR__HEADER__

#include <stdlib.h>
#include <stdio.h>

// ------------------------------------------------------------------------------- //

// These implementatins were not made with optimization in mind.

// For safety, pairs that are attributed to nodes within a tree
// are always copied. This can be easily changed.

// Less flexibility is more.
// Negative freedom is key for a group project.

// Also, do excuse my code structure.
// It may seem strange at first.
// But fret not. It has its own advantages.

// ------------------------------------------------------------------------------- //

// The order in which sections of code can be found is as follows.

// First, the basic code for the data structures can be found.

// Next, the code for decision-making based on entropy values
// can be found.

// ------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------- //

// The code sections begin here.

// ------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------- //


// The code section for entropy-based decision-making follows.

// ------------------------------------------------------------------------------- //

// For pairs of integers.
typedef struct IntPair {
  int fst ;
  int snd ;
} IntPair ;

// ------------------------------------------------------------------------------- //

// For Binary Search Trees.
// The left and right members refer
// to child nodes.

// The botLeftCorner and topRightCorner
// refer to the bottom left and the top
// right corners of each partition, which
// alone can identify the borders of their
// respective partition.
typedef struct BST {
  IntPair     topLeftCorner  ;
  IntPair     botRightCorner ;
  struct BST* left           ;
  struct BST* right          ;
} BST ;

// ------------------------------------------------------------------------------- //

// The CutAxis enum is probably not going to be useful.
// enum CutAxis { CutHori , CutVerti } ;

// ------------------------------------------------------------------------------- //

// Simple introduction and elimination forms for
// pairs of integers (elements of the IntPair type).

// This function, newIntPair, creates a pair out of
// two integers (Introduction form).
IntPair newIntPair ( int first , int second ) {
  return
    ( struct IntPair )
      { . fst = first
      , . snd = second
      } ;
}

// ------------------------------------------------------------------------------- //

// The following two functions, fstIntPair and sndIntPair,
// each extract the first and second element of the given
// pair, respectively (Elimination forms).
int fstIntPair ( IntPair pair ) { return pair . fst ; }

int sndIntPair ( IntPair pair ) { return pair . snd ; }

// ------------------------------------------------------------------------------- //

// The following function, printIntPair, prints a given IntPair,
// that is, it shows each of its entries as a pair ( fst , snd ).
void printIntPair ( IntPair pair )
  { printf( "( %d , %d )" , pair . fst , pair . snd ) ; }

// ------------------------------------------------------------------------------- //

// The following functions, insertLeftIntPair and insertRightIntPair,
// exist so that insertion into either entry of an IntPair is done
// efficiently. Respectively, they insert into the fst and the snd entries.
void insertFstIntPair ( IntPair pair , int newValueFst )
  { pair . fst = newValueFst ; }

void insertFstSndPair ( IntPair pair , int newValueSnd )
  { pair . snd = newValueSnd ; }

// ------------------------------------------------------------------------------- //

// This function, newBST, creates a base, single-partition, BST.

// Note that a BST must always refer to some map of the game,
// which is why it requires two arguments (for each corner
// coordinate of the map).

// Also note that the IntPair arguments are not passed as
// pointers, as a safety-measure. Every single pair has one
// single owner, this way (as long as only this function is
// the one used for BST creation)
BST* newBST ( IntPair leftPair , IntPair rightPair ) {
  BST* root = ( BST* )( malloc( sizeof( BST ) ) ) ;

  root -> left  = NULL ;
  root -> right = NULL ;
  root -> topLeftCorner  = leftPair  ;
  root -> botRightCorner = rightPair ;

  return root ;
}

// ------------------------------------------------------------------------------- //

// Convenience functions (not necessary but might be convenient).

// Both insertLeftBST and insertRightBST have the purpose of
// enabling the program to "insert" other BSTs into one of
// the sides of an existing tree. Respectively, into the left
// and the right sides.

// Not sure if they are going to be useful.

void insertLeftBST ( BST* root , BST* newLeft ) {
  free ( root -> left ) ;

  root -> left = newLeft ;
}

void insertRightBST ( BST* root , BST* newRight ) {
  free ( root -> right ) ;

  root -> right = newRight ;
}

// Inserts into both the left and right child nodes of a
// given tree.
void insertLeftRightBST ( BST* root
			, BST* newLeft
			, BST* newRight  ) {
  free ( root -> left  ) ;
  free ( root -> right ) ;

  root -> left  = newLeft  ;
  root -> right = newRight ;
}

// ------------------------------------------------------------------------------- //

// The following function, mergeBST, takes a tree and merges its
// children. As each node of the tree is an entire partition of
// the map of a game, and its children are two (possibly uneven)
// halves itself, all that needs to be done is get rid of its
// children.

// Feel free to comment out the satefy check. I'd say it's better
// to leave it only when performance actually matters. During
// development, I'd advise against its removal. It can save time.
// The point here is that certain behaviours are meaningless.
void mergeBST ( BST* root ) {
  if ( root -> left == NULL || root -> right == NULL ) {
    printf( "\n\nERROR: Attempted to merge a BST with null children!\n\n" ) ;
    abort() ;
  }

  free( root -> left  ) ;
  free( root -> right ) ;

  root -> left  = NULL ;
  root -> right = NULL ;
}

// ------------------------------------------------------------------------------- //

/**************************************************/
/* Regarding the fstCoord and sndCoord arguments, */
/* the coordinate arguments of the splitBST()	  */
/* functions:					  */
/**************************************************/
/**************************************************/
/* When determining the corners of the resulting  */
/* partitions (from the cut, a pair of two	  */
/* coordinates):				  */
/* 						  */
/* + An horizontal axis cut implies that the	  */
/*   leftmost coordinate of the cut is used as	  */
/*   the top-left corner of the new top side	  */
/*   partition. On the other hand, the rightmost  */
/*   coordinate is used as the bottom-right 	  */
/*   corner of the new bottom side partition.     */
/* + Similarly, a vertical axis cut implies that  */
/*   the topmost coordinate of the cut is used	  */
/*   as the top-left corner of the new left side  */
/*   partition. On the other hand, the bottommost */
/*   coordinate is used as the bottom left corner */
/*   of the new right side partition.		  */
/* 						  */
/* For simplicity and uniformity, we will use	  */
/* the first coordinate argument as both the	  */
/* leftmost coordinate (in the implied case	  */
/* of the horizontal axis cut) and the topmost	  */
/* coordinate (again, in the implied case of	  */
/* the vertical axis cut). The second variable	  */
/* will represent the second (respective)	  */
/* coordinate of the cut, as described above.	  */
/**************************************************/

/*****************************************************/
/* Note that, when searching the tree, the order of  */
/* the partitions is always left-right and top-down. */
/* 						     */
/* Particularly, the left child of a BST node is     */
/* always either going to be the left-side partition */
/* or the top-side partition. Predictably, the right */
/* child of a BST node is always either going to be  */
/* the right-side partition or the bottom-side	     */
/* partition.					     */
/*****************************************************/

// Feel free to comment out the satefy check. I'd say it's better
// to leave it only when performance actually matters. During
// development, I'd advise against its removal. It can save time.
// The point here is that certain behaviours are meaningless.
// As usual, all pairs must be passed as non-pointers.
// Recall that this is merely a safety measure.
// If need be, it can be removed.
void splitBST ( BST* root , IntPair fstCoord , IntPair sndCoord ) {
  if ( root -> left != NULL || root -> right != NULL ) {
    printf( "\n\nERROR: Attempted to split a BST with non null children!\n\n" ) ;
    abort() ;
  }

  root -> left  = newBST( root -> topLeftCorner , fstCoord   ) ;
  root -> right = newBST( sndCoord ,  root -> botRightCorner ) ;
}

// ------------------------------------------------------------------------------- //

void freeBST ( BST* root ) {
  if ( root != NULL ) {
    freeBST( root -> left  ) ;
    freeBST( root -> right ) ;

    free( root ) ;
  }
}

// ------------------------------------------------------------------------------- //

void printBST ( BST* root ) {
  if ( root == NULL ) {
    printf( "NullTree" ) ;
  } else {
    printf( "( BST " ) ;

    printIntPair( root -> botLeftCorner  ) ;

    printf( " " ) ;

    printIntPair( root -> topRightCorner ) ;

    printf( " " ) ;

    printBST( root -> left  ) ;

    printf( " " ) ;

    printBST( root -> right ) ;

    printf( " )" ) ;
  }
}

// ------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------- //

// The code section for entropy-based decision-making follows.

// ------------------------------------------------------------------------------- //

// My own 2-ary min/max functions.
int min ( int x , int y ) { return x < y ? x : y ; }

int max ( int x , int y ) { return x > y ? x : y ; }

// ------------------------------------------------------------------------------- //

/************************************************************/
/* Although this structure is predictable, it can take some */
/* getting used to. For smaller expressions it has a clear  */
/* negative effect. I hope that this did not end up looking */
/* too horrid.						    */
/************************************************************/
/************************************************************/
/* In essence, this function is a candidate for		    */
/* determining the entropy. It takes the number of	    */
/* bunnies and foxes, the total space in the partition	    */
/* of relevance, and the amount of steps (iterations)	    */
/* between each entropy recalculation.			    */
/*                                                          */
/* It attempts to predict the growth in the number of	    */
/* entities, by multiplying <nBunnies> - <nFoxes> by 2	    */
/* (thus the left-shift (<<)) <stepIterations> times.	    */
/* <nBunnies> - <nFoxes> may be lower than 0, so we use	    */
/* the max function for making sure it is not lower than 0. */
/*                                                          */
/* Finally, we add the number of foxes since they do not    */
/* usually go away. Finally, using the min function,	    */
/* we make sure this number is never greater than the	    */
/* <totalSpace>, since it is the greatest number of	    */
/* entities in a space. This is merely an heuristic	    */
/* though.						    */
/* 							    */
/* Enteties may cross to other partitions so this is	    */
/* not a great effort at predicting. It is optimistic	    */
/* in some senses and pessimistic in others. We could	    */
/* try tweaking the numbers using floats, but that is	    */
/* overhead on a (possibly) frequent calculation.	    */
/************************************************************/
int calculateEntropy ( int nBunnies   , int nFoxes
		     , int totalSpace , int stepIterations ) {
  return min ( ( max ( nBunnies - nFoxes
		     , 0
		     )
	         << stepIterations
	       ) + nFoxes
	     , totalSpace
	     ) ;
}

// ------------------------------------------------------------------------------- //

// The following functions might require access to stuff at main.c
// (and possibly more).

// ------------------------------------------------------------------------------- //

// The return type is not likely to just be an IntPair.
// Info could be stored in the BST nodes. This can be dangerous,
// since info can become outdated.
// Another solution for this is the usage of a vector for
// taking note of the positions of each kind of entity,
// and then merely looking for enteties that satisfy a
// given restriction. It would also be possible to pass
// down copies of sub-vectors that only include possibly
// valid entities for the partition. At the cost of memory,
// we might spend less time. The memory cost for this can be
// big though, since this would require recursive calls.
IntPair collectData ( BST* root ) {
  // recursively going down the tree
  // it seems likely that each node should hold
  // information
}

// ------------------------------------------------------------------------------- //

void decisionMakingLeaf ( BST* root ) {
  // check searchBorderBST, in particular the first
  // part of the inner if-statement
}

void decisionMakingNonLeaf ( BST* root ) {
  // check searchBorderBST, in particular the second
  // part of the inner if-statement  
}

// ------------------------------------------------------------------------------- //

// This function must use openMP.
// The fstCoord is always either the leftmost or the topmost coord, and the
// sndCoord is always either the rightmost or the bottommost coord.
// Where is the coord (0, 0)? Top-left? Bottom-right? Neither? This must be checked.
// I assume it is on the top-left.
void searchBorderBST ( BST* root , IntPair fstCoord , IntPair sndCoord , int isVerti ) {
  int isVerti = fstCoord . first - sndCoord . first == 0 ; // (this should be computed beforehand)
  // if this if-condition passes, it means the border is still valid,
  // otherwise nothing is done and the thread simply goes back.
  if (  ( isVerti
	  && ( fstCoord . snd <= ( root -> topLeftCorner ) . snd
  	       || sndCoord . snd >= ( root -> botRightCorner ) . snd ) )
     || ( !( isVerti )
	  && ( fstCoord . fst <= ( root -> topLeftCorner ) . fst
	       || sndCoord . fst >= ( root -> botRightCorner ) . fst ) ) ) {
    if ( root . left == NULL && root . right == NULL ) {
      // do critical, end-partition, stuff
      // for instance, call the game single-step evaluation function
      // decide whether or not this BST node should be split
      // (merging is not valid at end-partition BST nodes)
    } else {
      // do critical, non-end-partition, stuff
      // go left and send another thread to the right
      // whichever arrives here sooner disappears
      // the one that arrives later gets to do the decision-making
      // that is, decide whether or not this BST node should be
      // merged or split, by reevaluating the entropy. (how could
      // this entropy have changed? at end-partitions the goal is
      // to call a game single-step evaluation function and then
      // propagate the changes by going up)      
    }
  }
}

// ------------------------------------------------------------------------------- //

// TODO
// Data-collection (for entropy) -- This means having a way to get the number of
// bunnies and foxes within a space, as well as the total space.
// This is somewhat difficult to achieve without big expenses.
// How can we do this incrementally?

// Decision-making function.
// This has to do with baseline levels for merging/splitting.
// However, should it not have more to do with the relative amount of entropy out
// there?
// A segmentation tree would be great for lazy-updates.

// Search function.
// A simple binary search tree function. The criterium? Intervals.
// It must exist so that we can know the adjacent partitions.
// More concretely, given a pair of points (forming a straight line),
// how do we found a partition that shares them? Is this unique?

// This is problematic. Look at the following partitioning of a
// rectangle. The bottom-right partition shares a border with two
// rectangles at the same time. This means that, in a search,
// given the whole border (the pair of points forming a stright line),
// We must find every single partition matching that border in any way.
 
/*****************/
/* |-------|---| */
/* |       |   | */
/* |----|------- */
/* |    |      | */
/* |-----------| */
/*****************/

// ------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------- //

// The code sections end here.

// ------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------- //

#endif // !VECTOR
