/*-----------------------------------------------------------------------

File  : cte_termtypes.h

Author: Stephan Schulz

Contents

  Declarations for the basic term type and primitive functions, mainly
  on single term cells. This module mostly provides only
  infrastructure for higher level modules.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Created: Tue Feb 24 01:23:24 MET 1998 - Ripped out of the now obsolete
         cte_terms.h

-----------------------------------------------------------------------*/

#ifndef CTE_TERMTYPES

#define CTE_TERMTYPES

#include <clb_partial_orderings.h>
#include <cte_signature.h>
#include <clb_sysdate.h>
#include <clb_ptrees.h>
#include <clb_properties.h>
#include <cte_simpletypes.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_VWEIGHT  1  /* This has to be an integer > 0! */
#define DEFAULT_FWEIGHT  2  /* This has to be >= DEFAULT_VWEIGHT */

/* POWNRS = Probably obsolete with new rewriting scheme */

typedef enum
{
   TPIgnoreProps      =      0, /* For masking properties out */
   TPRestricted       =      1, /* Rewriting is restricted on this term */
   TPTopPos           =      2, /* This cell is a entry point */
   TPIsGround         =      4, /* Shared term is ground */
   TPPredPos          =      8, /* This is an original predicate
                                   position morphed into a term */
   TPIsRewritable     =     16, /* Term is known to be rewritable with
                                   respect to a current rule or rule
                                   set. Used for removing
                                   backward-rewritable clauses. Absence of
                                   this flag does not mean that the term
                                   is in any kind of normal form! POWNRS */
   TPIsRRewritable    =     32, /* Term is rewritable even if
                                   rewriting is restricted to proper
                                   instances at the top level.*/
   TPIsSOSRewritten   =     64, /* Term has been rewritten with a SoS
                                   clause (at top level) */
   TPSpecialFlag      =    128, /* For internal use with normalizing variables*/
   TPOpFlag           =    256, /* For internal use */
   TPCheckFlag        =    512, /* For internal use */
   TPOutputFlag       =   1024, /* Has this term already been printed (and
                                   thus defined)? */
   TPIsSpecialVar     =   2048, /* Is this a meta-variable generated by term
                                   top operations and the like? */
   TPIsRewritten      =   4096, /* Term has been rewritten (for the new
                                   rewriting scheme) */
   TPIsRRewritten     =   8192, /* Term has been rewritten at a
                                   subterm position or with a real
                                   instance (for the new rewriting
                                   scheme) */
   TPIsShared         =  16384, /* Term is in a term bank */
   TPGarbageFlag      =  32768, /* For the term bank garbage collection */
   TPIsFreeVar        =  65536, /* For Skolemization */
   TPPotentialParamod = 131072, /* This position needs to be tried for
                                   paramodulation */
   TPPosPolarity      = 1<<18,  /* In the term encoding of a formula,
                                   this occurs with positive polarity. */
   TPNegPolarity      = 1<<19,  /* In the term encoding of a formula,
                                   this occurs with negative polarity. */
   TPIsDerefedAppVar  = 1<<20,  /* Is the object obtained as a cache
                                   for applied variables -- dbg purposes */
   TPIsBetaReducible  = 1<<21,  /* Does the term have at least one subterm with
                                   lambda abstraction as term head */
   TPIsEtaReducible   = 1<<22,  /* Does the term have at least one subterm which is
                                   lambda abstraction and the last argument of body is
                                   the abstracted variable */
   TPIsDBVar          = 1<<23,   /* Term is a DB variable when it has positive f-code
                                    and this tag. Also, the term *must* have no arguments */
   TPHasLambdaSubterm = 1<<24,   /* Term has a subterm which is a lambda term */
   TPHasEtaExpandableSubterm = 1<<25,   /* Term has a subterm which can be a target of eta-expansion */
   TPHasDBSubterm     = 1<<26,    /* Term has a subterm which is a de Bruijn variable */
   TPHasNonPatternVar = 1<<27,    /* Term has an applied variable which is not a pattern*/
   TPHasAppVar = 1<<28,    /* Term has an applied variable*/
   TPHasEqNeqSym = 1<<29,    /* Term contains eq or neq symbol*/
   TPHasBoolSubterm = 1<<30,    /* Term has Boolean subterms or is a Boolean term itself*/
}TermProperties;



