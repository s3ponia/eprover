/*-----------------------------------------------------------------------

File  : ccl_freqvectors.h

Author: Stephan Schulz

Contents

  Functions for handling frequency count vectors and permutation
  vectors. 

  2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  8 21:48:35 CEST 2003  
    New (separated FreqVector from fcvindexing.*)

-----------------------------------------------------------------------*/

#ifndef CCL_FREQVECTORS

#define CCL_FREQVECTORS

#include <clb_pdarrays.h>
#include <clb_fixdarrays.h>
#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef FixedDArray_p PermVector_p;

typedef struct tuple2_cell
{
   long pos;
   long value;
}Tuple2Cell;

typedef struct freq_vector_cell
{
   long size;        /* How many fields? */
   long sig_symbols;
   long *array;
   Clause_p clause; /* Just an unprotected reference */
}FreqVectorCell, *FreqVector_p, *FVPackedClause_p;

#define NON_SIG_FEATURES 3


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PermVectorAlloc(size) FixedDArrayAlloc(size)
#define PermVectorFree(junk)  FixedDArrayFree(junk)
#define PermVectorCopy(vec)   FixedDArrayCopy(vec)
#define PermVectorPrint(out,vec) FixedDArrayPrint((out),(vec))

PermVector_p PermVectorCompute(FreqVector_p fmax, FreqVector_p fmin, 
			       FreqVector_p sums, 
			       long clauses, long pos_lit_clauses,
			       long neg_lit_clauses ,long max_len, 
			       bool eleminate_uninformative);


#define FreqVectorCellAlloc()    (FreqVectorCell*)SizeMalloc(sizeof(FreqVectorCell))
#define FreqVectorCellFree(junk) SizeFree(junk, sizeof(FreqVectorCell))

#define SigSizeToFreqVectorSize(size) (size*2-2+NON_SIG_FEATURES)
#define StandardFreqVNegIndex(vec, i) \
        (((i)>=NON_SIG_FEATURES) && ((i)<=((vec)->sig_symbols+NON_SIG_FEATURES-1)))
#define StandardFreqVPosIndex(vec, i) \
        (((i)>((vec)->sig_symbols+NON_SIG_FEATURES-1)) && \
	 ((i)<SigSizeToFreqVectorSize((vec)->sig_symbols)))
FreqVector_p FreqVectorAlloc(long size);

void         FreqVectorFreeReal(FreqVector_p junk);
#ifndef NDEBUG
#define FreqVectorFree(junk) FreqVectorFreeReal(junk);junk=NULL
#else
#define FreqVectorFree(junk) FreqVectorFreeReal(junk)
#endif

void         FreqVectorInitialize(FreqVector_p vec, long value);

void         FreqVectorPrint(FILE* out, FreqVector_p vec);

void             StandardFreqVectorAddVals(FreqVector_p vec, long sig_symbols, 
					   Clause_p clause);
FreqVector_p     StandardFreqVectorCompute(Clause_p clause, long sig_symbols);
FreqVector_p     OptimizedFreqVectorCompute(Clause_p clause, 
					    PermVector_p perm, 
					    long sig_symbols);
FVPackedClause_p FVPackClause(Clause_p clause, PermVector_p perm, 
			      long symbol_limit);
Clause_p         FVUnpackClause(FVPackedClause_p pack);

void             FVPackedClauseFreeReal(FVPackedClause_p pack);
#ifndef NDEBUG
#define FVPackedClauseFree(junk) FVPackedClauseFreeReal(junk);junk=NULL
#else
#define FVPackedClauseFree(junk) FVPackedClauseFreeReal(junk)
#endif


void FreqVectorAdd(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2);
void FreqVectorMulAdd(FreqVector_p dest, FreqVector_p s1, long f1, 
		      FreqVector_p s2, long f2);
#define FreqVectorSub(dest, s1, s2) FreqVectorMulAdd((dest),(s1), 1, (s2), -1)
void FreqVectorMax(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2);
void FreqVectorMin(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





