// ============================================================================
//        __
//   \\__/ o\    (C) 2017-2019  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
// CC64 - 'C' derived language compiler
//  - 64 bit CPU
//
// This source file is free software: you can redistribute it and/or modify 
// it under the terms of the GNU Lesser General Public License as published 
// by the Free Software Foundation, either version 3 of the License, or     
// (at your option) any later version.                                      
//                                                                          
// This source file is distributed in the hope that it will be useful,      
// but WITHOUT ANY WARRANTY; without even the implied warranty of           
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            
// GNU General Public License for more details.                             
//                                                                          
// You should have received a copy of the GNU General Public License        
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    
//                                                                          
// ============================================================================
//
#include "stdafx.h"

void swap_nodes(ENODE *node)
{
	ENODE *temp;
	temp = node->p[0];
	node->p[0] = node->p[1];
	node->p[1] = temp;
}

bool ENODE::IsEqualOperand(Operand *a, Operand *b)
{
	return (Operand::IsEqual(a, b));
};

char ENODE::fsize()
{
	switch (etype) {
	case bt_float:	return ('d');
	case bt_double:	return ('d');
	case bt_triple:	return ('t');
	case bt_quad:	return ('q');
	default:	return ('d');
	}
}

bool ENODE::HasAssignop()
{
	if (this == nullptr)
		return (false);
	if (nodetype == en_asadd
		|| nodetype == en_asadd
		|| nodetype == en_asdiv
		|| nodetype == en_asdivu
		|| nodetype == en_asmod
		|| nodetype == en_asmodu
		|| nodetype == en_asmul
		|| nodetype == en_asmulu
		|| nodetype == en_asor
		|| nodetype == en_asand
		|| nodetype == en_asxor
		|| nodetype == en_assub
		|| nodetype == en_assign
		|| nodetype == en_fcall
		|| nodetype == en_ifcall
		)
		return (true);
	if (p[0]->HasAssignop())
		return (true);
	if (p[1]->HasAssignop())
		return(true);
	if (p[2]->HasAssignop())
		return (true);
	return(false);
}

bool ENODE::HasCall()
{
	if (this == nullptr)
		return (false);
	if (nodetype == en_fcall)
		return (true);
	if (p[0]->HasCall())
		return (true);
	if (p[1]->HasCall())
		return (true);
	if (p[1]->HasCall())
		return (true);
	return (false);
}

long ENODE::GetReferenceSize()
{
	switch (nodetype)        /* get load size */
	{
	case en_b_ref:
	case en_ub_ref:
	case en_bfieldref:
	case en_ubfieldref:
		return (1);
	case en_c_ref:
	case en_uc_ref:
	case en_cfieldref:
	case en_ucfieldref:
		return (2);
	case en_ref32:
	case en_ref32u:
		return (4);
	case en_h_ref:
	case en_uh_ref:
	case en_hfieldref:
	case en_uhfieldref:
		return (sizeOfWord / 2);
	case en_w_ref:
	case en_uw_ref:
	case en_wfieldref:
	case en_uwfieldref:
		return (sizeOfWord);
	case en_fpregvar:
		if (tp)
			return(tp->size);
		else
			return (sizeOfFPD);
	case en_tempref:
	case en_regvar:
		return (sizeOfWord);
	case en_dbl_ref:
		return (sizeOfFPD);
	case en_quad_ref:
		return (sizeOfFPQ);
	case en_flt_ref:
		return (sizeOfFPD);
	case en_triple_ref:
		return (sizeOfFPT);
	case en_hp_ref:
		return (sizeOfPtr >> 1);
	case en_wp_ref:
		return (sizeOfPtr);
	case en_vector_ref:
		return (512);
		//			return node->esize;
	}
	return (8);
}
// return the natural evaluation size of a node.

int ENODE::GetNaturalSize()
{
	int siz0, siz1;
	if (this == NULL)
		return 0;
	switch (nodetype)
	{
	case en_uwfieldref:
	case en_wfieldref:
		return (sizeOfWord);
	case en_bfieldref:
	case en_ubfieldref:
		return (1);
	case en_cfieldref:
	case en_ucfieldref:
		return (2);
	case en_hfieldref:
	case en_uhfieldref:
		return (4);
	case en_icon:
		if (i >= -128 && i < 128)
			return (1);
		if (-32768 <= i && i <= 32767)
			return (2);
		if (-2147483648LL <= i && i <= 2147483647LL)
			return (4);
		return (8);
	case en_fcon:
		return (tp->precision / 16);
	case en_tcon: return (6);
	case en_labcon: case en_clabcon:
	case en_cnacon: case en_nacon:  case en_autocon: case en_classcon:
	case en_tempref:
	case en_cbw: case en_cubw:
	case en_ccw: case en_cucw:
	case en_chw: case en_cuhw:
	case en_cbu: case en_ccu: case en_chu:
	case en_cubu: case en_cucu: case en_cuhu:
	case en_ccwp: case en_cucwp:
	case en_sxb:	case en_sxc:	case en_sxh:
		return (8);
	case en_fcall:
	case en_regvar:
	case en_fpregvar:
		if (tp)
			return (tp->size);
		else
			return (8);
	case en_autofcon:
		return (8);
	case en_ref32: case en_ref32u:
		return (4);
	case en_b_ref:
	case en_ub_ref:
		return (1);
	case en_cbc:
	case en_c_ref:	return (2);
	case en_uc_ref:	return (2);
	case en_cbh:	return (4);
	case en_cch:	return (4);
	case en_h_ref:	return (4);
	case en_uh_ref:	return (4);
	case en_flt_ref: return (sizeOfFPS);
	case en_w_ref:  case en_uw_ref:
		return (8);
	case en_hp_ref:
		return (4);
	case en_wp_ref:
		return (8);
	case en_autovcon:
	case en_vector_ref:
		return (512);
	case en_dbl_ref:
		return (sizeOfFPD);
	case en_quad_ref:
		return (sizeOfFPQ);
	case en_triple_ref:
		return (sizeOfFPT);
	case en_tempfpref:
		if (tp)
			return (tp->precision / 16);
		else
			return (8);
	case en_not:    case en_compl:
	case en_uminus: case en_assign:
		return p[0]->GetNaturalSize();
	case en_fadd:	case en_fsub:
	case en_fmul:	case en_fdiv:
	case en_fsadd:	case en_fssub:
	case en_fsmul:	case en_fsdiv:
	case en_vadd:	case en_vsub:
	case en_vmul:	case en_vdiv:
	case en_vadds:	case en_vsubs:
	case en_vmuls:	case en_vdivs:
	case en_add:    case en_sub:
	case en_mul:    case en_mulu:
	case en_div:	case en_udiv:
	case en_mod:    case en_umod:
	case en_and:    case en_or:     case en_xor:
	case en_asl:
	case en_shl:    case en_shlu:
	case en_shr:	case en_shru:
	case en_asr:	case en_asrshu:
	case en_feq:    case en_fne:
	case en_flt:    case en_fle:
	case en_fgt:    case en_fge:
	case en_eq:     case en_ne:
	case en_lt:     case en_le:
	case en_gt:     case en_ge:
	case en_ult:	case en_ule:
	case en_ugt:	case en_uge:
	case en_land:   case en_lor:
	case en_land_safe:   case en_lor_safe:
	case en_asadd:  case en_assub:
	case en_asmul:  case en_asmulu:
	case en_asdiv:	case en_asdivu:
	case en_asmod:  case en_asmodu: case en_asand:
	case en_asor:   case en_asxor:	case en_aslsh:
	case en_asrsh:
		siz0 = p[0]->GetNaturalSize();
		siz1 = p[1]->GetNaturalSize();
		if (siz1 > siz0)
			return (siz1);
		else
			return (siz0);
	case en_void:   case en_cond:	case en_safe_cond:
		return (p[1]->GetNaturalSize());
	case en_bchk:
		return (p[0]->GetNaturalSize());
	case en_chk:
		return 8;
	case en_q2i:
	case en_t2i:
		return (sizeOfWord);
	case en_i2d:
		return (sizeOfWord);
	case en_i2t:
	case en_d2t:
		return (sizeOfFPT);
	case en_i2q:
	case en_d2q:
	case en_t2q:
		return (sizeOfFPQ);
	case en_object_list:
		return (p[0]->GetNaturalSize());
	case en_addrof:
		return (sizeOfPtr);
	default:
		printf("DIAG - natural size error %d.\n", nodetype);
		break;
	}
	return (0);
}


bool ENODE::IsBitfield()
{
	return (nodetype == en_wfieldref
		|| nodetype == en_bfieldref
		|| nodetype == en_cfieldref
		|| nodetype == en_hfieldref
		|| nodetype == en_ubfieldref
		|| nodetype == en_ucfieldref
		|| nodetype == en_uhfieldref
		|| nodetype == en_uwfieldref
		);
}

