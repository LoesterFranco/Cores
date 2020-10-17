#include "stdafx.h"

extern int countLeadingBits(int64_t val);
extern double clog2(double n);

int64_t Posit64::posWidth = 64;
int64_t Posit64::expWidth = 3;

Posit64::Posit64(int64_t i)
{
  val = 0;
  IntToPosit(i);
}

Posit64 Posit64::Addsub(int8_t op, Posit64 a, Posit64 b)
{
  int64_t rs = (int64_t)clog2((double)(posWidth - 1LL)) - 1LL;
  int64_t es = expWidth;
  int8_t sa, sb, so;
  int8_t rop;
  int64_t rgma, rgmb, rgm1, rgm2, argm1, argm2;
  int64_t absrgm1;
  int8_t rgsa, rgsb, rgs1, rgs2;
  int64_t diff;
  int8_t expa, expb, exp1, exp2;
  int16_t exp_diff;
  uint64_t siga, sigb, sig1, sig2;
  uint64_t sigi;
  Int128 sig1s, sig2s, sig_sd, sig_ls, t1, t2;
  int8_t zera, zerb;
  int8_t infa, infb;
  int8_t sigov;
  int64_t lzcnt;
  int64_t aa, bb;
  bool aa_gt_bb;
  int8_t inf, zero;
  int64_t rxtmp, rxtmp1, srxtmp1, abs_rxtmp;
  int64_t rgmo;
  int64_t expo;
  int64_t exp_mask = (((uint64_t)1 << (uint64_t)expWidth) - 1LL);
  int64_t rs_mask = (((uint64_t)1 << (uint64_t)rs) - 1LL);
  RawPosit ad((int8_t)3, (int8_t)64), bd((int8_t)3, (int8_t)64);
  int64_t S, T, L, G, R, St, St1;
  CSet s1, s2;
  int nn;
  uint64_t ulp, rnd_ulp, tmp1_rnd_ulp, tmp1_rnd;
  int64_t abs_tmp, o;
  Posit64 out;

  Decompose(a, &ad);
  sa = ad.sign;
  rgsa = ad.regsign;
  rgma = ad.regime;
  expa = ad.exp;
  siga = ad.sig.low;
  zera = ad.isZero;
  infa = ad.isInf;

  Decompose(b, &bd);
  sb = bd.sign;
  rgsb = bd.regsign;
  rgmb = bd.regime;
  expb = bd.exp;
  sigb = bd.sig.low;
  zerb = bd.isZero;
  infb = bd.isInf;

  inf = infa | infb;
  zero = zera & zerb;
  aa = sa ? -a.val : a.val;
  bb = sb ? -b.val : b.val;
  aa_gt_bb = aa >= bb;

  // Determine op really wanted
  rop = sa ^ sb ^ op;

  // Sort operand components
  rgs1 = aa_gt_bb ? rgsa : rgsb;
  rgs2 = aa_gt_bb ? rgsb : rgsa;
  rgm1 = aa_gt_bb ? rgma : rgmb;
  rgm2 = aa_gt_bb ? rgmb : rgma;
  exp1 = aa_gt_bb ? expa : expb;
  exp2 = aa_gt_bb ? expb : expa;
  sig1 = aa_gt_bb ? siga : sigb;
  sig2 = aa_gt_bb ? sigb : siga;

  argm1 = rgs1 ? rgm1 : -rgm1;
  argm2 = rgs2 ? rgm2 : -rgm2;

  diff = ((argm1 << expWidth) | exp1) - ((argm2 << expWidth) | exp2);
  exp_diff = (diff > (1LL << (rs + 1LL))) ? -1LL : diff & rs_mask;
  sig1s.low = 0;
  sig1s.high = sig1;
  sig2s.low = 0;
  sig2s.high = sig2;
  Int128::Shr(&sig2s, &sig2s, exp_diff);
  if (rop)
    sigov = Int128::Sub(&sig_sd, &sig1s, &sig2s);
  else
    sigov = Int128::Add(&sig_sd, &sig1s, &sig2s);
  sigi = ((int64_t)sigov << 63LL) | (sig_sd.high & 0x7FFFFFFFFFFFFFFFLL);
  lzcnt = countLeadingZeros(sigi);
  Int128::Shl(&sig_ls, &sig_sd, lzcnt);
  absrgm1 = rgs1 ? rgm1 : -rgm1;  // rgs1 = 1 = positive
  if (expWidth > 0) {
    rxtmp = ((absrgm1 << (int64_t)expWidth) | exp1) - (lzcnt-(int64_t)expWidth);
    rxtmp1 = rxtmp + sigov; // add in overflow if any
    srxtmp1 = (uint64_t)rxtmp1 >> (int64_t)(expWidth + rs + 1LL);
    abs_rxtmp = srxtmp1 ? -rxtmp1 : rxtmp1;
    if (srxtmp1 && (abs_rxtmp & exp_mask))
      expo = rxtmp1 & exp_mask;
    else
      expo = abs_rxtmp & exp_mask;
    if (~srxtmp1 || (srxtmp1 & ((abs_rxtmp & exp_mask) != 0LL)))
      rgmo = ((abs_rxtmp >> expWidth) & rs_mask) + 1LL;
    else
      rgmo = ((abs_rxtmp >> expWidth) & rs_mask);
  }
  else {
    rxtmp = absrgm1 - lzcnt;
    rxtmp1 = rxtmp + sigov;   // add in overflow if any
    srxtmp1 = (rxtmp1 >> rs) & 1LL;
    abs_rxtmp = srxtmp1 ? -rxtmp1 : rxtmp1;
    expo = 0;
    rgmo = ~srxtmp1 ? (abs_rxtmp & rs_mask) + 1LL : abs_rxtmp & rs_mask;
  }
  // Exponent and Significand Packing
  s1.enlarge(8);  // ensure there are enough bits
  switch (es) {
  case 0:
    S = (sig_ls.low & 0x1FFFFFFFFFFFFFFFLL) != 0;
    s1.clear();
    s1.insert(S, 0, 1);
    Int128::Shr(&t1, &sig_ls, posWidth - 2);
    s1.insert(sig_ls.low >> 62LL, 1, 2);
    s1.insert(sig_ls.high, 3, 63);
    s1.insert(srxtmp1, 66, 64);
    s1.insert(~srxtmp1, 130, 64);
    break;
  case 1:
    S = (sig_ls.low & 0x3FFFFFFFFFFFFFFFLL) != 0;
    s1.clear();
    s1.insert(S, 0, 1);
    Int128::Shr(&t1, &sig_ls, posWidth - 1);
    s1.insert(sig_ls.low >> 63LL, 1, 1);
    s1.insert(sig_ls.high, 2, 63);
    s1.insert(expo, 65, 1);
    s1.insert(srxtmp1, 66, 64);
    s1.insert(~srxtmp1, 130, 64);
    break;
  case 2:
    S = (sig_ls.low & 0x7FFFFFFFFFFFFFFFLL) != 0;
    s1.clear();
    s1.insert(S, 0, 1);
    Int128::Shr(&t1, &sig_ls, posWidth);
    s1.insert(sig_ls.high, 1, 63);
    s1.insert(expo, 64, 2);
    s1.insert(srxtmp1, 66, 64);
    s1.insert(~srxtmp1, 130, 64);
    break;
  case 3:
    S = (sig_ls.low & 0xFFFFFFFFFFFFFFFFLL) != 0;
    s1.clear();
    s1.insert(S, 0, 1);
    Int128::Shr(&t1, &sig_ls, posWidth);
    s1.insert(sig_ls.low >> 1LL, 1, 62);
    s1.insert(expo, 63, 3);
    s1.insert(srxtmp1, 66, 64);
    s1.insert(~srxtmp1, 130, 64);
    break;
  }
  s1.shl(posWidth);
  s2.copy(s1);
  s2.shr(rgmo);

//             wire[3 * PSTWID - 1 + 3:0] tmp1 = { tmp,{PSTWID{1'b0}}} >> rgmo;
// Rounding
// Guard, Round, and Sticky
  s2.extract(&L, posWidth + 4, 1);
  s2.extract(&G, posWidth + 3, 1);
  s2.extract(&R, posWidth + 2, 1);
  St = 0;
  for (nn = posWidth + 1; nn >= 0; nn--) {
    s2.extract(&St1, nn, 1);
    St = St | St1;
  }
  ulp = ((G & (R | St)) | (L & G & ~(R | St)));
  s2.extract((int64_t*)&tmp1_rnd_ulp, posWidth * 2 - 1 + 3, posWidth);
  t1.low = tmp1_rnd_ulp;
  t1.high = 0;
  t2.low = ulp;
  t2.high = 0;
  Int128::Add(&t1, &t1, &t2);
  s2.extract((int64_t*)&t2.low, posWidth + 3, posWidth);
  tmp1_rnd = (rgmo < posWidth - es - 2) ? tmp1_rnd_ulp : t2.low;

  // Compute output sign
  switch ((zero << 3) | (sa << 2) | (op << 1) | sb) {
  case 0: so = 0; break;
  case 1: so = !aa_gt_bb; break;
  case 2: so = !aa_gt_bb; break;
  case 3: so = 0; break;
  case 4: so = aa_gt_bb; break;
  case 5: so = 1; break;
  case 6: so = 1; break;
  case 7: so = aa_gt_bb; break;
  default: so = 0; break;
  }

  abs_tmp = so ? ~tmp1_rnd + 1 : tmp1_rnd;

  switch ((zero << 2) | (inf << 1) | (sig_ls.high & 1LL)) {
  case 0: o = (so << 63LL) | (abs_tmp >> 1); break;
  case 1: o = 0; break;
  case 2:
  case 3: o = (1LL << 63LL); break;
  case 4:
  case 5:
  case 6:
  case 7: o = 0; break;
  }
  out.val = o;
  return (out);
}