typedef enum  /* See CLAUSES/ccl_rewrite.c for more */
{
   NoRewrite = 0,     /* Just for completness */
   RuleRewrite = 1,   /* Rewrite with rules only */
   FullRewrite = 2    /* Rewrite with rules and equations */
}RewriteLevel;

typedef struct
{
   SysDate          nf_date[FullRewrite]; /* If term is not rewritten,
                                             it is in normal form with
                                             respect to the
                                             demodulators at this date */
   struct {
      struct termcell*   replace;         /* ...otherwise, it has been
                                             rewritten to this term */
      // long               demod_id;        /* 0 means subterm! */
      struct clause_cell *demod;          /* NULL means subterm! */
   }rw_desc;
}RewriteState;

struct tbcell;

typedef struct termcell
{
   FunCode          f_code;        /* Top symbol of term */
   TermProperties   properties;    /* Like basic, lhs, top */
   int              arity;         /* Redundant, but saves handing
                                      around the signature all the
                                      time */
   struct termcell* binding;       /* For variable bindings,
                                      potentially for temporary
                                      rewrites - it might be possible
                                      to combine the previous two in a
                                      union. */
   long             entry_no;      /* Counter for terms in a given
                                      termbank - needed for
                                      administration and external
                                      representation */
   long             weight;        /* Weight of the term, if term is in term bank */
   unsigned int     v_count;       /* Number of variables, if term is in term bank */
   unsigned int     f_count;       /* Number of function symbols, if term is in term bank */
   RewriteState     rw_data;       /* See above */
   Type_p           type;          /* Sort of the term */
   struct termcell* lson;          /* For storing shared term nodes in */
   struct termcell* rson;          /* a splay tree - see
                                      cte_termcellstore.[ch] */

#ifdef ENABLE_LFHO
   struct termcell* binding_cache; /* For caching the term applied variable
                                      expands to. */
   struct tbcell* owner_bank;                /* Bank that owns this term cell and that
                                      is responsible for lifetime management
                                      of the term */
#endif

   struct termcell* args[];         /* Flexible array member containing the arguments */

}TermCell, *Term_p, **TermRef;


typedef uintptr_t DerefType, *DerefType_p;

#define DEREF_NEVER   0
#define DEREF_ONCE    1
#define DEREF_ALWAYS  2

/* The following is an estimate for the memory taken up by a term cell
   with arguments (the argument array is not counted separately). */

#ifdef CONSTANT_MEM_ESTIMATE
#define TERMCELL_MEM 48
#define TERMARG_MEM  4
#define TERMP_MEM    4
#else
#define TERMCELL_MEM MEMSIZE(TermCell)
#define TERMARG_MEM  sizeof(void*)
#define TERMP_MEM    sizeof(Term_p)
#endif

#define TERMCELL_DYN_MEM (TERMCELL_MEM+4*TERMARG_MEM)

#ifdef ENABLE_LFHO
#define CAN_DEREF(term) ((TermIsFreeVar(term) && (term)->binding) || \
                            (TermIsAppliedFreeVar(term) && ((term)->args[0]->binding)))
#else
#define CAN_DEREF(term) (((term)->binding))
#endif


// checks if the binding is present and if it is the cache for the
// right term
#define BINDING_FRESH(t) (TermGetCache(t) && (t)->binding && \
                          (t)->binding == (t)->args[0]->binding)

#ifdef ENABLE_LFHO
/* Sometimes we are not interested in the arity of the term, but the
   number of arguments the term has. Due to encoding of applied variables,
   we have to discard argument 0, which is actually the head variable */
#define ARG_NUM(term)    (TermIsPhonyApp(term) ? (term)->arity-1 : (term)->arity)
/* If we have the term X a Y and bindings X -> f X Y and Y -> Z
   when we deref once we want to get f X Y a Z. When dereferencing applied
   var X a Y we can behave like with variables and decrease deref (see TermDeref)
   in which case we get term f X Y a Y as result. If we do not decrease deref
   we get f (f X Y) a Z as result. Netiher are correct. Thus, there is
   a part of term (up to DEREF_LIMIT) for which we do not follow pointers and
   then other part (after and including DEREF_LIMIT) for which we do follow pointers.  */
#define DEREF_LIMIT(t,d) ((TermIsAppliedFreeVar(t) && (t)->args[0]->binding && (d) == DEREF_ONCE) ? \
                           (((TermIsLambda((t)->args[0]->binding)) ? 1 : (t)->args[0]->binding->arity) +\
                           ((TermIsFreeVar((t)->args[0]->binding)) ? 1 : 0)) \
                           : 0)