//
// equalnode will return 1 if the expressions pointed to by
// node1 and node2 are equivalent.
//
bool ENODE::IsEqual(ENODE *node1, ENODE *node2, bool lit)
{
	ENODE *ep1, *ep2;

	if (node1 == nullptr && node2 == nullptr)
		return (true);
	if (!lit) {
		if (node1 == nullptr || node2 == nullptr) {
			return (false);
		}
	}
	else {
		if (node1 == nullptr || node2 == nullptr)
			return (true);
	}
	if (node1->nodetype != node2->nodetype) {
		return (false);
	}
	switch (node1->nodetype) {
	case en_fcon:
		return (Float128::IsEqual(&node1->f128, &node2->f128));
		//			return (node1->f == node2->f);
	case en_regvar:
	case en_fpregvar:
	case en_tempref:
	case en_tempfpref:
	case en_icon:
	case en_labcon:
	case en_classcon:	// Check type ?
	case en_autocon:
	case en_autovcon:
	case en_autofcon:
	{
		return (node1->i == node2->i);
	}
	case en_nacon: {
		return (node1->sp->compare(*node2->sp) == 0);
	}
	case en_cnacon:
		return (node1->sp->compare(*node2->sp) == 0);
	case en_aggregate:
		return (IsEqual(node1->p[0], node2->p[0], lit));
	case en_list:
		for (ep1 = node1->p[2],
				 ep2 = node2->p[2];
				 ep1 && ep2; ep1 = ep1->p[2], ep2 = ep2->p[2]) {
			if (!IsEqual(ep1->p[0], ep2->p[0], lit))
				return (false);
			if (!IsEqual(ep1->p[1], ep2->p[1], lit))
				return (false);
		}
		return (true);
	default:
		if (!IsEqual(node1->p[0], node2->p[0], lit))
			return (false);
		if (!IsEqual(node1->p[1], node2->p[1], lit))
			return (false);
		if (!IsEqual(node1->p[2], node2->p[2], lit))
			return (false);
		return (true);
		//if (IsLValue(node1) && IsEqual(node1->p[0], node2->p[0])) {
		//	//	        if( equalnode(node1->p[0], node2->p[0])  )
		//	return (true);
		//}
	}
	return (false);
}


ENODE *ENODE::Clone()
{
	ENODE *temp;

	if (this == nullptr)
		return (ENODE *)nullptr;
	temp = allocEnode();
	memcpy(temp, this, sizeof(ENODE));	// copy all the fields
	p[0] = p[0]->Clone();
	p[1] = p[1]->Clone();
	p[2] = p[2]->Clone();
	return (temp);
}

// ============================================================================
// ============================================================================
// Parsing
// ============================================================================
// ============================================================================

// Assign a type to a whole list.

bool ENODE::AssignTypeToList(TYP *tp)
{
	ENODE *ep;
	bool isConst = true;
	TYP *btp;
	int ne, cnt;

	if (nodetype != en_aggregate)
		return (false);

	esize = 0;
	this->tp = tp;
	this->tp->isConst = isConst;
	this->esize = tp->size;
	if (tp->isArray) {
		ne = tp->numele;
		cnt = 0;
		btp = tp->GetBtp();
		for (ep = p[0]->p[2]; ep; ep = ep->p[2]) {
			cnt++;
			ep->tp = btp;
			ep->tp->isConst = isConst;
			ep->esize = btp->size;
			//if (!ep->tp->isConst)
			//	isConst = false;
			if (btp->isArray) {
				if (ep->nodetype == en_aggregate) {
					if (!ep->AssignTypeToList(btp))
						isConst = false;
				}
			}
			else if (btp->IsAggregateType()) {
				if (ep->nodetype == en_aggregate) {
					if (!ep->AssignTypeToList(btp))
						isConst = false;
				}
			}
		}
		//if (cnt < tp->numele) {
		//	esize += (tp->numele - cnt) * btp->size;
		//}
	}
	else if (tp->IsAggregateType()) {
		SYM *thead;

		thead = SYM::GetPtr(tp->lst.GetHead());
		while (thead && ep) {
			ep->tp = thead->tp;
			ep->tp->isConst = isConst;
			ep->esize = thead->tp->size;
			if (thead->tp->IsAggregateType()) {
				if (ep->nodetype == en_aggregate) {
					if (!ep->AssignTypeToList(thead->tp))
						isConst = false;
				}
			}
			ep = ep->p[2];
			thead = SYM::GetPtr(thead->next);
		}
	}
	return (isConst);
}


// ============================================================================
// ============================================================================
// Optimization
// ============================================================================
// ============================================================================

//	repexpr will replace all allocated references within an expression
//	with tempref nodes.

void ENODE::repexpr()
{
	CSE *csp;
	ENODE *ep;

	if (this == nullptr)
		return;
	switch (nodetype) {
	case en_fcon:
	case en_tempfpref:
		if ((csp = currentFn->csetbl->Search(this)) != nullptr) {
			csp->isfp = TRUE; //**** a kludge
			if (csp->reg > 0) {
				nodetype = en_fpregvar;
				rg = csp->reg;
			}
		}
		break;
	// Autofcon resolve to *pointers* which are stored in integer registers.
	case en_autofcon:
		if ((csp = currentFn->csetbl->Search(this)) != nullptr) {
			csp->isfp = FALSE; //**** a kludge
			if (csp->reg > 0) {
				nodetype = en_fpregvar;
				rg = csp->reg;
			}
		}
		break;
		/*
		if( (csp = SearchCSEList(node)) != NULL ) {
		if( csp->reg > 0 ) {
		node->nodetype = en_fpregvar;
		node->i = csp->reg;
		}
		}
		break;
		*/
	case en_icon:
	case en_nacon:
	case en_labcon:
	case en_autovcon:
	case en_autocon:
	case en_classcon:
	case en_cnacon:
	case en_clabcon:
	case en_tempref:
		if ((csp = currentFn->csetbl->Search(this)) != NULL) {
			if (csp->reg > 0) {
				nodetype = en_regvar;
				rg = csp->reg;
			}
		}
		break;
	case en_ref32: case en_ref32u:
	case en_b_ref:
	case en_c_ref:
	case en_h_ref:
	case en_w_ref:
	case en_ub_ref:
	case en_uc_ref:
	case en_uh_ref:
	case en_uw_ref:
	case en_wp_ref:
	case en_hp_ref:
	case en_bfieldref:
	case en_ubfieldref:
	case en_cfieldref:
	case en_ucfieldref:
	case en_hfieldref:
	case en_uhfieldref:
	case en_wfieldref:
	case en_uwfieldref:
	case en_vector_ref:
		if ((csp = currentFn->csetbl->Search(this)) != NULL) {
			if (csp->reg > 0) {
				nodetype = en_regvar;
				rg = csp->reg;
			}
			else
				p[0]->repexpr();
		}
		else
			p[0]->repexpr();
		break;
	case en_dbl_ref:
	case en_flt_ref:
	case en_quad_ref:
		if ((csp = currentFn->csetbl->Search(this)) != NULL) {
			if (csp->reg > 0) {
				nodetype = en_fpregvar;
				rg = csp->reg;
			}
			else
				p[0]->repexpr();
		}
		else
			p[0]->repexpr();
		break;
	case en_cbc: case en_cubw:
	case en_cbh: case en_cucw:
	case en_cbw: case en_cuhw:
	case en_cbu: case en_ccu: case en_chu:
	case en_cubu: case en_cucu: case en_cuhu:
	case en_ccwp: case en_cucwp:
	case en_cch:
	case en_ccw:
	case en_chw:
	case en_uminus:
	case en_abs:
	case en_sxb: case en_sxh: case en_sxc:
	case en_not:    case en_compl:
	case en_chk:
		p[0]->repexpr();
		break;
	case en_i2d:
		p[0]->repexpr();
		break;
	case en_i2q:
	case en_d2i:
	case en_q2i:
	case en_s2q:
	case en_d2q:
	case en_t2q:
		p[0]->repexpr();
		break;
	case en_add:    case en_sub:
	case en_mulf:   case en_mul:    case en_mulu:   case en_div:	case en_udiv:
	case en_mod:    case en_umod:
	case en_shl:	case en_asl:
	case en_shlu:	case en_shru:	case en_asr:
	case en_shr:
	case en_and:
	case en_or:     case en_xor:
	case en_land:   case en_lor:
	case en_land_safe:   case en_lor_safe:
	case en_eq:     case en_ne:
	case en_lt:     case en_le:
	case en_gt:     case en_ge:
	case en_ult:	case en_ule:
	case en_ugt:	case en_uge:

	case en_feq:    case en_fne:
	case en_flt:    case en_fle:
	case en_fgt:    case en_fge:
	case en_fdmul:  case en_fddiv:
	case en_fdadd:  case en_fdsub:
	case en_fadd: case en_fsub:
	case en_fmul: case en_fdiv:

	case en_veq:    case en_vne:
	case en_vlt:    case en_vle:
	case en_vgt:    case en_vge:
	case en_vadd: case en_vsub:
	case en_vmul: case en_vdiv:
	case en_vadds: case en_vsubs:
	case en_vmuls: case en_vdivs:

	case en_cond:   case en_void:	case en_safe_cond:
	case en_asadd:  case en_assub:
	case en_asmul:  case en_asmulu:
	case en_asdiv:  case en_asdivu:
	case en_asor:   case en_asand:    case en_asxor:
	case en_asmod:  case en_aslsh:
	case en_asrsh:  case en_fcall:
	case en_aggregate:
	case en_assign:
		p[0]->repexpr();
		p[1]->repexpr();
		break;
	case en_list:
		for (ep = p[2]; ep; ep = ep->p[2])
			ep->repexpr();
		break;
	case en_regvar:
	case en_fpregvar:
		break;
	case en_bchk:
		p[0]->repexpr();
		p[1]->repexpr();
		p[2]->repexpr();
		break;
	case en_addrof:
		p[0]->repexpr();
		break;
	default:
		dfs.printf("Uncoded node in repexr():%d\r\n", nodetype);
	}
}