Posit64 Posit64::Add(Posit64 a, Posit64 b)
{
  return (Addsub(0, a, b));
}

Posit64 Posit64::Sub(Posit64 a, Posit64 b)
{
  return (Addsub(1, a, b));
}

Posit64 Posit64::Multiply(Posit64 a, Posit64 b)
{
  RawPosit rawA(3, 64);
  RawPosit rawB(3, 64);
  RawPosit rawP(3, 64);
  Int128 prod, prod1;
  int64_t argma, argmb;
  int64_t mo;
  int64_t rs = clog2(posWidth - 1);
  Posit64 o;

  Decompose(a, &rawA);
  Decompose(b, &rawB);
  int64_t inf = rawA.isInf | rawB.isInf;
  int64_t zero = rawA.isZero | rawB.isZero;
  int64_t so = rawA.sign ^ rawB.sign;
  rawP.isNaR = rawA.isNaR || rawB.isNaR;
  rawP.isInf = rawA.isInf || rawB.isInf;
  rawP.isZero = rawA.isZero || rawB.isZero;
  rawP.sign = rawA.sign ^ rawB.sign;
  rawP.regexp = rawA.regexp + rawB.regexp;
  Int128::Mul(&rawP.sig, &rawA.sig, &rawB.sig);
  if ((rawP.sig.high & 0x80000000000000LL) == 0LL) {
    Int128::Shl(&rawP.sig, &rawP.sig, 1);
    rawP.regexp--;
  }
  rawP.regime = rawP.regexp >> 3LL;
  rawP.regsign = (rawP.regime >> 63LL) & 1LL;
  rawP.exp = rawP.regexp & 7LL;
  argma = rawA.regsign ? -rawA.regime : rawA.regime;
  argmb = rawB.regsign ? -rawB.regime : rawB.regime;
  mo = rawP.sig.Int128::extract(&rawP.sig, (posWidth - expWidth) * 2LL - 1LL, 1LL);
  if (mo)
    Int128::Assign(&prod1, &rawP.sig);
  else {
    Int128::Assign(&prod1, &rawP.sig);
    Int128::Shl(&prod1, &prod1, 1LL);
  }
  int64_t rxtmp = ((argma << expWidth) | rawA.exp) + ((argmb << expWidth) | rawB.exp) + mo;
  int64_t srxtmp = ((rxtmp >> (rs + expWidth + 1)) & 1LL);
  int64_t rxtmp2c = srxtmp ? -rxtmp : rxtmp;
  int64_t exp = rxtmp & ((1LL << expWidth) - 1LL);
  int64_t rgm = (srxtmp ? -rxtmp : rxtmp) >> expWidth;
  int64_t rxn = srxtmp ? rxtmp2c : rxtmp;
  int64_t rgml = (~srxtmp || ((rxn & ((1LL << expWidth) - 1LL)) != 0)) ? (rxtmp2c >> expWidth) + 1LL : (rxtmp2c >> expWidth);
  rgml = rgml & ((1LL << rs) - 1LL);
  Int256 tmp;
  tmp = *tmp.Zero();
  tmp.insert(rawP.sig.StickyCalc(&rawP.sig, posWidth - expWidth - 3),0,1);
  tmp.insert(rawP.sig.extract(&rawP.sig, posWidth - expWidth - 2, (posWidth - expWidth) * 2LL - 2LL - (posWidth - expWidth - 2LL)), 1, 
    (posWidth - expWidth) * 2LL - 2LL - (posWidth - expWidth - 2LL));
  tmp.insert(exp, (posWidth - expWidth) * 2LL - 2LL - (posWidth - expWidth - 2LL) + 2, expWidth);
  int64_t srxx = srxtmp ? 0LL : -1LL;
  tmp.insert(srxtmp, (posWidth - expWidth) * 2LL - 2LL - (posWidth - expWidth - 2LL) + 2 + expWidth, 1);
  tmp.insert(srxx, (posWidth - expWidth) * 2LL - 2LL - (posWidth - expWidth - 2LL) + 2 + expWidth + 1, posWidth);
  Int256 tmp1;
  Int256::Assign(&tmp1, &tmp);
  Int256::Shl(&tmp1, &tmp1, posWidth);
  Int256::Shr(&tmp1, &tmp1, rgml);
  int64_t L = tmp1.extract(&tmp1, posWidth + 4, 1);
  int64_t G = tmp1.extract(&tmp1, posWidth + 3, 1);
  int64_t R = tmp1.extract(&tmp1, posWidth + 2, 1);
  int64_t St = tmp1.StickyCalc(&tmp1, posWidth + 1);
  int64_t ulp = ((G & (R | St)) | (L & G & ~(R | St)));
  int64_t tmp1_rnd_ulp = tmp1.extract(&tmp1, posWidth + 3, posWidth) + ulp;
  int64_t c = tmp1.AddCarry(tmp1_rnd_ulp, tmp1.extract(&tmp1, posWidth + 3, posWidth), ulp);
  int64_t tmp1_rnd = (rgml < posWidth - expWidth - 2LL) ? tmp1_rnd_ulp : tmp1.extract(&tmp1, posWidth + 3, posWidth);
  int64_t abs_tmp = so ? -tmp1_rnd : tmp1_rnd;
  int64_t out = zero ? 0LL : inf ? 0x8000000000000000LL : (so << 63LL) | (abs_tmp >> 1LL);
  o.val = out;
  return (o);
}