/* Sets derefs according to the previous comment and expects i to be an index
   into arugment array, l to be DEREF_LIMIT and d wanted deref mode*/
#define CONVERT_DEREF(i, l, d) (((i) < (l) && (d) == DEREF_ONCE) ? DEREF_NEVER : (d))
#else
/* making sure no compiler warnings are produced */
#define ARG_NUM(term)          ((term)->arity)
#define DEREF_LIMIT(t,d)       (UNUSED(t),UNUSED(d),0)
#define CONVERT_DEREF(i, l, d) (UNUSED(i),UNUSED(l),d)
#endif

typedef Term_p (*TermMapper_p)(void*, Term_p);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* Functions which take two terms and return a boolean, i.e. test for
   equality */

#define TERMS_INITIAL_ARGS 10

#define RewriteAdr(level) (assert(level),(level)-1)
#define TermIsFreeVar(t) ((t)->f_code < 0)
#define TermIsConst(t)(!TermIsAnyVar(t) && ((t)->arity==0))
#define TermHasEqNeq(t)(QueryProp((t), (TPHasEqNeqSym)))
#ifdef ENABLE_LFHO
#define TermIsDBVar(term) (QueryProp((term), (TPIsDBVar)))
#define TermHasBoolSubterm(t)(QueryProp((t), (TPHasBoolSubterm)))
#define TermIsPhonyApp(term) (!TermIsDBVar(term) && (term)->f_code == SIG_PHONY_APP_CODE)
#define TermIsAppliedFreeVar(term) (!TermIsDBVar(term) && (term)->f_code == SIG_PHONY_APP_CODE && \
                                    TermIsFreeVar((term)->args[0]))
#define TermIsAppliedDBVar(term) (!TermIsDBVar(term) && (term)->f_code == SIG_PHONY_APP_CODE && \
                                    TermIsDBVar((term)->args[0]))
#define TermIsAppliedAnyVar(term) (!TermIsDBVar(term) && (term)->f_code == SIG_PHONY_APP_CODE && \
                                   TermIsAnyVar((term)->args[0]))
#define TermIsLambda(term) (!TermIsDBVar(term) &&\
                            ((term)->f_code == SIG_NAMED_LAMBDA_CODE || \
                            (term)->f_code == SIG_DB_LAMBDA_CODE))
#define TermIsAnyVar(term) (TermIsFreeVar(term) || TermIsDBVar(term))
#define TermHasLambdaSubterm(term) (QueryProp((term), (TPHasLambdaSubterm)))
#define TermHasEtaExpandableSubterm(term) (QueryProp((term), (TPHasEtaExpandableSubterm)))
#define TermHasDBSubterm(term) (QueryProp((term), (TPHasDBSubterm)))
#define TermHasAppVar(term) (QueryProp((term), (TPHasAppVar)))
// does a term have a feature that does not belong to LFHOL?
#define LFHOL_UNSUPPORTED(t) (TermHasLambdaSubterm(t) || TermHasDBSubterm(t))
#define TermIsPattern(term) (!(QueryProp((term), (TPHasNonPatternVar))))
#define TermIsNonFOPattern(term) (TermIsPattern(term) && LFHOL_UNSUPPORTED(term))
#define TermIsDBLambda(term) (!TermIsDBVar(term) && (term)->f_code == SIG_DB_LAMBDA_CODE)
#define TermIsPhonyAppTarget(term) (TermIsAnyVar(term) || TermIsLambda(term) || \
                                    (term)->f_code == SIG_ITE_CODE || \
                                    (term)->f_code == SIG_LET_CODE)
#else
#define TermIsPhonyApp(term) (false)
#define TermIsAppliedFreeVar(term) (false)
#define TermHasBoolSubterm(t)(false)
#define TermIsAppliedDBVar(term) (false)
#define TermIsAppliedAnyVar(term) (false)
#define TermIsDBLambda(term) (false)
#define TermIsLambda(term) (false)
#define TermIsDBVar(term) (false)
#define TermIsAnyVar(term) (TermIsFreeVar(term))
#define TermHasLambdaSubterm(term) (false)
#define TermHasEtaExpandableSubterm(term) (false)
#define TermHasDBSubterm(term) (false)
#define LFHOL_UNSUPPORTED(t) (false)
#define TermIsPattern(term) (true) // every fo subterm is a pattern
#define TermIsNonFOPattern(term) (false) // every fo subterm is a pattern
#define TermHasAppVar(term) (false)
#define TermIsPhonyAppTarget(term) (false)
#endif
#define TermIsTopLevelFreeVar(term) (TermIsFreeVar(term) || TermIsAppliedFreeVar(term))
#define TermIsTopLevelDBVar(term) (TermIsDBVar(term) || TermIsAppliedDBVar(term))
#define TermIsTopLevelAnyVar(term)  (TermIsAnyVar(term) || TermIsAppliedAnyVar(term))