/*
*      scanexpr will scan the expression pointed to by node for optimizable
*      subexpressions. when an optimizable expression is found it is entered
*      into the tree. if a reference to an autocon node is scanned the
*      corresponding auto dereferenced node will be voided. duse should be
*      set if the expression will be dereferenced.
*/
void ENODE::scanexpr(int duse)
{
	CSE *csp, *csp1;
	int first;
	int nn;
	ENODE *ep;

	if (this == nullptr)
		return;

	switch (nodetype) {
	case en_fpregvar:
	case en_regvar:
		break;
	case en_cnacon:
	case en_clabcon:
	case en_fcon:
	case en_icon:
	case en_labcon:
	case en_nacon:
		currentFn->csetbl->InsertNode(this, duse);
		break;
	case en_autofcon:
		csp1 = currentFn->csetbl->InsertNode(this, duse);
		csp1->isfp = FALSE;
		if ((nn = currentFn->csetbl->voidauto2(this)) > 0) {
			csp1->duses += loop_active;
			csp1->uses = csp1->duses + nn - loop_active;
		}
		break;
	case en_tempfpref:
		csp1 = currentFn->csetbl->InsertNode(this, duse);
		csp1->isfp = TRUE;
		if ((nn = currentFn->csetbl->voidauto2(this)) > 0) {
			csp1->duses += loop_active;
			csp1->uses = csp1->duses + nn - loop_active;
		}
		break;
	case en_autovcon:
	case en_autocon:
	case en_classcon:
	case en_tempref:
		csp1 = currentFn->csetbl->InsertNode(this, duse);
		if ((nn = currentFn->csetbl->voidauto2(this)) > 0) {
			csp1->duses += loop_active;
			csp1->uses = csp1->duses + nn - loop_active;
		}
		break;
	case en_ref32: case en_ref32u:
	case en_b_ref:
	case en_c_ref:
	case en_h_ref:
	case en_w_ref:
	case en_ub_ref:
	case en_uc_ref:
	case en_uh_ref:
	case en_uw_ref:
	case en_flt_ref:
	case en_dbl_ref:
	case en_quad_ref:
	case en_bfieldref:
	case en_ubfieldref:
	case en_cfieldref:
	case en_ucfieldref:
	case en_hfieldref:
	case en_uhfieldref:
	case en_wfieldref:
	case en_uwfieldref:
	case en_wp_ref:
	case en_hp_ref:
	case en_vector_ref:
		//if (p[0] == nullptr) break;
		// There is something wrong with the following code that causes
		// it to remove zero extension conversion from a byte to a word.
		if (p[0]->nodetype == en_autocon || p[0]->nodetype == en_autofcon
			|| p[0]->nodetype == en_classcon || p[0]->nodetype == en_autovcon) {
			first = (currentFn->csetbl->Search(this) == nullptr);	// Detect if this is the first insert
			csp = currentFn->csetbl->InsertNode(this, duse);
			if (csp->voidf)
				p[0]->scanexpr(1);
			// take care: the non-derereferenced use of the autocon node may
			// already be in the list. In this case, set voidf to 1
			if (currentFn->csetbl->Search(p[0]) != NULL) {
				csp->voidf = 1;
				p[0]->scanexpr(1);
			}
			else {
				//                        if( csp->voidf )
				//                             scanexpr(node->p[0],1);
				if (first) {
					///* look for register nodes */
					//int i = 0;
					//long j = node->p[0]->i;
					//if ((node->p[0]->nodetype== en_regvar || node->p[0]->nodetype==en_bregvar) &&
					//	(j >= 11 && j < 18))
					//{
					//	csp->voidf--;	/* this is not in auto_lst */
					//	//csp->uses += 90 * (100 - i);
					//	//csp->duses += 30 * (100 - i);
					//	break;
					//}
					///* set voidf if the node is not in autolst */
					//csp->voidf++;
					//i = 0;
					//while (i < autoptr) {
					//	if (autolst[i] == j) {
					//		csp->voidf--;
					//		break;
					//	}
					//	++i;
					//}
					/*
					* even if that item must not be put in a register,
					* it is legal to put its address therein
					*/
					//if (csp->voidf)
					//	scanexpr(node->p[0], 1);
					//}

					//if( csp->voidf )
					//    scanexpr(node->p[0],1);
				}
			}
		}
		else
			p[0]->scanexpr(1);
		break;
	case en_cbc: case en_cubw:
	case en_cbh: case en_cucw:
	case en_cbw: case en_cuhw:
	case en_cbu: case en_ccu: case en_chu:
	case en_cubu: case en_cucu: case en_cuhu:
	case en_ccwp: case en_cucwp:
	case en_cch:
	case en_ccw:
	case en_chw:
	case en_uminus:
	case en_abs:
	case en_sxb: case en_sxc: case en_sxh:
	case en_zxb: case en_zxc: case en_zxh:
	case en_compl:
	case en_not:
	case en_chk:
		p[0]->scanexpr(duse);
		break;
	case en_i2d:
		p[0]->scanexpr(duse);
		break;
	case en_i2q:
	case en_d2i:
	case en_q2i:
	case en_s2q:
	case en_d2q:
	case en_t2q:
		p[0]->scanexpr(duse);
		break;
	case en_asadd:  case en_assub:
	case en_add:    case en_sub:
		p[0]->scanexpr(duse);
		p[1]->scanexpr(duse);
		break;
	case en_mulf:
	case en_mul:    case en_mulu:   case en_div:	case en_udiv:
	case en_shl:    case en_asl:	case en_shlu:	case en_shr:	case en_shru:	case en_asr:
	case en_mod:    case en_umod:   case en_and:
	case en_or:     case en_xor:
	case en_lor:    case en_land:
	case en_lor_safe:    case en_land_safe:
	case en_eq:     case en_ne:
	case en_gt:     case en_ge:
	case en_lt:     case en_le:
	case en_ugt:    case en_uge:
	case en_ult:    case en_ule:
	case en_feq:    case en_fne:
	case en_flt:    case en_fle:
	case en_fgt:    case en_fge:
	case en_fdmul:  case en_fddiv:
	case en_fdadd:  case en_fdsub:
	case en_fadd: case en_fsub:
	case en_fmul: case en_fdiv:

	case en_veq:    case en_vne:
	case en_vlt:    case en_vle:
	case en_vgt:    case en_vge:
	case en_vadd: case en_vsub:
	case en_vmul: case en_vdiv:
	case en_vadds: case en_vsubs:
	case en_vmuls: case en_vdivs:

	case en_asmul:  case en_asmulu:
	case en_asdiv:	case en_asdivu:
	case en_asmod:  case en_aslsh:
	case en_asrsh:
	case en_asand:	case en_asxor: case en_asor:
	case en_cond:	case en_safe_cond:
	case en_void:
	case en_aggregate:
		p[0]->scanexpr(0);
		p[1]->scanexpr(0);
		break;
	case en_list:
		for (ep = p[2]; ep; ep = ep->p[2])
			ep->scanexpr(0);
		break;
	case en_assign:
		p[0]->scanexpr(0);
		p[1]->scanexpr(0);
		break;
	case en_fcall:
		p[0]->scanexpr(1);
		p[1]->scanexpr(0);
		break;
	case en_bchk:
		p[0]->scanexpr(0);
		p[1]->scanexpr(0);
		p[2]->scanexpr(0);
		break;
	case en_addrof:
		p[0]->scanexpr(0);
		break;
	default: dfs.printf("Uncoded node in ENODE::scanexpr():%d\r\n", nodetype);
	}
}