Posit64 Posit64::Divide(Posit64 a, Posit64 b)
{
  RawPosit aa(Posit64::expWidth, Posit64::posWidth);
  RawPosit bb(Posit64::expWidth, Posit64::posWidth);
  Posit64 out;
  int64_t so;
  int64_t inf;
  int64_t zer;
  int64_t m1, m2;
  int16_t m2_inv0_tmp;
  int64_t argma, argmb;
  int M = posWidth - expWidth;
  int64_t Bs = clog2(posWidth - 1LL);
  int8_t NR_Iter;
  int8_t IW_MAX = 17;
  int8_t IW = 17;
  int8_t AW = 16;
  int8_t AW_MAX = 16;
  int64_t m2_inv0;
  Int128 div_m, div_mN, div_mNm;
  Int128 m2_inv[5];
  Int128 m2_inv_X_m2[5];
  int64_t m2_inv_X_m2_64;
  Int128 t1, t2, t3;
  Int128 m2_128, mask;
  int64_t two_m2_inv_X_m2[5];
  int8_t ii, jj;
  int64_t St, tt;
  int64_t div_m_udf;
  int64_t div_e, div_eN;
  int64_t bin;
  int64_t e_o, r_o, ro_s;
  int64_t exp_oN;
  int64_t div_e_mask;
  int64_t tmp_o0, tmp_o1, tmp_o2, tmp_o3;
  Int256 tmp_o, tmp1_o;

  if (M > 136)
    NR_Iter = 4;
  else if (M > 68)
    NR_Iter = 3;
  else if (M > 34)
    NR_Iter = 2;
  else if (M > 17)
    NR_Iter = 1;
  else
    NR_Iter = 0;

  Posit64::Decompose(a, &aa);
  Posit64::Decompose(b, &bb);
  inf = aa.isInf | bb.isZero;
  zer = aa.isZero | bb.isInf;
  so = aa.sign ^ bb.sign;
  m1 = aa.sig.low << 1LL;
  m2 = bb.sig.low << 1LL;
  argma = aa.regsign ? -aa.regime : aa.regime;
  argmb = bb.regsign ? -bb.regime : bb.regime;

  if (M < AW_MAX)
    m2_inv0_tmp = DividerLUT[(m2 << (AW_MAX-M)) & 0xFFFFLL];
  else if (M == AW_MAX)
    m2_inv0_tmp = DividerLUT[m2 & 0xFFFFLL];
  else
    m2_inv0_tmp = DividerLUT[(m2 >> (M-AW_MAX)) & 0xFFFFLL];

  m2_inv0 = m2_inv0_tmp;

  if (NR_Iter > 0) {
    m2_inv[0].low = m2_inv0;
    m2_inv[0].high = 0;
    Int128::Shl(&m2_inv[0], &m2_inv[0], M+(M-IW));
    for (ii = 0; ii < NR_Iter; ii++) {
      Int128::Assign(&t1, &m2_inv[ii]);
      Int128::Shr(&t1, &t1, 2 * M - IW * (ii + 1));
      Int128::Assign(&t2, &t1);
      Int128::Shl(&t2, &t2, 2 * M - IW * (ii + 1) - M);
      m2_128.low = m2;
      m2_128.high = 0;
      Int128::Mul(&m2_inv_X_m2[ii], &t2, &m2_128);
      Int128::Shr(&t1, &m2_inv_X_m2[ii], M + 3);
      Int128::Assign(&t2, &t1);
      tt = ((1LL << (M + 2)) - 1LL);
      St = (m2_inv_X_m2[ii].low & tt) != 0LL;
      two_m2_inv_X_m2[ii] = (1LL << M) - ((m2_inv_X_m2[ii].low << 1LL) | St);
      Int128::Assign(&t1, &m2_inv[ii]);
      Int128::Shr(&t1, &t1, 2 * M - IW * (ii + 1));
      Int128::Assign(&t2, &t1);
      Int128::Shl(&t2, &t2, M - IW * (ii + 1));
      t1.low = (two_m2_inv_X_m2[ii] & ((1LL << M) - 1)) << 1LL;
      t1.high = 0;
      Int128::Mul(&m2_inv[ii+1], &t2, &t1);
    }
  }
  else {
    m2_inv[0].low = m2_inv0;
    m2_inv[0].high = 0;
    Int128::Shl(&m2_inv[0], &m2_inv[0], M);
  }
  tt = bb.sig.low & ((1LL << (M - 2)) - 1);
  t1.low = m1;
  t1.high = 0;
  Int128::Shl(&t1, &t1, M);
  t2.low = m1;
  t2.high = 0;
  Int128::Shr(&t3, &m2_inv[NR_Iter], M);
  Int128::Mul(&t2, &t2, &t3);
  if (tt == 0)
    Int128::Assign(&div_m, &t2);
  else
    Int128::Assign(&div_m, &t3);
  Int128::Shr(&t1, &div_m, 2 * M + 1);
  div_m_udf = t1.low & 1LL;
  Int128::Shl(&div_mN, &div_m, div_m_udf==0LL);
  bin = tt == 0 || div_m_udf ? 0 : 1;
  div_e = ((argma << expWidth) | aa.exp) - ((argmb << expWidth) | bb.exp) - bin;
  e_o = div_e & ((1LL << expWidth) - 1LL);
  div_e_mask = (1LL << (expWidth + Bs + 1LL)) - 1LL;
  exp_oN = ((div_e >> (expWidth + Bs + 1LL)) & 1LL) ? -div_e & div_e_mask : div_e & div_e_mask;
  r_o = ~((div_e >> (expWidth + Bs + 1LL)) & 1LL) || ((exp_oN >> expWidth) & 1LL) ? (exp_oN >> expWidth) + 1LL : (exp_oN >> expWidth);

  // Exponent and mantissa packing
  mask.low = 1LL;
  mask.high = 0LL;
  t1.low = 1LL;
  t1.high = 0LL;
  Int128::Shl(&mask, &mask, 2LL * M);
  Int128::Sub(&mask, &mask, &t1);
  div_mNm.low = div_mN.low & mask.low;
  div_mNm.high = div_mN.high & mask.high;
  tmp_o.insert(Int128::StickyCalc(&div_mNm, 2LL * M - (posWidth - expWidth - 1LL) - 2LL), 0LL, 1LL);
  Int128::Shr(&t1, &div_mNm, 2LL * M - (posWidth - expWidth - 1LL) - 1LL);
  tmp_o.insert(t1, 1LL, 2LL);
  Int128::Shr(&t1, &div_mNm, 2LL * M - (posWidth - expWidth - 1LL) + 1LL);
  tmp_o.insert(t1, 3LL, (posWidth - expWidth - 1LL) + 1LL);
  tmp_o.insert(e_o, 3LL + (posWidth - expWidth - 1LL) + 1LL, expWidth);
  tmp_o.insert((div_e >> (expWidth + Bs + 1LL)) & 1LL, expWidth + 3LL + (posWidth - expWidth - 1LL) + 1LL, 1LL);
  if ((div_e >> (expWidth + Bs + 1LL)) & 1LL)
    div_eN = 0LL;
  else
    div_eN = -1LL;
  tmp_o.insert(div_eN, expWidth + 3LL + (posWidth - expWidth - 1LL) + 1LL + 1LL, 64LL);
  Int256::Assign(&tmp1_o, &tmp_o);
  Int256::Shl(&tmp1_o, &tmp1_o, 64LL);
  ro_s = (r_o >> Bs) & 1LL;
  tt = ro_s ? ((1LL << Bs) - 1LL) : r_o;
  Int256::Shr(&tmp1_o, &tmp1_o, tt);

  // Rounding
  int64_t L = (tmp1_o.midLow >> 4LL) & 1LL;
  int64_t G = (tmp1_o.midLow >> 3LL) & 1LL;
  int64_t R = (tmp1_o.midLow >> 2LL) & 1LL;
  St = (tmp1_o.low != 0LL) || ((tmp1_o.midLow & 3LL) != 0LL);
  int64_t ulp = ((G & (R | St)) | (L & G & ~(R | St)));
  Int256::Shr(&tmp1_o, &tmp1_o, 3LL);
  int64_t tmp1_o_rnd_ulp = tmp1_o.low + ulp;
  int64_t c = Int128::AddCarry(tmp1_o_rnd_ulp, tmp1_o.low, ulp);
  int64_t tmp1_o_rnd = (r_o < M - 2) ? tmp1_o_rnd_ulp : tmp1_o.low;

  // Final Output
  int64_t tmp1_oN = so ? -tmp1_o_rnd : tmp1_o_rnd;
  Int128::Shr(&div_mN, &div_mN, 2LL * M + 1LL);
  int64_t o = inf || zer || (div_mN.low & 1LL) ? (inf << (posWidth - 1LL)) : (so << (posWidth - 1LL)) | (tmp1_oN >> 1LL);
  out.val = o;
  return (out);
}


