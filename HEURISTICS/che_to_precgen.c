/*-----------------------------------------------------------------------

File  : che_to_precgen.c

Author: Stephan Schulz

Contents

  Functions implementing several precedence generation schemes for
  term orderings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Sep 25 02:49:11 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_to_precgen.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* TOPrecGenNames[]=
{
   "none",             /* PNoMethod */
   "unary_first",      /* PUnaryFirst */
   "unary_freq",       /* PUnaryFristFreq */
   "arity",            /* PArity */
   "invarity",         /* PInvArity */
   "const_max",        /* PConstFirst  */
   "const_min",        /* PInvArConstMin */
   "freq",             /* PByFrequency */
   "invfreq",          /* PByInvFrequency */
   "invconjfreq",      /* PByInvConjFrequency */
   "invfreqconjmax",   /* PByInvFreqConjMax */
   "invfreqconjmin",   /* PByInvFreqConjMin */
   "invfreqconstmin",  /* PByInvFreqConstMin */
   "invfreqhack",      /* PByInvFreqHack */
   "arrayopt",         /* PArrayOpt */
   "orient_axioms",    /* POrientAxioms */
   NULL
};


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#define PRINT_PRECEDENCE

#ifdef PRINT_PRECEDENCE
/*-----------------------------------------------------------------------
//
// Function: print_prec_array()
//
//   Print the precedence described by the array. Hacked to enable
//   Waldmeister to use the same orderings as generated by E.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void print_prec_array(FILE* out, Sig_p sig, FCodeFeatureArray_p array)
{
   FunCode i;
   char *del = "";

   fprintf(out, "# Ordering precedence: ");
   for(i = sig->f_count; i > 0; i--)
   {
      if(!SigIsSpecial(sig, array->array[i].symbol))
      {
    fprintf(out, "%s%s", del, SigFindName(sig,array->array[i].symbol));
    del = " > ";
      }
   }
   fprintf(out, "\n");
}
#endif


/*-----------------------------------------------------------------------
//
// Function: compute_precedence_from_array()
//
//   Given an ocb and a sorted array of type featuresortcell[], set
//   the precedence in the ocb.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void compute_precedence_from_array(OCB_p ocb, FCodeFeatureArray_p
                 array)
{
   FunCode i, last;

   assert(ocb->sig_size == array->size-1);
   if(ocb->prec_weights)
   {
      for(i = SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
      {
         if((SigFindArity(ocb->sig, i)==0) &&
            !SigIsPredicate(ocb->sig, i) &&
            !SigQueryFuncProp(ocb->sig, i, FPSpecial) &&
            !ocb->min_constant)
         {
            ocb->min_constant = i;
         }
    ocb->prec_weights[array->array[i].symbol] = i;
      }
      ocb->prec_weights[SIG_TRUE_CODE] = (LONG_MIN/2);
   }
   else
   {
      last = SIG_TRUE_CODE;
      for(i = SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
      {
    OCBPrecedenceAddTuple(ocb, last, array->array[i].symbol, to_lesser);
    last = array->array[i].symbol;
      }
   }
#ifdef PRINT_PRECEDENCE
   print_prec_array(GlobalOut, ocb->sig, array);
#endif
}



/*-----------------------------------------------------------------------
//
// Function: generate_unary_first_precedence()
//
//   Generate a precence in which symbols with higher arity are
//   larger, but unary symbols are larger still. Order of occurence in
//   the signature is used as a tie-breaker.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_unary_first_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode i;
   long    arity;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      arity = SigFindArity(ocb->sig, i);
      if(arity == 1)
      {
        array->array[i].key1 = INT_MAX;
      }
      else
      {
        array->array[i].key1 = arity;
      }
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_unary_first_freq_precedence()
//
//   Generate a precence in which rarer symbols are
//   larger, but unary symbols are larger still (and constants are
//   minimal).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_unary_first_freq_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode i;
   long    arity;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      arity = SigFindArity(ocb->sig, i);
      if(arity == 1)
      {
    array->array[i].key1 = 2;
      }
      else if(arity == 0)
      {
    array->array[i].key1 = 0;
      }
      else
      {
    array->array[i].key1 = 1;
      }
      array->array[i].key2 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_arity_precedence()
//
//   Generate a precende in which symbols with higher arity are
//   larger. Order of occurence in the signature is used as a
//   tie-breaker.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_arity_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invarity_precedence()
//
//   Generate a precende in which symbols with higher arity are
//   smaller. Order of occurence in the signature is used as a
//   tie-breaker.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invarity_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = -SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_const_max_precedence()
//
//   Generate a precedence in which symbols with higher arity are
//   larger, but constants are the largest symbols. Order of occurence
//   in the signature is used as a tie-breaker.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_const_max_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = SigFindArity(ocb->sig, i);
      if(!array->array[i].key1)
      {
    array->array[i].key1 = INT_MAX;
      }
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_const_min_precedence()
//
//   Generate a precedence in which symbols with higher arity are
//   smaller, but constants are the smallest symbols. Order of
//   occurence in the signature is used as a tie-breaker.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_const_min_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = -SigFindArity(ocb->sig, i);
      if(!array->array[i].key1)
      {
    array->array[i].key1 = -FREQ_SEMI_INFTY;
      }
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}



/*-----------------------------------------------------------------------
//
// Function: generate_freq_precedence()
//
//   Generate a precedence in which symbols which occur more often in
//   the specification are bigger. Arity is used as a tie-breaker,
//   then order of occurence in the signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_freq_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
      array->array[i].key2 = SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invfreq_precedence()
//
//   Generate a precedence in which symbols which occur more often in
//   the specification are smaller. Arity is used as a tie-breaker,
//   then order of occurence in the signature. .
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreq_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = -array->array[i].freq;
      array->array[i].key2 = SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invconjfreq_precedence()
//
//   Generate a precedence in which symbols which occur in conjectures
//   are larger, ordered by inverse frequency in conjectures. Ties are
//   broken by inverse overall frequency. Arity is used as a
//   tie-breaker, then order of occurence in the signature. .
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invconjfreq_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].conjfreq?
         (INT_MAX-array->array[i].conjfreq):0;
      array->array[i].key2 = -array->array[i].freq;
      array->array[i].key3 = SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invfreq_conjmax_precedence()
//
//   Generate a precedence in which conjecture symbols are larger than
//   other symbols. Inverse frequency is used within the classes,
//   arity is used as a tie-breaker, then order of occurence in the
//   signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreq_conjmax_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].conjfreq?1:0;
      array->array[i].key2 = -array->array[i].freq;
      array->array[i].key3 = SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}

/*-----------------------------------------------------------------------
//
// Function: generate_invfreq_conjmin_precedence()
//
//   Generate a precedence in which conjecture symbols are larger than
//   other symbols. Inverse frequency is used within the classes,
//   arity is used as a tie-breaker, then order of occurence in the
//   signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreq_conjmin_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].conjfreq?0:1;
      array->array[i].key2 = -array->array[i].freq;
      array->array[i].key3 = SigFindArity(ocb->sig, i);
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invfreq_constmin_precedence()
//
//   Generate a precedence in which symbols which occur more often in
//   the specification are smaller, but constants are smaller
//   still. Arity is used as an additional tie-breaker, then order of
//   occurence in the signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreq_constmin_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   int arity;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      arity = SigFindArity(ocb->sig, i);
      if(arity == 0)
      {
    array->array[i].key1 = -FREQ_SEMI_INFTY;
    array->array[i].key2 = array->array[i].freq;
      }
      else
      {
    array->array[i].key1 = -array->array[i].freq;
    array->array[i].key2 = SigFindArity(ocb->sig, i);
      }
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invfreq_hack_precedence()
//
//   Generate a precedence in which symbols which occur more often in
//   the specification are smaller, but constants are smaller
//   still. All unary function symbols that occur with the maximal
//   frequency are largest. Arity is used as an additional
//   tie-breaker, then order of occurence in the signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreq_hack_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   int arity, max_freq=-1;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      arity = SigFindArity(ocb->sig, i);
      if(arity == 1)
      {
    max_freq = MAX(max_freq, array->array[i].freq);
      }
   }
   for(i=1; i<= ocb->sig->f_count; i++)
   {
      arity = SigFindArity(ocb->sig, i);
      if(arity == 0)
      {
    array->array[i].key1 = -FREQ_SEMI_INFTY;
    array->array[i].key2 = -array->array[i].freq;
      }
      else if((arity == 1) && (array->array[i].freq == max_freq))
      {
    array->array[i].key1 = FREQ_SEMI_INFTY;
    array->array[i].key2 = 0;
      }
      else
      {
    array->array[i].key1 = -array->array[i].freq;
    array->array[i].key2 = SigFindArity(ocb->sig, i);
      }
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}



/*-----------------------------------------------------------------------
//
// Function: generate_arrayopt_precedence()
//
//   Generate a precedence for array problems with store > select >
//   a* > e* > whatever > i*.
//
//   Inverse frequency is the tie breaker.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_arrayopt_precedence(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   char* id;

   for(i=1; i<= ocb->sig->f_count; i++)
   {
      id = SigFindName(ocb->sig, i);
      if(strcmp(id, "store") == 0)
      {
         array->array[i].key1 = 30;
      }
      else if(strcmp(id, "select") == 0)
      {
         array->array[i].key1 = 25;
      }
      else if(strcmp(id, "sk") == 0)
      {
         array->array[i].key1 = 20;
      }
      else if((strncmp(id, "a_",2) == 0) || (strncmp(id, "b_",2) == 0))
      {
         array->array[i].key1 = 10;
      }
      else if((strncmp(id, "a",1) == 0) || (strncmp(id, "b",1) == 0))
      {
         array->array[i].key1 = 15;
      }
      else if(strncmp(id, "e_",2) == 0)
      {
         array->array[i].key1 = 5;
      }
      else if(strncmp(id, "e",1) == 0)
      {
         array->array[i].key1 = 7;
      }
      else if(strncmp(id, "i_",2) == 0)
      {
         array->array[i].key1 = 0;
      }
      else if(strncmp(id, "i",1) == 0)
      {
         array->array[i].key1 = 2;
      }
      else
      {
         array->array[i].key1 = 5;
      }
      array->array[i].key2 = -array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   compute_precedence_from_array(ocb, array);

   FCodeFeatureArrayFree(array);
}





/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: TOTranslatePrecGenMethod()
//
//   Given a string, return the corresponding TOPrecGenMethod
//   token.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TOPrecGenMethod TOTranslatePrecGenMethod(char* name)
{
   TOPrecGenMethod method;

   method = StringIndex(name, TOPrecGenNames);

   if(method == -1)
   {
      method = PNoMethod;
   }
   return method;
}


/*-----------------------------------------------------------------------
//
// Function: TOGeneratePrecedence()
//
//   Given a pre-initialized OCB, compute a good precedence for a term
//   ordering.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

void TOGeneratePrecedence(OCB_p ocb, ClauseSet_p axioms,
                          char* predefined, TOPrecGenMethod method)
{
   assert(ocb);
   assert(ocb->precedence||ocb->prec_weights);
   assert(ocb->sig);

   if(predefined)
   {
      Scanner_p in = CreateScanner(StreamTypeUserString, predefined,
                                   true, NULL);

      TOPrecedenceParse(in, ocb);

      DestroyScanner(in);
   }

   VERBOUTARG("Generating ordering precedence with ",
              TOPrecGenNames[method]);
   switch(method)
   {
   case POrientAxioms:
         Error("Not yet implemented", OTHER_ERROR);
         break;
   case PNoMethod:
         if(predefined) /* User should know what he does now! */
         {
            break;
         } /* else: Fall through */
         VERBOUT("Fall-through to unary_first\n");
   case PUnaryFirst:
         generate_unary_first_precedence(ocb, axioms);
         break;
   case PUnaryFirstFreq:
         generate_unary_first_freq_precedence(ocb, axioms);
         break;
   case PArity:
         generate_arity_precedence(ocb, axioms);
         break;
   case PInvArity:
         generate_invarity_precedence(ocb, axioms);
         break;
   case PConstMax:
         generate_const_max_precedence(ocb, axioms);
         break;
   case PInvArConstMin:
         generate_const_min_precedence(ocb, axioms);
         break;
   case PByFrequency:
         generate_freq_precedence(ocb, axioms);
         break;
   case PByInvFrequency:
         generate_invfreq_precedence(ocb, axioms);
         break;
   case PByInvConjFrequency:
         generate_invconjfreq_precedence(ocb, axioms);
         break;
   case PByInvFreqConjMax:
         generate_invfreq_conjmax_precedence(ocb, axioms);
         break;
   case PByInvFreqConjMin:
         generate_invfreq_conjmin_precedence(ocb, axioms);
         break;
   case PByInvFreqConstMin:
         generate_invfreq_constmin_precedence(ocb, axioms);
         break;
   case PByInvFreqHack:
         generate_invfreq_hack_precedence(ocb, axioms);
         break;
   case PArrayOpt:
         generate_arrayopt_precedence(ocb, axioms);
         break;
   default:
         assert(false && "Precedence generation method unimplemented");
         break;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