// ============================================================================
// ============================================================================
// Code Generation
// ============================================================================
// ============================================================================


// ----------------------------------------------------------------------------
// Generate code to evaluate an index node (^+) and return the addressing mode
// of the result. This routine takes no flags since it always returns either
// am_ind or am_indx.
//
// No reason to ReleaseTempReg() because the registers used are transported
// forward.
// ----------------------------------------------------------------------------
Operand *ENODE::GenIndex()
{
	Operand *ap1, *ap2;

	if ((p[0]->nodetype == en_tempref || p[0]->nodetype == en_regvar)
		&& (p[1]->nodetype == en_tempref || p[1]->nodetype == en_regvar))
	{       /* both nodes are registers */
			// Don't need to free ap2 here. It is included in ap1.
		GenerateHint(8);
		ap1 = cg.GenerateExpression(p[0], F_REG, 8);
		ap2 = cg.GenerateExpression(p[1], F_REG, 8);
		GenerateHint(9);
		ap1->mode = am_indx2;
		ap1->sreg = ap2->preg;
		ap1->deep2 = ap2->deep2;
		ap1->offset = makeinode(en_icon, 0);
		ap1->scale = scale;
		return (ap1);
	}
	GenerateHint(8);
	ap1 = cg.GenerateExpression(p[0], F_REG | F_IMMED, 8);
	if (ap1->mode == am_imm)
	{
		ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, 8);
		if (ap2->mode == am_reg && ap2->preg==0) {	// value is zero
			ap1->mode = am_direct;
			return (ap1);
		}
		ap2->isConst = ap2->mode==am_imm;
		if (ap2->mode == am_imm && ap2->offset->i != 0)
			ap2->offset2 = ap2->offset;
		else
			ap2->offset2 = nullptr;
		GenerateHint(9);
		ap2->mode = am_indx;
		ap2->offset = ap1->offset;
		ap2->isUnsigned = ap1->isUnsigned;
		return (ap2);
	}
	ap2 = cg.GenerateExpression(p[1], F_ALL, 8);   /* get right op */
	GenerateHint(9);
	if (ap2->mode == am_imm && ap1->mode == am_reg) /* make am_indx */
	{
		ap2->mode = am_indx;
		ap2->preg = ap1->preg;
		ap2->deep = ap1->deep;
		return (ap2);
	}
	if (ap2->mode == am_ind && ap1->mode == am_reg) {
		ap2->mode = am_indx2;
		ap2->sreg = ap1->preg;
		ap2->deep2 = ap1->deep;
		return ap2;
	}
	if (ap2->mode == am_direct && ap1->mode == am_reg) {
		ap2->mode = am_indx;
		ap2->preg = ap1->preg;
		ap2->deep = ap1->deep;
		return ap2;
	}
	// ap1->mode must be F_REG
	ap2->MakeLegal( F_REG, 8);
	ap1->mode = am_indx2;            /* make indexed */
	ap1->sreg = ap2->preg;
	ap1->deep2 = ap2->deep;
	ap1->offset = makeinode(en_icon, 0);
	ap1->scale = scale;
	return ap1;                     /* return indexed */
}


//
// Generate code to evaluate a condition operator node (??:)
//
Operand *ENODE::GenSafeHook(int flags, int size)
{
	Operand *ap1, *ap2, *ap3, *ap4;
	int false_label, end_label;
	OCODE *ip1;
	int n1;
	ENODE *node;

	false_label = nextlabel++;
	end_label = nextlabel++;
	//flags = (flags & F_REG) | F_VOL;
	flags |= F_VOL;
	/*
	if (p[0]->constflag && p[1]->constflag) {
	GeneratePredicateMonadic(hook_predreg,op_op_ldi,make_immed(p[0]->i));
	GeneratePredicateMonadic(hook_predreg,op_ldi,make_immed(p[0]->i));
	}
	*/
	ip1 = currentFn->pl.tail;
	// cmovenz integer only
	if (!opt_nocgo) {
		ap4 = GetTempRegister();
		ap1 = cg.GenerateExpression(p[0], F_REG, size);
		ap2 = cg.GenerateExpression(p[1]->p[0], F_REG | F_FPREG, size);
		ap3 = cg.GenerateExpression(p[1]->p[1], F_REG | F_FPREG | F_IMMED, size);
		if (ap2->mode == am_fpreg || ap3->mode == am_fpreg)
			goto j1;
		n1 = currentFn->pl.Count(ip1);
		if (n1 < 20 && !currentFn->pl.HasCall(ip1)) {
			Generate4adic(op_cmovenz, 0, ap4, ap1, ap2, ap3);
			ReleaseTempReg(ap3);
			ReleaseTempReg(ap2);
			ReleaseTempReg(ap1);
			ap4->MakeLegal(flags,size);
			return (ap4);
		}
j1:
		ReleaseTempReg(ap3);
		ReleaseTempReg(ap2);
		ReleaseTempReg(ap1);
		ReleaseTempReg(ap4);
		currentFn->pl.tail = ip1;
		currentFn->pl.tail->fwd = nullptr;
	}
	ap2 = cg.GenerateExpression(p[1]->p[1], flags, size);
	n1 = currentFn->pl.Count(ip1);
	if (opt_nocgo)
		n1 = 9999;
	if (n1 > 4 || currentFn->pl.HasCall(ip1))
	{
		ReleaseTempReg(ap2);
		currentFn->pl.tail = ip1;
		currentFn->pl.tail->fwd = nullptr;
		cg.GenerateFalseJump(p[0], false_label, 0);
		node = p[1];
		ap1 = cg.GenerateExpression(node->p[0], flags, size);
		GenerateDiadic(op_bra, 0, make_clabel(end_label), 0);
		GenerateLabel(false_label);
		ap2 = cg.GenerateExpression(node->p[1], flags, size);
		if (!IsEqualOperand(ap1, ap2))
		{
			GenerateMonadic(op_hint, 0, make_immed(2));
			switch (ap1->mode)
			{
			case am_reg:
				switch (ap2->mode) {
				case am_reg:
					GenerateDiadic(op_mov, 0, ap1, ap2);
					break;
				case am_imm:
					GenerateDiadic(op_ldi, 0, ap1, ap2);
					if (ap2->isPtr)
						ap1->isPtr = true;
					break;
				default:
					GenLoad(ap1, ap2, size, size);
					break;
				}
				break;
			case am_fpreg:
				switch (ap2->mode) {
				case am_fpreg:
					GenerateDiadic(op_mov, 0, ap1, ap2);
					break;
				case am_imm:
					ap4 = GetTempRegister();
					GenerateDiadic(op_ldi, 0, ap4, ap2);
					GenerateDiadic(op_mov, 0, ap1, ap4);
					if (ap2->isPtr)
						ap1->isPtr = true;
					break;
				default:
					GenLoad(ap1, ap2, size, size);
					break;
				}
				break;
			case am_imm:
				break;
			default:
				GenStore(ap2, ap1, size);
				break;
			}
		}
		GenerateLabel(end_label);
		ap1->MakeLegal(flags, size);
		return (ap1);
	}
	// N1 <= 4
	cg.GenerateFalseJump(p[0], false_label, 0);
	node = p[1];
	ap1 = cg.GenerateExpression(node->p[0], flags, size);
	GenerateDiadic(op_bra, 0, make_clabel(end_label), 0);
	GenerateLabel(false_label);
	if (!IsEqualOperand(ap1, ap2))
	{
		GenerateMonadic(op_hint, 0, make_immed(2));
		switch (ap1->mode)
		{
		case am_reg:
			switch (ap2->mode) {
			case am_reg:
				GenerateDiadic(op_mov, 0, ap1, ap2);
				break;
			case am_imm:
				GenerateDiadic(op_ldi, 0, ap1, ap2);
				if (ap2->isPtr)
					ap1->isPtr = true;
				break;
			default:
				GenLoad(ap1, ap2, size, size);
				break;
			}
			break;
		case am_fpreg:
			switch (ap2->mode) {
			case am_fpreg:
				GenerateDiadic(op_mov, 0, ap1, ap2);
				break;
			case am_imm:
				ap4 = GetTempRegister();
				GenerateDiadic(op_ldi, 0, ap4, ap2);
				GenerateDiadic(op_mov, 0, ap1, ap4);
				if (ap2->isPtr)
					ap1->isPtr = true;
				break;
			default:
				GenLoad(ap1, ap2, size, size);
				break;
			}
			break;
		case am_imm:
			break;
		default:
			GenStore(ap2, ap1, size);
			break;
		}
	}
	GenerateLabel(end_label);
	ap1->MakeLegal(flags, size);
	return (ap1);
}