#define TermCellSetProp(term, prop) SetProp((term), (prop))
#define TermCellDelProp(term, prop) DelProp((term), (prop))
#define TermCellAssignProp(term, sel, prop) AssignProp((term),(sel),(prop))
/* Are _all_ properties in prop set in term? */
#define TermCellQueryProp(term, prop) QueryProp((term), (prop))

/* Are any properties in prop set in term? */
#define TermCellIsAnyPropSet(term, prop) IsAnyPropSet((term), (prop))

#define TermCellGiveProps(term, props) GiveProps((term),(props))
#define TermCellFlipProp(term, props) FlipProp((term),(props))

#define TermCellAlloc() (TermCell*)SizeMalloc(sizeof(TermCell))
#define TermCellArityAlloc(arity) (TermCell*)SizeMalloc(sizeof(TermCell) + (arity) * sizeof(Term_p))
#define TermCellFree(junk, arity)         SizeFree(junk, sizeof(TermCell) + (arity) * sizeof(Term_p))

/* ACHTUNG: To be used only when allocating/deallocating arrays that are of temporary nature
   and will *not* be directly assigned to flexibly crated array */
#define TermArgTmpArrayAlloc(n) ((TermCell**) ((n) == 0 ? NULL : SizeMalloc((n)*sizeof(Term_p))))
#define TermArgTmpArrayFree(junk, n) (((n)==0) ? NULL : ( SizeFreeReal((junk), (n)*sizeof(Term_p)) ))

#define TermIsRewritten(term) TermCellQueryProp((term), TPIsRewritten)
#define TermIsRRewritten(term) TermCellQueryProp((term), TPIsRRewritten)
#define TermIsTopRewritten(term) (TermIsRewritten(term)&&TermRWDemodField(term))
#define TermIsShared(term)       TermCellQueryProp((term), TPIsShared)

#ifdef ENABLE_LFHO
Term_p  MakeRewrittenTerm(Term_p orig, Term_p new, int orig_remains, struct tbcell* bank);
#else
#define MakeRewrittenTerm(orig, new, remains, bank) (assert(!remains), new)
#endif

#define TermNFDate(term,i) (TermIsRewritten(term)?\
                           SysDateCreationTime():(term)->rw_data.nf_date[i])

/* Absolutely get the value of the replace and demod fields */
#define TermRWReplaceField(term) ((term)->rw_data.rw_desc.replace)
#define TermRWDemodField(term)   ((term)->rw_data.rw_desc.demod)
#define REWRITE_AT_SUBTERM 0

/* Get the logical value of the replaced term / demodulator */
#define TermRWReplace(term) (TermIsRewritten(term)?TermRWTargetField(term):NULL)
#define TermRWDemod(term) (TermIsRewritten(term)?TermRWDemodField(term):NULL)

static inline Term_p TermDefaultCellAlloc(void);
static inline Term_p TermDefaultCellArityAlloc(int arity);
static inline Term_p TermConstCellAlloc(FunCode symbol);
static inline Term_p TermTopAlloc(FunCode f_code, int arity);
static inline Term_p TermTopCopy(Term_p source);
static inline Term_p TermTopCopyWithoutArgs(Term_p source);

void    TermTopFree(Term_p junk);
void    TermFree(Term_p junk);
Term_p  TermAllocNewSkolem(Sig_p sig, PStack_p variables, Type_p type);

void    TermSetProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermSearchProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermVerifyProp(Term_p term, DerefType deref, TermProperties prop,
                       TermProperties expected);
void    TermDelProp(Term_p term, DerefType deref, TermProperties prop);
void    TermDelPropOpt(Term_p term, TermProperties prop);
void    TermVarSetProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermVarSearchProp(Term_p term, DerefType deref, TermProperties prop);
void    TermVarDelProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermHasInterpretedSymbol(Term_p term);

