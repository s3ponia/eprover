/*-----------------------------------------------------------------------

File  : cco_ho_inferences.j

Author: Petar Vukmirovic

Contents

  Declarations of functions that implement higher-order inferences that are
  non-essential to superposition. 

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <che_proofcontrol.h>

void ComputeHOInferences(ProofState_p state, ProofControl_p control, Clause_p clause);