//
// Generate code to evaluate a condition operator node (?:)
//
Operand *ENODE::GenHook(int flags, int size)
{
	Operand *ap1, *ap2, *ap3, *ap4;
	int false_label, end_label;
	OCODE *ip1;
	int n1;
	ENODE *node;

	false_label = nextlabel++;
	end_label = nextlabel++;
	//flags = (flags & F_REG) | F_VOL;
	flags |= F_VOL;
	/*
	if (p[0]->constflag && p[1]->constflag) {
	GeneratePredicateMonadic(hook_predreg,op_op_ldi,make_immed(p[0]->i));
	GeneratePredicateMonadic(hook_predreg,op_ldi,make_immed(p[0]->i));
	}
	*/
	ip1 = currentFn->pl.tail;
	ap2 = cg.GenerateExpression(p[1]->p[1], flags, size);
	n1 = currentFn->pl.Count(ip1);
	if (opt_nocgo)
		n1 = 9999;
	ReleaseTempReg(ap2);
	currentFn->pl.tail = ip1;
	currentFn->pl.tail->fwd = nullptr;
	cg.GenerateFalseJump(p[0], false_label, 0);
	node = p[1];
	ap1 = cg.GenerateExpression(node->p[0], flags, size);
	GenerateDiadic(op_bra, 0, make_clabel(end_label), 0);
	GenerateLabel(false_label);
	ap2 = cg.GenerateExpression(node->p[1], flags, size);
	if (!IsEqualOperand(ap1, ap2))
	{
		GenerateMonadic(op_hint, 0, make_immed(2));
		switch (ap1->mode)
		{
		case am_reg:
			switch (ap2->mode) {
			case am_reg:
				GenerateDiadic(op_mov, 0, ap1, ap2);
				break;
			case am_imm:
				GenerateDiadic(op_ldi, 0, ap1, ap2);
				if (ap2->isPtr)
					ap1->isPtr = true;
				break;
			default:
				GenLoad(ap1, ap2, size, size);
				break;
			}
			break;
		case am_fpreg:
			switch (ap2->mode) {
			case am_fpreg:
				GenerateDiadic(op_mov, 0, ap1, ap2);
				break;
			case am_imm:
				ap4 = GetTempRegister();
				GenerateDiadic(op_ldi, 0, ap4, ap2);
				GenerateDiadic(op_mov, 0, ap1, ap4);
				if (ap2->isPtr)
					ap1->isPtr = true;
				break;
			default:
				GenLoad(ap1, ap2, size, size);
				break;
			}
			break;
		case am_imm:
			break;
		default:
			GenStore(ap2, ap1, size);
			break;
		}
	}
	GenerateLabel(end_label);
	ap1->MakeLegal(flags, size);
	return (ap1);
}

Operand *ENODE::GenShift(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3;

	ap3 = GetTempRegister();
	ap1 = cg.GenerateExpression(p[0], F_REG, size);
	ap2 = cg.GenerateExpression(p[1], F_REG | F_IMM6, 8);
	GenerateTriadic(op, size, ap3, ap1, ap2);
	// Rotates automatically sign extend
	if ((op == op_rol || op == op_ror) && ap2->isUnsigned)
		switch (size) {
		case 1:	GenerateDiadic(op_zxb, 0, ap3, ap3); break;
		case 2:	GenerateDiadic(op_zxc, 0, ap3, ap3); break;
		case 4:	GenerateDiadic(op_zxh, 0, ap3, ap3); break;
		default:;
		}
	ReleaseTempRegister(ap2);
	ReleaseTempRegister(ap1);
	ap3->MakeLegal(flags, size);
	return (ap3);
}


Operand *ENODE::GenAssignShift(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3;
	MachineReg *mr;

	//size = GetNaturalSize(node->p[0]);
	ap3 = cg.GenerateExpression(p[0], F_ALL & ~F_IMMED, size);
	ap2 = cg.GenerateExpression(p[1], F_REG | F_IMM6, size);
	if (ap3->mode == am_reg) {
		GenerateTriadic(op, size, ap3, ap3, ap2);
		mr = &regs[ap3->preg];
		if (mr->assigned)
			mr->modified = true;
		mr->assigned = true;
		mr->isConst = ap3->isConst && ap2->isConst;
		ReleaseTempRegister(ap2);
		ap3->MakeLegal(flags, size);
		return (ap3);
	}
	ap1 = GetTempRegister();
	GenLoad(ap1, ap3, size, size);
	GenerateTriadic(op, size, ap1, ap1, ap2);
	GenStore(ap1, ap3, size);
	ReleaseTempRegister(ap1);
	ReleaseTempRegister(ap2);
	ap3->MakeLegal(flags, size);
	return (ap3);
}


//
//      generate code to evaluate a mod operator or a divide
//      operator.
//
Operand *ENODE::GenDivMod(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3;

	//if( node->p[0]->nodetype == en_icon ) //???
	//	swap_nodes(node);
	if (op == op_fdiv) {
		ap3 = GetTempFPRegister();
		ap1 = cg.GenerateExpression(p[0], F_FPREG, 8);
		ap2 = cg.GenerateExpression(p[1], F_FPREG, 8);
	}
	else {
		ap3 = GetTempRegister();
		ap1 = cg.GenerateExpression(p[0], F_REG, 8);
		if (op == op_modu)	// modu only supports register mode
			ap2 = cg.GenerateExpression(p[1], F_REG, 8);
		else
			ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, 8);
	}
	if (op == op_fdiv) {
		// Generate a convert operation ?
		if (ap1->fpsize() != ap2->fpsize()) {
			if (ap2->fpsize() == 's')
				GenerateDiadic(op_fcvtsq, 0, ap2, ap2);
		}
		GenerateTriadic(op, ap1->fpsize(), ap3, ap1, ap2);
	}
	else
		GenerateTriadic(op, 0, ap3, ap1, ap2);
	//    GenerateDiadic(op_ext,0,ap3,0);
	ap3->MakeLegal( flags, 2);
	ReleaseTempReg(ap2);
	ReleaseTempReg(ap1);
	return (ap3);
}


//
//      generate code to evaluate a multiply node.
//
Operand *ENODE::GenMultiply(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3;
	bool square = false;

	//Enter("Genmul");
	if (p[0]->nodetype == en_icon)
		swap_nodes(this);
	if (IsEqual(p[0], p[1]))
		square = !opt_nocgo;
	if (op == op_fmul) {
		ap3 = GetTempFPRegister();
		ap1 = cg.GenerateExpression(p[0], F_FPREG, size);
		if (!square)
			ap2 = cg.GenerateExpression(p[1], F_FPREG, size);
	}
	else {
		ap3 = GetTempRegister();
		ap1 = cg.GenerateExpression(p[0], F_REG, 8);
		if (!square)
			ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, 8);
	}
	if (op == op_fmul) {
		// Generate a convert operation ?
		if (ap1->fpsize() != ap2->fpsize()) {
			if (ap2->fpsize() == 's')
				GenerateDiadic(op_fcvtsq, 0, ap2, ap2);
		}
		if (square)
			GenerateTriadic(op, ap1->fpsize(), ap3, ap1, ap1);
		else
			GenerateTriadic(op, ap1->fpsize(), ap3, ap1, ap2);
	}
	else {
		if (square)
			GenerateTriadic(op, 0, ap3, ap1, ap1);
		else
			GenerateTriadic(op, 0, ap3, ap1, ap2);
	}
	if (!square)
		ReleaseTempReg(ap2);
	ReleaseTempReg(ap1);
	ap3->MakeLegal(flags, 2);
	//Leave("Genmul", 0);
	return (ap3);
}


//
// Generate code to evaluate a unary minus or complement.
//
Operand *ENODE::GenUnary(int flags, int size, int op)
{
	Operand *ap, *ap2;

	if (IsFloatType()) {
		ap2 = GetTempFPRegister();
		ap = cg.GenerateExpression(p[0], F_FPREG, size);
		if (op == op_neg)
			op = op_fneg;
		GenerateDiadic(op, fsize(), ap2, ap);
	}
	else if (etype == bt_vector) {
		ap2 = GetTempVectorRegister();
		ap = cg.GenerateExpression(p[0], F_VREG, size);
		GenerateDiadic(op, 0, ap2, ap);
	}
	else {
		ap2 = GetTempRegister();
		ap = cg.GenerateExpression(p[0], F_REG, size);
		GenerateHint(3);
		GenerateDiadic(op, 0, ap2, ap);
	}
	ReleaseTempReg(ap);
	ap2->MakeLegal(flags, size);
	return (ap2);
}

// Generate code for a binary expression