Posit64 Posit64::IntToPosit(int64_t i)
{
  int8_t sgn;
  int64_t ii;
  int8_t lzcnt;
  int8_t rgm, rgm_sh;
  uint64_t sig;
  int8_t exp;
  Int128 tmp, tmp1, tmp2, tmp3, tmp2_rnd;
  int64_t ones;
  Int128 L, G, R;
  int64_t S;
  int64_t nn;
  int64_t rnd_ulp;
  Posit64 pst;

  if (i == 0) {
    pst.val = 0;
    return (pst);
  }
  ii = i < 0 ? -i : i;
  lzcnt = countLeadingZeros(ii) - 1;
  sgn = (i >> 63LL) & 1LL;
  rgm = (posWidth - (lzcnt + 2)) >> expWidth;
  sig = ii << lzcnt;  // left align number
  sig &= 0x3fffffffffffffffLL;  // chop off leading one
  if (expWidth > 0) {
    exp = (posWidth - (lzcnt + 2LL)) & ((1LL << expWidth) - 1LL);
    ones = -1LL;
    ones <<= ((expWidth + 1LL) + 1LL);
    tmp.low = sig << 3LL;
    tmp.high = (sig >> 63LL) | (exp << 1LL) | ones;
  }
  else {
    ones = -1LL;
    ones <<= (1 + 1);
    tmp.low = sig << 3LL;
    tmp.high = (sig >> 63LL) | ones;
  }
  rgm_sh = rgm + 2;
  Int128::Shr(&tmp1, &tmp, rgm_sh);
  // Get least significant, guard and round bits
  Int128::Shr(&L, &tmp, rgm_sh + expWidth);
  L.low &= 1LL;
  L.high = 0LL;
  Int128::Shr(&G, &tmp, rgm_sh + expWidth - 1);
  G.low &= 1LL;
  G.high = 0LL;
  Int128::Shr(&R, &tmp, rgm_sh + expWidth - 2);
  R.low &= 1LL;
  R.high = 0LL;
  // Calc sticky bit
  S = 0LL;
  for (nn = 0; nn < posWidth; nn++) {
    if (nn < rgm_sh - 2 + expWidth)
      S = S | ((tmp.low >> nn) & 1LL);
  }
  Int128::Lsr(&tmp2, &tmp1, expWidth + 2LL);
  rnd_ulp = ((G.low & (R.low | S)) | (L.low & G.low & ~(R.low | S)));
  tmp3.low = rnd_ulp;
  tmp3.high = 0LL;
  Int128::Add(&tmp2_rnd, &tmp2, &tmp3);
  tmp2_rnd.low &= 0x7fffffffffffffffLL;
  if (i == 0)
    pst.val = 0;
  else if (i < 0)
    pst.val = -tmp2_rnd.low;
  else
    pst.val = tmp2_rnd.low;
  return (pst);
}

