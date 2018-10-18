// ============================================================================
//        __
//   \\__/ o\    (C) 2018  Robert Finch, Stratford
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
// AS64 - Assembler
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
#ifndef TOKEN_H
#define TOKEN_H

enum {
     tk_eof = -1,
     tk_none = 0,
     tk_comma = ',',
     tk_hash = '#',
     tk_plus = '+',
     tk_eol = '\n',
     tk_add = 128,
     tk_addi,
     tk_addu, // 130
     tk_2addu,
     tk_4addu,
     tk_8addu,
	 tk_10addu,
     tk_16addu,
     tk_2addui,
     tk_4addui,
     tk_8addui,
     tk_16addui,
	 tk_abs,
     tk_addui,
     tk_align, //140
     tk_and,
     tk_andi,
     tk_asl,
     tk_asli,
     tk_asr,
     tk_asri,
     tk_begin_expand,
	 tk_bbc,
	 tk_bbs,
     tk_beq,
	 tk_beqi,
     tk_bge,
	 tk_bgei,
     tk_bgeu,
	 tk_bgeui,
     tk_bfchg, // 150
     tk_bfclr,
     tk_bfext,
     tk_bfextu,
     tk_bfins,
     tk_bfinsi,
     tk_bfset,
     tk_bgt,
	 tk_bgti,
     tk_bgtu,
	 tk_bgtui,
     tk_bit,
     tk_biti, // 160
     tk_bits,
     tk_ble,
	 tk_blei,
     tk_bleu,
	 tk_bleui,
     tk_blt,
	 tk_blti,
     tk_bltu,
	 tk_bltui,
     tk_bmi,
     tk_bne,
	 tk_bnei,
     tk_bpl,
     tk_br,
     tk_bra, // 170
     tk_brk,
     tk_brnz,
     tk_brz,
     tk_bsr,
     tk_bss,
     tk_bvc,
     tk_bvs,
     tk_byte,
	 tk_cache,
	 tk_call,
	 tk_calltgt,
     tk_cas,
     tk_chk, // 180
     tk_chki,
     tk_cli,
	 tk_cmovenz,
	 tk_cmovez,
	 tk_cmovfnz,
     tk_cmp,
     tk_cmpi,
     tk_cmpu,
	 tk_cmpui,
     tk_code,
     tk_com,
     tk_cpuid,
     tk_cs,
	 tk_csev,
	 tk_csp,
	 tk_csod,
	 tk_csn,
	 tk_csnp,
	 tk_csnn,
	 tk_csnz,
	 tk_csz,
	 tk_csrrc,
	 tk_csrrd,
	 tk_csrrs,
     tk_csrrw, // 190
     tk_data,
     tk_db, 
     tk_dbnz,
     tk_dc,
     tk_dec,
     tk_dh,
     tk_dh_htbl,
     tk_div,
     tk_divi,
     tk_divu,
     tk_divui, // 200
		 tk_divwait,
	 tk_dd,
	 tk_do,
     tk_ds,
	 tk_dt,
     tk_dw,
	 tk_else,
     tk_end,
     tk_end_expand,
	 tk_endif,
	 tk_endm,
     tk_endpublic,
     tk_enor,
     tk_eor,
     tk_eori,
     tk_eq,
     tk_equ,
	 tk_eret,
     tk_es, // 210
     tk_extern,
     tk_fabs,  
     tk_fadd,
	 tk_fbeq,
	 tk_fbge,
	 tk_fbgt,
	 tk_fble,
	 tk_fblt,
	 tk_fbne,
	 tk_fbor,
	 tk_fbun,
     tk_fcmp,
     tk_fcx,
     tk_fdiv,
     tk_fdx,
     tk_fex,
		 tk_file,
     tk_fill,
     tk_fix2flt,//220
     tk_flt2fix,
     tk_fmov,
     tk_fmul,
     tk_fnabs,
     tk_fneg,
     tk_frm,
     tk_fs,
		tk_fslt,
     tk_fstat,
     tk_fsub,
	 tk_ftoi,
     tk_ftst, // 230
     tk_ftx,
			 tk_fxdiv,
			 tk_fxmul,
     tk_ge,
     tk_gran,
     tk_gs,
     tk_gt,
	 tk_hint,
     tk_hs,
	 tk_ibne,
     tk_icon,
     tk_id,
     tk_inc,
	 tk_if,
	 tk_ifdef,
	 tk_ifndef,
     tk_int, // 240
     tk_ios,
	 tk_ipush,
	 tk_ipop,
	 tk_iret,
	 tk_isnull,
	 tk_isptr,
	 tk_itof,
     tk_jal,
     tk_jci,
     tk_jhi,
     tk_jgr,
     tk_jmp,
     tk_jsf,
     tk_jsp,
     tk_jsr,
     tk_land, // 250
     tk_lb,
     tk_lbu,
     tk_lc,
     tk_lcu,
	 tk_ld,
	 tk_ldb,
	 tk_ldbu,
	 tk_ldd,
     tk_ldi,
     tk_ldis,
	 tk_ldp,
	 tk_ldpu,
	 tk_ldt,
	 tk_ldtu,
	 tk_ldvdar,
	 tk_ldw,
	 tk_ldwu,
     tk_le,
     tk_lea,
	 tk_leax,
	 tk_lf,
     tk_lfd,
	 tk_lfq,
     tk_lh,
     tk_lhu, // 260
	 tk_link,
     tk_lla,
     tk_llax,
     tk_lmr,
     tk_loop,
     tk_lor,
     tk_lshift,
     tk_lsr,
     tk_lsri,
     tk_lt,  // 270
	 tk_ltcb,
     tk_lui,
	 tk_lv,
     tk_lvb,
	tk_lvbu,
     tk_lvc,
	tk_lvcu,
     tk_lvh,
	tk_lvhu,
     tk_lvw,
     tk_lvwar,
     tk_lw,
	 tk_lwr,
     tk_lwar,
     tk_lws,
	 tk_macro,
     tk_max,
	 tk_mark1,
	 tk_mark2,
	 tk_mark3,
	 tk_mark4,
     tk_memdb,
     tk_memsb,
     tk_message,
     tk_mffp, //280
     tk_mfspr,
     tk_mod,
     tk_modi,
	tk_modsu,
     tk_modu,
     tk_modui,
			 tk_modwait,
     tk_mov,
     tk_mtfp,
     tk_mtspr,
     tk_mul,
	tk_mulh,
     tk_muli, //290
	tk_mulsu,
	tk_mulsuh,
     tk_mulu,
	tk_muluh,
     tk_mului,
     tk_mv2fix,
     tk_mv2flt,
     tk_nand,
     tk_ne,
     tk_neg,
     tk_nop,
     tk_nor,
     tk_not, //300
     tk_or,
     tk_ori,
     tk_org,
     tk_pand,
     tk_pandc,
     tk_pea,
     tk_penor,
     tk_peor,
     tk_php, //310
     tk_plp,
     tk_pnand,
     tk_pnor,
     tk_pop,
     tk_por,
     tk_porc,
     tk_pred,
     tk_public,
     tk_push,
     tk_rconst,//320
	 tk_redor,
	 tk_ret,
	 tk_rex,
     tk_rodata,
     tk_rol,
     tk_roli,
     tk_ror,
     tk_rori,
     tk_rshift,
     tk_rtd,
     tk_rte,
     tk_rtf,
     tk_rti,
     tk_rtl,
     tk_rts,
     tk_sb,
     tk_sc,
     tk_sei,
     tk_seq,
     tk_seqi,
		 tk_setwb,
	 tk_sf,
     tk_sfd,
	 tk_sfq,
     tk_sgt,
     tk_sgti,
     tk_sgtu,
     tk_sgtui,
     tk_sge,
     tk_sgei,
     tk_sgeu,
     tk_sgeui,
     tk_sh,
     tk_shl,
     tk_shli,
     tk_shr,
     tk_shri,
     tk_shru,
     tk_shrui,
     tk_slli,
     tk_slt,
     tk_slti,
     tk_sltu,
     tk_sltui,
     tk_sle,
     tk_slei,
     tk_sleu,
     tk_sleui,
     tk_smr,
     tk_sne,
     tk_snei,
     tk_sptr,
     tk_srai,
     tk_srli,
     tk_ss,
	 tk_stb,
	 tk_std,
	 tk_stdcr,
	 tk_stp,
	 tk_stt,
	 tk_stw,
	 tk_stcb,
     tk_stcmp,
     tk_stmov,
     tk_stop,
			 tk_strconst,
     tk_stset,
     tk_stsb,
     tk_stsc,
     tk_stsh,
     tk_stsw,
     tk_sub,
     tk_subi,
     tk_subu,
     tk_subui,
	 tk_sv,
     tk_sw,
     tk_swc,
     tk_swcr,
     tk_swap,
	 tk_swp,
     tk_sws,
     tk_sxb,
     tk_sxc,
     tk_sxh,
     tk_sync,
     tk_sys,
	 tk_tgt,
     tk_tlbdis,
     tk_tlben,
     tk_tlbpb,
     tk_tlbrd,
     tk_tlbrdreg,
     tk_tlbwi,
     tk_tlbwr,
     tk_tlbwrreg,
     tk_tls,
     tk_to,
			 tk_transform,
     tk_tst,
	 tk_unlink,
	 tk_vadd,
	 tk_vadds,
	 tk_vand,
	 tk_vands,
	 tk_vdiv,
	 tk_vdivs,
	 tk_vmov,
	 tk_vmul,
	 tk_vmuls,
	 tk_vor,
	 tk_vors,
	 tk_vseq,
	 tk_vsge,
	 tk_vslt,
	 tk_vsne,
	 tk_vsub,
	 tk_vsubs,
	 tk_vxor,
	 tk_vxors,
     tk_wai,
     tk_xnor,
     tk_xor,
     tk_xori,
     tk_zs,
	 tk_zsev,
	 tk_zsp,
	 tk_zsod,
	 tk_zsn,
	 tk_zsnp,
	 tk_zsnn,
	 tk_zsnz,
	 tk_zsz,
     tk_zxb,
     tk_zxc,
     tk_zxh
};

extern int token;
extern int isIdentChar(char ch);
extern void ScanToEOL();
extern int NextToken();
extern void SkipSpaces();
extern void prevToken();
extern int need(int);
extern int expect(int);
extern int getRegister();
extern int getSprRegister();
extern int getFPRegister();
extern int getFPRoundMode();

#endif