Operand *ENODE::GenBinary(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3, *ap4;
	bool dup = false;
	int flags2;

	if (IsFloatType())
	{
		ap3 = GetTempFPRegister();
		if (IsEqual(p[0], p[1]))
			dup = !opt_nocgo;
		ap1 = cg.GenerateExpression(p[0], F_FPREG, size);
		if (!dup)
			ap2 = cg.GenerateExpression(p[1], F_FPREG, size);
		// Generate a convert operation ?
		if (!dup) {
			if (ap1->fpsize() != ap2->fpsize()) {
				if (ap2->fpsize() == 's')
					GenerateDiadic(op_fcvtsq, 0, ap2, ap2);
			}
		}
		if (dup)
			GenerateTriadic(op, ap1->fpsize(), ap3, ap1, ap1);
		else
			GenerateTriadic(op, ap1->fpsize(), ap3, ap1, ap2);
		ap3->type = ap1->type;
	}
	else if (op == op_vadd || op == op_vsub || op == op_vmul || op == op_vdiv
		|| op == op_vadds || op == op_vsubs || op == op_vmuls || op == op_vdivs
		|| op == op_veins) {
		ap3 = GetTempVectorRegister();
		if (ENODE::IsEqual(p[0], p[1]) && !opt_nocgo) {
			ap1 = cg.GenerateExpression(p[0], F_VREG, size);
			ap2 = cg.GenerateExpression(vmask, F_VMREG, size);
			Generate4adic(op, 0, ap3, ap1, ap1, ap2);
			ReleaseTempReg(ap2);
			ap2 = nullptr;
		}
		else {
			ap1 = cg.GenerateExpression(p[0], F_VREG, size);
			ap2 = cg.GenerateExpression(p[1], F_VREG, size);
			ap4 = cg.GenerateExpression(vmask, F_VMREG, size);
			Generate4adic(op, 0, ap3, ap1, ap2, ap4);
			ReleaseTempReg(ap4);
		}
		// Generate a convert operation ?
		//if (fpsize(ap1) != fpsize(ap2)) {
		//	if (fpsize(ap2)=='s')
		//		GenerateDiadic(op_fcvtsq, 0, ap2, ap2);
		//}
	}
	else if (op == op_vex) {
		ap3 = GetTempRegister();
		ap1 = cg.GenerateExpression(p[0], F_REG, size);
		ap2 = cg.GenerateExpression(p[1], F_REG, size);
		GenerateTriadic(op, 0, ap3, ap1, ap2);
	}
	else {
		ap3 = GetTempRegister();
		if (ENODE::IsEqual(p[0], p[1]) && !opt_nocgo) {
			ap1 = cg.GenerateExpression(p[0], F_REG, size);
			ap2 = nullptr;
			GenerateTriadic(op, 0, ap3, ap1, ap1);
		}
		else {
			ap1 = cg.GenerateExpression(p[0], F_REG, size);
			// modu does not have an immediate mode
			ap2 = cg.GenerateExpression(p[1], op==op_modu ? F_REG: F_REG | F_IMMED, size);
			if (ap2->mode == am_imm) {
				switch (op) {
				case op_and:
					GenerateTriadic(op, 0, ap3, ap1, make_immed(ap2->offset->i));
					/*
					if (ap2->offset->i & 0xFFFF0000LL)
					GenerateDiadic(op_andq1,0,ap3,make_immed((ap2->offset->i >> 16) & 0xFFFFLL));
					if (ap2->offset->i & 0xFFFF00000000LL)
					GenerateDiadic(op_andq2,0,ap3,make_immed((ap2->offset->i >> 32) & 0xFFFFLL));
					if (ap2->offset->i & 0xFFFF000000000000LL)
					GenerateDiadic(op_andq3,0,ap3,make_immed((ap2->offset->i >> 48) & 0xFFFFLL));
					*/
					break;
				case op_or:
					GenerateTriadic(op, 0, ap3, ap1, make_immed(ap2->offset->i));
					/*
					if (ap2->offset->i & 0xFFFF0000LL)
					GenerateDiadic(op_orq1,0,ap3,make_immed((ap2->offset->i >> 16) & 0xFFFFLL));
					if (ap2->offset->i & 0xFFFF00000000LL)
					GenerateDiadic(op_orq2,0,ap3,make_immed((ap2->offset->i >> 32) & 0xFFFFLL));
					if (ap2->offset->i & 0xFFFF000000000000LL)
					GenerateDiadic(op_orq3,0,ap3,make_immed((ap2->offset->i >> 48) & 0xFFFFLL));
					*/
					break;
					// Most ops handle a max 16 bit immediate operand. If the operand is over 16 bits
					// it has to be loaded into a register.
				default:
					//if (ap2->offset->i < -32768LL || ap2->offset->i > 32767LL) {
						//ap4 = GetTempRegister();
						//GenerateTriadic(op_or, 0, ap4, makereg(regZero), make_immed(ap2->offset->i));
						/*
						if (ap2->offset->i & 0xFFFF0000LL)
						GenerateDiadic(op_orq1,0,ap4,make_immed((ap2->offset->i >> 16) & 0xFFFFLL));
						if (ap2->offset->i & 0xFFFF00000000LL)
						GenerateDiadic(op_orq2,0,ap4,make_immed((ap2->offset->i >> 32) & 0xFFFFLL));
						if (ap2->offset->i & 0xFFFF000000000000LL)
						GenerateDiadic(op_orq3,0,ap4,make_immed((ap2->offset->i >> 48) & 0xFFFFLL));
						*/
						//GenerateTriadic(op, 0, ap3, ap1, ap4);
						//ReleaseTempReg(ap4);
					//}
					//else
						GenerateTriadic(op, 0, ap3, ap1, ap2);
				}
			}
			else
				GenerateTriadic(op, 0, ap3, ap1, ap2);
		}
	}
	if (!dup)
		if (ap2)
			ReleaseTempReg(ap2);
	ReleaseTempReg(ap1);
	ap3->MakeLegal( flags, size);
	return (ap3);
}


Operand *ENODE::GenAssignAdd(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3;
	int ssize;
	bool negf = false;
	bool intreg = false;
	MachineReg *mr;

	ssize = p[0]->GetNaturalSize();
	if (ssize > size)
		size = ssize;
	if (p[0]->IsBitfield()) {
		ap3 = GetTempRegister();
		ap1 = cg.GenerateBitfieldDereference(p[0], F_REG | F_MEM, size);
		GenerateDiadic(op_mov, 0, ap3, ap1);
		ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, size);
		GenerateTriadic(op, 0, ap1, ap1, ap2);
		cg.GenerateBitfieldInsert(ap3, ap1, ap1->offset->bit_offset, ap1->offset->bit_width);
		GenStore(ap3, ap1->next, ssize);
		ReleaseTempReg(ap2);
		ReleaseTempReg(ap1->next);
		ReleaseTempReg(ap1);
		ap3->MakeLegal( flags, size);
		return (ap3);
	}
	if (IsFloatType()) {
		ap1 = cg.GenerateExpression(p[0], F_FPREG | F_MEM, ssize);
		ap2 = cg.GenerateExpression(p[1], F_FPREG, size);
		if (op == op_add)
			op = op_fadd;
		else if (op == op_sub)
			op = op_fsub;
	}
	else if (etype == bt_vector) {
		ap1 = cg.GenerateExpression(p[0], F_REG | F_MEM, ssize);
		ap2 = cg.GenerateExpression(p[1], F_REG, size);
		if (op == op_add)
			op = op_vadd;
		else if (op == op_sub)
			op = op_vsub;
	}
	else {
		ap1 = cg.GenerateExpression(p[0], F_ALL, ssize);
		ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, size);
		intreg = true;
	}
	if (ap1->mode == am_reg) {
		GenerateTriadic(op, 0, ap1, ap1, ap2);
		if (intreg) {
			mr = &regs[ap1->preg];
			if (mr->assigned)
				mr->modified = true;
			mr->assigned = true;
			mr->isConst = ap1->isConst && ap2->isConst;
		}
	}
	else if (ap1->mode == am_fpreg) {
		GenerateTriadic(op, ap1->fpsize(), ap1, ap1, ap2);
		ReleaseTempReg(ap2);
		ap1->MakeLegal( flags, size);
		return (ap1);
	}
	else {
		GenMemop(op, ap1, ap2, ssize);
	}
	ReleaseTempReg(ap2);
	//if (ap1->type != stddouble.GetIndex() && !ap1->isUnsigned)
	//	ap1 = ap1->GenSignExtend(ssize, size, flags);
	ap1->MakeLegal( flags, size);
	return (ap1);
}