bool    TermIsPrefix(Term_p needle, Term_p haystack);
static inline Type_p GetHeadType(Sig_p sig, Term_p term);

static inline Term_p  TermDerefAlways(Term_p term);
static inline Term_p  TermDeref(Term_p term, DerefType_p deref);

static inline Term_p  TermTopCopy(Term_p source);

void    TermStackSetProps(PStack_p stack, TermProperties prop);
void    TermStackDelProps(PStack_p stack, TermProperties prop);

#ifdef ENABLE_LFHO
#define TermGetCache(t)    ((t)->binding_cache)
#define TermSetCache(t,c)  ((t)->binding_cache = (c))
#define TermGetBank(t)     ((t)->owner_bank)
#define TermSetBank(t,b)   ((t)->owner_bank = (b))

#define TermIsBetaReducible(t) TermCellQueryProp((t), TPIsBetaReducible)
#define TermIsEtaReducible(t)  TermCellQueryProp((t), TPIsEtaReducible)
#else
#define TermGetCache(t)    (UNUSED(t), NULL)
#define TermSetCache(t,c)  (UNUSED(t), UNUSED(c), UNUSED(NULL))
#define TermGetBank(t)     (UNUSED(t), NULL)
#define TermSetBank(t,b)   (UNUSED(t), UNUSED(b), UNUSED(NULL))
#define TermIsBetaReducible(t) false
#define TermIsEtaReducible(t)  false
#endif


/*---------------------------------------------------------------------*/
/*                  Inline functions                                   */
/*---------------------------------------------------------------------*/

#ifdef ENABLE_LFHO
// forward declaration of function used in inline functions
Term_p applied_var_deref(Term_p orig);
#endif

/*-----------------------------------------------------------------------
//
// Function: GetHeadType()
//
//   Returns the type of the head term symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Type_p GetHeadType(Sig_p sig, Term_p term)
{
   if(term->f_code == SIG_ITE_CODE)
   {
      assert(term->arity==3);
      return term->type;
   }
   else if(term->f_code == SIG_LET_CODE)
   {
      return term->type;
   }
   else if((term->f_code == sig->qex_code) || (term->f_code == sig->qall_code))
   {
      return sig->type_bank->bool_type;
   }
#ifdef ENABLE_LFHO
   else if(TermIsAppliedAnyVar(term))
   {
      assert(!sig || term->f_code == SIG_PHONY_APP_CODE);
      return term->args[0]->type;
   }
   else if(TermIsAnyVar(term) || TermIsLambda(term))
   {
      assert(!TermIsAnyVar(term) || term->arity == 0);
      return term->type;
   }
   else
   {
      assert(term->f_code != SIG_PHONY_APP_CODE);
      return SigGetType(sig, term->f_code);
   }
#else
   return SigGetType(sig, term->f_code);
#endif
}

/*-----------------------------------------------------------------------
//
// Function: GetFVarHead()
//
//   If a term is (possibly applied) free variable, get the term
//   which represents this free variable.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p GetFVarHead(Term_p t)
{
   assert(TermIsTopLevelFreeVar(t));
   if(TermIsAppliedFreeVar(t))
   {
      return t->args[0];
   }
   else
   {
      return t;
   }
}

/*-----------------------------------------------------------------------
//
// Function: deref_step()
//
//   Dereference term once
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#ifdef ENABLE_LFHO
static inline Term_p deref_step(Term_p orig)
{
   assert(TermIsTopLevelFreeVar(orig));

   if(TermIsFreeVar(orig))
   {
      return orig->binding;
   }
   else
   {
      return applied_var_deref(orig);
   }
}
#else
#define deref_step(orig) ((orig)->binding)
#endif

/*-----------------------------------------------------------------------
//
// Function: TermDerefAlways()
//
//   Dereference a term as many times as possible.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p TermDerefAlways(Term_p term)
{
   assert(TermIsTopLevelFreeVar(term) || !(term->binding));

   while(CAN_DEREF(term))
   {
#ifdef ENABLE_LFHO
      term = deref_step(term);
#else
      term = term->binding;
#endif
   }
   return term;
}

/*-----------------------------------------------------------------------
//
// Function: TermDeref()
//
//   Dereference a term. deref* tells us how many derefences to do
//   at most, it will be decremented for each dereferenciation.
//   Dereferencing applied variables creates new terms, which
//   are cached in the original applied variable. Derefing applied
//   variable will NOT decrease deref (just like it does not decrease
//   deref for a normal term). Because of this, additional care
//   needs to be taken not to take into account substitution
//   for the head of the applied variable (which is prefix of the
//   expanded term) -- see macros DEREF_LIMIT and CONVERT_DEREF.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p inline TermDeref(Term_p term, DerefType_p deref)
{
   assert(TermIsTopLevelAnyVar(term) || !(term->binding));

   if(*deref == DEREF_ALWAYS)
   {
      while(CAN_DEREF(term))
      {
#ifdef ENABLE_LFHO
      term = deref_step(term);
#else
      term = term->binding;
#endif
      }
   }
   else
   {
      while(*deref && CAN_DEREF(term))
      {
#ifdef ENABLE_LFHO
         bool originally_app_var = TermIsAppliedFreeVar(term);
         term = deref_step(term);
         if((*deref) == DEREF_ONCE && originally_app_var)
         {
            break;
         }
         else
         {
            (*deref)--;
         }
#else
      term = term->binding;
      (*deref)--;
#endif
      }
   }
   return term;
}


/*-----------------------------------------------------------------------
//
// Function: TermTopCopyWithoutArgs()
//
//   Return a copy of the term node.
//   Only the top node is duplicated.
//   Arguments are not initialized.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline Term_p TermTopCopyWithoutArgs(restrict Term_p source)
{
   Term_p handle = NULL;

   if(source->arity)
   {
      handle = TermDefaultCellArityAlloc(source->arity);
   }
   else
   {
      handle = TermDefaultCellAlloc();
   }

   /* All other properties are tied to the specific term! */
   handle->properties = (source->properties&(TPPredPos|TPIsDBVar));
   TermCellDelProp(handle, TPOutputFlag); /* As it gets a new id below */

   handle->f_code = source->f_code;
   handle->type   = source->type;

   if(source->arity)
   {
      handle->arity = source->arity;
   }

   TermSetBank(handle, TermGetBank(source));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermTopCopy()