int64_t Posit64::PositToInt(Posit64 p)
{
  return (0);
}

void Posit64::Decompose(Posit64 a, RawPosit* b)
{
  uint64_t n;
  int8_t rgmlen;
  int8_t exp;
  int8_t sign;

  sign = (a.val >> 63LL) & 1LL;
  n = a.val < 0 ? -a.val : a.val;
  rgmlen = countLeadingBits(n << 1LL) + 1;
  exp = (n >> (posWidth - rgmlen - expWidth)) & ((1 << expWidth) - 1);
  b->Size(3, 64);
  b->isNaR = a.val == 0x8000000000000000LL;
  b->isInf = a.val == 0x8000000000000000LL;
  b->isZero = a.val == 0LL;
  b->sign = sign;
  b->exp = exp;
  b->regsign = (n >> (posWidth - 2)) & 1LL;
  b->regime = b->regsign ? rgmlen : rgmlen - 1;
  b->sigWidth = max(0, posWidth - rgmlen - expWidth);
  // Significand is left aligned.
  if (posWidth - rgmlen - expWidth - 1 >= 0) {
    n = n << rgmlen;
    n = n & ((1LL << (posWidth - 2LL - expWidth)) - 1LL);
    if (!b->isZero)
      n = n | (1LL << posWidth-2LL-expWidth);
    b->sig.low = n;
    b->sig.high = 0LL;
  }
  else
    Int128::Assign(&b->sig, &Int128::Convert(0x8000000000000000LL));
}

char* Posit64::ToString()
{
  static char buf[20];
  sprintf_s(buf, sizeof(buf), "%08LX", val);
  return (buf);
}