Operand *ENODE::GenAssignLogic(int flags, int size, int op)
{
	Operand *ap1, *ap2, *ap3;
	int ssize;
	MachineReg *mr;

	ssize = p[0]->GetNaturalSize();
	if (ssize > size)
		size = ssize;
	if (p[0]->IsBitfield()) {
		ap3 = GetTempRegister();
		ap1 = cg.GenerateBitfieldDereference(p[0], F_REG | F_MEM, size);
		GenerateDiadic(op_mov, 0, ap3, ap1);
		ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, size);
		GenerateTriadic(op, 0, ap1, ap1, ap2);
		cg.GenerateBitfieldInsert(ap3, ap1, ap1->offset->bit_offset, ap1->offset->bit_width);
		GenStore(ap3, ap1->next, ssize);
		ReleaseTempReg(ap2);
		ReleaseTempReg(ap1->next);
		ReleaseTempReg(ap1);
		ap3->MakeLegal( flags, size);
		return (ap3);
	}
	ap1 = cg.GenerateExpression(p[0], F_ALL & ~F_FPREG, ssize);
	ap2 = cg.GenerateExpression(p[1], F_REG | F_IMMED, size);
	if (ap1->mode == am_reg) {
		GenerateTriadic(op, 0, ap1, ap1, ap2);
		mr = &regs[ap1->preg];
		if (mr->assigned)
			mr->modified = true;
		mr->assigned = true;
		mr->isConst = ap1->isConst && ap2->isConst;
	}
	else {
		GenMemop(op, ap1, ap2, ssize);
	}
	ReleaseTempRegister(ap2);
	if (!ap1->isUnsigned && !(flags & F_NOVALUE)) {
		if (size > ssize) {
			if (ap1->mode != am_reg) {
				ap1->MakeLegal( F_REG, ssize);
			}
			switch (ssize) {
			case 1:	GenerateDiadic(op_sxb, 0, ap1, ap1); break;
			case 2:	GenerateDiadic(op_sxc, 0, ap1, ap1); break;
			case 4:	GenerateDiadic(op_sxh, 0, ap1, ap1); break;
			}
			ap1->MakeLegal( flags, size);
			return (ap1);
		}
		ap1 = ap1->GenSignExtend(ssize, size, flags);
	}
	ap1->MakeLegal( flags, size);
	return (ap1);
}

Operand *ENODE::GenLand(int flags, int op, bool safe)
{
	Operand *ap1, *ap2, *ap3, *ap4, *ap5;
	int lab0, lab1;

	if (safe) {
		lab0 = nextlabel++;
		ap3 = GetTempRegister();
		ap1 = cg.GenerateExpression(p[0], F_REG, 8);
		ap4 = GetTempRegister();
		//if (op == op_and) {
		//	GenerateTriadic(op_beq, 0, ap1, makereg(0), make_label(lab0));
		//	ap2 = cg.GenerateExpression(p[1], F_REG, 8);
		//}
		if (!ap1->isBool)
			GenerateDiadic(op_redor, 0, ap4, ap1);
		else {
			ReleaseTempReg(ap4);
			ap4 = ap1;
		}
		ap2 = cg.GenerateExpression(p[1], F_REG, 8);
		ap5 = GetTempRegister();
		if (!ap2->isBool)
			GenerateDiadic(op_redor, 0, ap5, ap2);
		else {
			ReleaseTempReg(ap5);
			ap5 = ap2;
		}
		GenerateTriadic(op, 0, ap3, ap4, ap5);
		ReleaseTempReg(ap5);
		if (ap5 != ap2)
			ReleaseTempReg(ap2);
		if (ap4 != ap1)
			ReleaseTempReg(ap4);
		ReleaseTempReg(ap1);
		ap3->MakeLegal(flags, 8);
		ap3->isBool = true;
		return (ap3);
	}
	lab0 = nextlabel++;
	lab1 = nextlabel++;
	ap1 = GetTempRegister();
	GenerateDiadic(op_ldi, 0, ap1, make_immed(1));
	cg.GenerateFalseJump(this, lab0, 0);
	GenerateDiadic(op_ldi, 0, ap1, make_immed(0));
	GenerateLabel(lab0);
	ap1->MakeLegal(flags, 8);
	ap1->isBool = true;
	return (ap1);
}

// return the natural evaluation size of a node.

int GetNaturalSize(ENODE *node)
{ 
	int siz0, siz1;
	if( node == NULL )
		return 0;
	switch( node->nodetype )
	{
	case en_uwfieldref:
	case en_wfieldref:
		return (sizeOfWord);
	case en_bfieldref:
	case en_ubfieldref:
		return (1);
	case en_cfieldref:
	case en_ucfieldref:
		return (2);
	case en_hfieldref:
	case en_uhfieldref:
		return (4);
	case en_icon:
		if (node->i >= -128 && node->i < 128)
			return (1);
		if( -32768 <= node->i && node->i <= 32767 )
			return (2);
		if (-2147483648LL <= node->i && node->i <= 2147483647LL)
			return (4);
		return (8);
	case en_fcon:
		return (node->tp->precision / 16);
	case en_tcon: return (6);
	case en_labcon: case en_clabcon:
	case en_cnacon: case en_nacon:  case en_autocon: case en_classcon:
	case en_tempref:
	case en_cbw: case en_cubw:
	case en_ccw: case en_cucw:
	case en_chw: case en_cuhw:
	case en_cbu: case en_ccu: case en_chu:
	case en_cubu: case en_cucu: case en_cuhu:
	case en_ccwp: case en_cucwp:
	case en_sxb:	case en_sxc:	case en_sxh:
		return (8);
	case en_fcall:
	case en_regvar:
	case en_fpregvar:
		if (node->tp)
			return (node->tp->size);
		else
			return (8);
	case en_autofcon:
		return (8);
	case en_ref32: case en_ref32u:
		return (4);
	case en_b_ref:
	case en_ub_ref:
		return (1);
	case en_cbc:
	case en_c_ref:	return (2);
	case en_uc_ref:	return (2);
	case en_cbh:	return (4);
	case en_cch:	return (4);
	case en_h_ref:	return (4);
	case en_uh_ref:	return (4);
	case en_flt_ref: return (sizeOfFPS);
	case en_w_ref:  case en_uw_ref:
		return (8);
	case en_hp_ref:
		return (4);
	case en_wp_ref:
		return (8);
	case en_autovcon:
	case en_vector_ref:
		return (512);
	case en_dbl_ref:
		return (sizeOfFPD);
	case en_quad_ref:
		return (sizeOfFPQ);
	case en_triple_ref:
		return (sizeOfFPT);
	case en_tempfpref:
	if (node->tp)
		return (node->tp->precision/16);
	else
		return (8);
	case en_not:    case en_compl:
	case en_uminus: case en_assign:
		return GetNaturalSize(node->p[0]);
	case en_fadd:	case en_fsub:
	case en_fmul:	case en_fdiv:
	case en_fsadd:	case en_fssub:
	case en_fsmul:	case en_fsdiv:
	case en_vadd:	case en_vsub:
	case en_vmul:	case en_vdiv:
	case en_vadds:	case en_vsubs:
	case en_vmuls:	case en_vdivs:
	case en_add:    case en_sub:
	case en_mul:    case en_mulu:
	case en_div:	case en_udiv:
	case en_mod:    case en_umod:
	case en_and:    case en_or:     case en_xor:
	case en_asl:
	case en_shl:    case en_shlu:
	case en_shr:	case en_shru:
	case en_asr:	case en_asrshu:
	case en_feq:    case en_fne:
	case en_flt:    case en_fle:
	case en_fgt:    case en_fge:
	case en_eq:     case en_ne:
	case en_lt:     case en_le:
	case en_gt:     case en_ge:
	case en_ult:	case en_ule:
	case en_ugt:	case en_uge:
	case en_land:   case en_lor:
	case en_land_safe:   case en_lor_safe:
	case en_asadd:  case en_assub:
	case en_asmul:  case en_asmulu:
	case en_asdiv:	case en_asdivu:
	case en_asmod:  case en_asmodu: case en_asand:
	case en_asor:   case en_asxor:	case en_aslsh:
	case en_asrsh:
		siz0 = GetNaturalSize(node->p[0]);
		siz1 = GetNaturalSize(node->p[1]);
		if( siz1 > siz0 )
			return (siz1);
		else
			return (siz0);
	case en_void:   case en_cond:	case en_safe_cond:
		return (GetNaturalSize(node->p[1]));
	case en_bchk:
		return (GetNaturalSize(node->p[0]));
	case en_chk:
		return 8;
	case en_q2i:
	case en_t2i:
		return (sizeOfWord);
	case en_i2d:
		return (sizeOfWord);
	case en_i2t:
	case en_d2t:
		return (sizeOfFPT);
	case en_i2q:
	case en_d2q:
	case en_t2q:
		return (sizeOfFPQ);
	case en_object_list:
		return (GetNaturalSize(node->p[0]));
	case en_addrof:
		return (sizeOfPtr);
	default:
		printf("DIAG - natural size error %d.\n", node->nodetype);
		break;
	}
	return (0);
}