//
//   Return a copy of the term node (and potential argument
//   pointers). Only the top node and the pointers are duplicated, the
//   arguments are shared between source and copy. As this function
//   operates on nodes, it does not follow bindings! Administrative
//   stuff (refs etc. will, of course, not be copied but initialized
//   to rational values for an unshared
//   term).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline Term_p TermTopCopy(restrict Term_p source)
{
   Term_p handle = TermTopCopyWithoutArgs(source);

   for(int i=0; i<source->arity; i++)
   {
      handle->args[i] = source->args[i];
   }

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermDefaultCellAlloc()
//
//   Allocate a term cell with default values.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline Term_p TermDefaultCellAlloc(void)
{
   Term_p handle = TermCellAlloc();

   handle->properties = TPIgnoreProps;
   handle->arity      = 0;
   handle->type       = NULL;
   handle->binding    = NULL;
   handle->rw_data.nf_date[0] = SysDateCreationTime();
   handle->rw_data.nf_date[1] = SysDateCreationTime();
   handle->lson = NULL;
   handle->rson = NULL;
   TermSetCache(handle, NULL);
   TermSetBank(handle, NULL);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermDefaultCellArityAlloc()
//
//   Allocate a term cell with default values.
//   Furthermore allocates the arguments of the term using the given arity.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline Term_p TermDefaultCellArityAlloc(int arity)
{
   Term_p handle = TermCellArityAlloc(arity);

   handle->properties = TPIgnoreProps;
   handle->arity      = arity;
   handle->type       = NULL;
   handle->binding    = NULL;

   for(int i = 0; i < arity; ++i)
      handle->args[i] = NULL;

   handle->rw_data.nf_date[0] = SysDateCreationTime();
   handle->rw_data.nf_date[1] = SysDateCreationTime();
   handle->lson = NULL;
   handle->rson = NULL;
   TermSetCache(handle, NULL);
   TermSetBank(handle, NULL);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermConstCellAlloc()
//
//   Allocate a term cell for the constant term with symbol symbol.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline Term_p TermConstCellAlloc(FunCode symbol)
{
   Term_p handle = TermDefaultCellAlloc();
   handle->f_code = symbol;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermTopAlloc()
//
//   Allocate a term top with given f_code and (uninitialized)
//   argument array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p TermTopAlloc(FunCode f_code, int arity)
{
   Term_p handle = TermDefaultCellArityAlloc(arity);
   handle->f_code = f_code;

   return handle;
}

#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