void ENODE::PutConstant(txtoStream& ofs, unsigned int lowhigh, unsigned int rshift)
{
	// ASM statment text (up to 3500 chars) may be placed in the following buffer.
	static char buf[4000];

	switch (nodetype)
	{
	case en_autofcon:
		sprintf_s(buf, sizeof(buf), "%lld", i);
		ofs.write(buf);
		break;
	case en_fcon:
		goto j1;
		// The following spits out a warning, but is okay.
		sprintf_s(buf, sizeof(buf), "0x%llx", f);
		ofs.write(buf);
		break;
	case en_autovcon:
	case en_autocon:
	case en_icon:
		if (lowhigh == 2) {
			sprintf_s(buf, sizeof(buf), "%lld", i & 0xffff);
			ofs.write(buf);
		}
		else if (lowhigh == 3) {
			sprintf_s(buf, sizeof(buf), "%lld", (i >> 16) & 0xffff);
			ofs.write(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "%lld", i);
			ofs.write(buf);
		}
		if (rshift > 0) {
			sprintf_s(buf, sizeof(buf), ">>%d", rshift);
			ofs.write(buf);
		}
		break;
	case en_labcon:
	j1:
		sprintf_s(buf, sizeof(buf), "%s_%lld", GetNamespace(), i);
		ofs.write(buf);
		if (rshift > 0) {
			sprintf_s(buf, sizeof(buf), ">>%d", rshift);
			ofs.write(buf);
		}
		break;
	case en_clabcon:
		sprintf_s(buf, sizeof(buf), "%s_%lld", GetNamespace(), i);
		ofs.write(buf);
		if (rshift > 0) {
			sprintf_s(buf, sizeof(buf), ">>%d", rshift);
			ofs.write(buf);
		}
		break;
	case en_nacon:
		sprintf_s(buf, sizeof(buf), "%s", (char *)sp->c_str());
		ofs.write(buf);
		if (lowhigh == 3) {
			sprintf_s(buf, sizeof(buf), ">>16");
			ofs.write(buf);
		}
		if (rshift > 0) {
			sprintf_s(buf, sizeof(buf), ">>%d", rshift);
			ofs.write(buf);
		}
		break;
	case en_scon:
		sprintf_s(buf, sizeof(buf), "\"%s\",0", (char *)sp->c_str());
		ofs.write(buf);
		break;
	case en_cnacon:
		sprintf_s(buf, sizeof(buf), "%s", (char *)msp->c_str());
		ofs.write(buf);
		if (rshift > 0) {
			sprintf_s(buf, sizeof(buf), ">>%d", rshift);
			ofs.write(buf);
		}
		break;
	case en_add:
		p[0]->PutConstant(ofs, 0, 0);
		ofs.write("+");
		p[1]->PutConstant(ofs, 0, 0);
		break;
	case en_sub:
		p[0]->PutConstant(ofs, 0, 0);
		ofs.write("-");
		p[1]->PutConstant(ofs, 0, 0);
		break;
	case en_uminus:
		ofs.write("-");
		p[0]->PutConstant(ofs, 0, 0);
		break;
	default:
		printf("DIAG - illegal constant node.\n");
		break;
	}
}

ENODE *ENODE::GetConstantHex(std::ifstream& ifs)
{
	static char buf[4000];
	ENODE *ep;

	ep = allocEnode();
	ifs.read(buf, 2);
	buf[2] = '\0';
	ep->nodetype = (e_node)strtol(buf, nullptr, 16);
	switch (ep->nodetype) {
	case en_autofcon:
		ifs.read(buf, 8);
		buf[8] = '\0';
		ep->i = strtol(buf, nullptr, 16);
		break;
	case en_fcon:
		goto j1;
	case en_autovcon:
	case en_autocon:
	case en_icon:
		ifs.read(buf, 8);
		buf[8] = '\0';
		ep->i = strtol(buf, nullptr, 16);
		break;
	case en_labcon:
j1:		;
	}
	return (ep);
}

void ENODE::PutConstantHex(txtoStream& ofs, unsigned int lowhigh, unsigned int rshift)
{
	// ASM statment text (up to 3500 chars) may be placed in the following buffer.
	static char buf[4000];

	ofs.printf("N%02X", nodetype);
	switch (nodetype)
	{
	case en_autofcon:
		sprintf_s(buf, sizeof(buf), "%08LLX", i);
		ofs.write(buf);
		break;
	case en_fcon:
		sprintf_s(buf, sizeof(buf), "%s_%lld:", GetNamespace(), i);
		ofs.write(buf);
		break;
	case en_autovcon:
	case en_autocon:
	case en_icon:
		sprintf_s(buf, sizeof(buf), "%08llX", i);
		ofs.write(buf);
		break;
	case en_labcon:
		sprintf_s(buf, sizeof(buf), "%s_%lld:", GetNamespace(), i);
		ofs.write(buf);
		break;
	case en_clabcon:
		sprintf_s(buf, sizeof(buf), "%s_%lld:", GetNamespace(), i);
		ofs.write(buf);
		break;
	case en_nacon:
		sprintf_s(buf, sizeof(buf), "%s:", (char *)sp->c_str());
		ofs.write(buf);
		break;
	case en_cnacon:
		sprintf_s(buf, sizeof(buf), "%s:", (char *)msp->c_str());
		ofs.write(buf);
		break;
	case en_add:
		p[0]->PutConstantHex(ofs, 0, 0);
		ofs.write("+");
		p[1]->PutConstantHex(ofs, 0, 0);
		break;
	case en_sub:
		p[0]->PutConstantHex(ofs, 0, 0);
		ofs.write("-");
		p[1]->PutConstantHex(ofs, 0, 0);
		break;
	case en_uminus:
		ofs.write("-");
		p[0]->PutConstantHex(ofs, 0, 0);
		break;
	default:
		printf("DIAG - illegal constant node.\n");
		break;
	}
}

void ENODE::storeHex(txtoStream& ofs)
{
	if (this == nullptr) {
		ofs.puts("x");
		return;
	}
	ofs.puts("X");
	ofs.writeAsHex((char *)this, sizeof(ENODE));
	if (tp)
		ofs.printf("%05X:", tp->typeno);
	else
		ofs.printf("FFFFF:");
	if (sym)
		ofs.printf("%05X:", sym->number);
	else
		ofs.printf("FFFFF:");
	vmask->store(ofs);
	if (sp) {
		ofs.printf("%03X:", (int)sp->length());
		ofs.writeAsHex(sp->c_str(), sp->length());
	}
	else
		ofs.printf("000:");
	if (msp) {
		ofs.printf("%03X:", (int)msp->length());
		ofs.writeAsHex(msp->c_str(), msp->length());
	}
	else
		ofs.printf("000:");
	if (udnm) {
		ofs.printf("%03X:", (int)udnm->length());
		ofs.writeAsHex(udnm->c_str(), udnm->length());
	}
	else
		ofs.printf("000:");
	p[0]->store(ofs);
	p[1]->store(ofs);
	p[2]->store(ofs);
}

void ENODE::loadHex(txtiStream& ifs)
{
	int len, nn;
	static char buf[8000];

	ifs.readAsHex(this, sizeof(ENODE));
	ifs.read(buf, 6);
	nn = strtoul(buf, nullptr, 16);
	if (nn < 65535)
		tp = &compiler.typeTable[nn];
	ifs.read(buf, 6);
	nn = strtoul(buf, nullptr, 16);
	if (nn < 65535)
		sym = &compiler.symbolTable[nn];
	vmask = allocEnode();
	vmask->load(ifs);
	ifs.read(buf, 4);
	nn = strtoul(buf, nullptr, 16);
	ifs.readAsHex(buf, nn * 2);
	buf[nn * 2] = '\0';
	sp = new std::string(buf);
	ifs.read(buf, 4);
	nn = strtoul(buf, nullptr, 16);
	ifs.readAsHex(buf, nn * 2);
	buf[nn * 2] = '\0';
	msp = new std::string(buf);
	ifs.read(buf, 4);
	nn = strtoul(buf, nullptr, 16);
	ifs.readAsHex(buf, nn * 2);
	buf[nn * 2] = '\0';
	udnm = new std::string(buf);
	ifs.read(buf, 1);	// should be 'X'
	if (buf[0]=='X')
		p[0]->load(ifs);
	ifs.read(buf, 1);
	if (buf[0] == 'X')
		p[1]->load(ifs);
	ifs.read(buf, 1);
	if (buf[0] == 'X')
		p[2]->load(ifs);
}

void ENODE::store(txtoStream& ofs)
{
}

void ENODE::load(txtiStream& ifs)
{
}
