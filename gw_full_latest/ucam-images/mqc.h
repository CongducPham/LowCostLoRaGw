
/************************************************************************
 *									*
 *	Fast DCT for image compression scheme				*
 *									*
 *	Author: Vincent LECUIRE, CRAN UMR 7039, Nancy-Université, CNRS	*
 *	Date: march, 16 2012						*
 *									*
 ************************************************************************/
 
 /**
@file mqc.h
@brief Implementation of an MQ-Coder (MQC)
*/

/** @defgroup MQC MQC - Implementation of an MQ-Coder */
/*@{*/

/******************************************
This struct defines the state of a context.
*******************************************/
typedef struct opj_mqc_state {
	/** the probability of the Least Probable Symbol (0.75->0x8000, 1.5->0xffff) */
	unsigned int qeval;
	/** the Most Probable Symbol (0 or 1) */
	int mps;
	/** next state if the next encoded symbol is the MPS */
	struct opj_mqc_state *nmps;
	/** next state if the next encoded symbol is the LPS */
	struct opj_mqc_state *nlps;
} opj_mqc_state_t;

#define MQC_NUMCTXS 265

/********
MQ coder
*********/
typedef struct opj_mqc {
	unsigned int c;
	unsigned int a;
	unsigned int ct;
	unsigned char *bp;
	unsigned char *start;
	unsigned char *end;
	opj_mqc_state_t *ctxs[MQC_NUMCTXS];
	opj_mqc_state_t **curctx;
} opj_mqc_t;


/* ----------------------------------------------------------------------- */
/** @name Exported functions */
/**
/**
Return the number of bytes written/read since initialisation
@param mqc MQC handle
@return Returns the number of bytes already encoded
*/
int mqc_numbytes(opj_mqc_t *mqc);
/**
Reset the states of all the context of the coder/decoder 
(each context is set to a state where 0 and 1 are more or less equiprobable)
@param mqc MQC handle
*/
void mqc_resetstates(opj_mqc_t *mqc);
/**
Initialize the encoder
@param mqc MQC handle
@param bp Pointer to the start of the buffer where the bytes will be written
*/
void mqc_init_enc(opj_mqc_t *mqc, unsigned char *bp);
/**
Set the current context used for coding/decoding
@param mqc MQC handle
@param ctxno Number that identifies the context
*/
#define mqc_setcurctx(mqc, ctxno)	(mqc)->curctx = &(mqc)->ctxs[(int)(ctxno)]
/**
Encode a symbol using the MQ-coder
@param mqc MQC handle
@param d The symbol to be encoded (0 or 1)
*/
void mqc_encode(opj_mqc_t *mqc, int d);
/**
Flush the encoder, so that all remaining data is written
@param mqc MQC handle
*/
void mqc_flush(opj_mqc_t *mqc);
/**
Initialize the decoder
@param mqc MQC handle
@param bp Pointer to the start of the buffer from which the bytes will be read
@param len Length of the input buffer
*/
void mqc_init_dec(opj_mqc_t *mqc, unsigned char *bp, int len);
/**
Decode a symbol
@param mqc MQC handle
@return Returns the decoded symbol (0 or 1)
*/
int mqc_decode(opj_mqc_t *mqc);


/* ----------------------------------------------------------------------- */
/** @name Local static functions */
/**
/**
Output a byte, doing bit-stuffing if necessary.
After a 0xff byte, the next byte must be smaller than 0x90.
@param mqc MQC handle
*/
static void mqc_byteout(opj_mqc_t *mqc);
/**
Renormalize mqc->a and mqc->c while encoding, so that mqc->a stays between 0x8000 and 0x10000
@param mqc MQC handle
*/
static void mqc_renorme(opj_mqc_t *mqc);
/**
Encode the most probable symbol
@param mqc MQC handle
*/
static void mqc_codemps(opj_mqc_t *mqc);
/**
Encode the most least symbol
@param mqc MQC handle
*/
static void mqc_codelps(opj_mqc_t *mqc);
/**
Fill mqc->c with 1's for flushing
@param mqc MQC handle
*/
static void mqc_setbits(opj_mqc_t *mqc);
/**
FIXME: documentation ???
@param mqc MQC handle
@return 
*/
static int mqc_mpsexchange(opj_mqc_t *mqc);
/**
FIXME: documentation ???
@param mqc MQC handle
@return 
*/
static int mqc_lpsexchange(opj_mqc_t *mqc);
/**
Input a byte
@param mqc MQC handle
*/
static void mqc_bytein(opj_mqc_t *mqc);
/**
Renormalize mqc->a and mqc->c while decoding
@param mqc MQC handle
*/
static void mqc_renormd(opj_mqc_t *mqc);


/* --------------------------------------------------------- */
/* This array defines all the possible states for a context. */
/** 
/**
/**/
static opj_mqc_state_t mqc_states[47 * 2] = {
	{0x5601, 0, &mqc_states[2], &mqc_states[3]},
	{0x5601, 1, &mqc_states[3], &mqc_states[2]},
	{0x3401, 0, &mqc_states[4], &mqc_states[12]},
	{0x3401, 1, &mqc_states[5], &mqc_states[13]},
	{0x1801, 0, &mqc_states[6], &mqc_states[18]},
	{0x1801, 1, &mqc_states[7], &mqc_states[19]},
	{0x0ac1, 0, &mqc_states[8], &mqc_states[24]},
	{0x0ac1, 1, &mqc_states[9], &mqc_states[25]},
	{0x0521, 0, &mqc_states[10], &mqc_states[58]},
	{0x0521, 1, &mqc_states[11], &mqc_states[59]},
	{0x0221, 0, &mqc_states[76], &mqc_states[66]},
	{0x0221, 1, &mqc_states[77], &mqc_states[67]},
	{0x5601, 0, &mqc_states[14], &mqc_states[13]},
	{0x5601, 1, &mqc_states[15], &mqc_states[12]},
	{0x5401, 0, &mqc_states[16], &mqc_states[28]},
	{0x5401, 1, &mqc_states[17], &mqc_states[29]},
	{0x4801, 0, &mqc_states[18], &mqc_states[28]},
	{0x4801, 1, &mqc_states[19], &mqc_states[29]},
	{0x3801, 0, &mqc_states[20], &mqc_states[28]},
	{0x3801, 1, &mqc_states[21], &mqc_states[29]},
	{0x3001, 0, &mqc_states[22], &mqc_states[34]},
	{0x3001, 1, &mqc_states[23], &mqc_states[35]},
	{0x2401, 0, &mqc_states[24], &mqc_states[36]},
	{0x2401, 1, &mqc_states[25], &mqc_states[37]},
	{0x1c01, 0, &mqc_states[26], &mqc_states[40]},
	{0x1c01, 1, &mqc_states[27], &mqc_states[41]},
	{0x1601, 0, &mqc_states[58], &mqc_states[42]},
	{0x1601, 1, &mqc_states[59], &mqc_states[43]},
	{0x5601, 0, &mqc_states[30], &mqc_states[29]},
	{0x5601, 1, &mqc_states[31], &mqc_states[28]},
	{0x5401, 0, &mqc_states[32], &mqc_states[28]},
	{0x5401, 1, &mqc_states[33], &mqc_states[29]},
	{0x5101, 0, &mqc_states[34], &mqc_states[30]},
	{0x5101, 1, &mqc_states[35], &mqc_states[31]},
	{0x4801, 0, &mqc_states[36], &mqc_states[32]},
	{0x4801, 1, &mqc_states[37], &mqc_states[33]},
	{0x3801, 0, &mqc_states[38], &mqc_states[34]},
	{0x3801, 1, &mqc_states[39], &mqc_states[35]},
	{0x3401, 0, &mqc_states[40], &mqc_states[36]},
	{0x3401, 1, &mqc_states[41], &mqc_states[37]},
	{0x3001, 0, &mqc_states[42], &mqc_states[38]},
	{0x3001, 1, &mqc_states[43], &mqc_states[39]},
	{0x2801, 0, &mqc_states[44], &mqc_states[38]},
	{0x2801, 1, &mqc_states[45], &mqc_states[39]},
	{0x2401, 0, &mqc_states[46], &mqc_states[40]},
	{0x2401, 1, &mqc_states[47], &mqc_states[41]},
	{0x2201, 0, &mqc_states[48], &mqc_states[42]},
	{0x2201, 1, &mqc_states[49], &mqc_states[43]},
	{0x1c01, 0, &mqc_states[50], &mqc_states[44]},
	{0x1c01, 1, &mqc_states[51], &mqc_states[45]},
	{0x1801, 0, &mqc_states[52], &mqc_states[46]},
	{0x1801, 1, &mqc_states[53], &mqc_states[47]},
	{0x1601, 0, &mqc_states[54], &mqc_states[48]},
	{0x1601, 1, &mqc_states[55], &mqc_states[49]},
	{0x1401, 0, &mqc_states[56], &mqc_states[50]},
	{0x1401, 1, &mqc_states[57], &mqc_states[51]},
	{0x1201, 0, &mqc_states[58], &mqc_states[52]},
	{0x1201, 1, &mqc_states[59], &mqc_states[53]},
	{0x1101, 0, &mqc_states[60], &mqc_states[54]},
	{0x1101, 1, &mqc_states[61], &mqc_states[55]},
	{0x0ac1, 0, &mqc_states[62], &mqc_states[56]},
	{0x0ac1, 1, &mqc_states[63], &mqc_states[57]},
	{0x09c1, 0, &mqc_states[64], &mqc_states[58]},
	{0x09c1, 1, &mqc_states[65], &mqc_states[59]},
	{0x08a1, 0, &mqc_states[66], &mqc_states[60]},
	{0x08a1, 1, &mqc_states[67], &mqc_states[61]},
	{0x0521, 0, &mqc_states[68], &mqc_states[62]},
	{0x0521, 1, &mqc_states[69], &mqc_states[63]},
	{0x0441, 0, &mqc_states[70], &mqc_states[64]},
	{0x0441, 1, &mqc_states[71], &mqc_states[65]},
	{0x02a1, 0, &mqc_states[72], &mqc_states[66]},
	{0x02a1, 1, &mqc_states[73], &mqc_states[67]},
	{0x0221, 0, &mqc_states[74], &mqc_states[68]},
	{0x0221, 1, &mqc_states[75], &mqc_states[69]},
	{0x0141, 0, &mqc_states[76], &mqc_states[70]},
	{0x0141, 1, &mqc_states[77], &mqc_states[71]},
	{0x0111, 0, &mqc_states[78], &mqc_states[72]},
	{0x0111, 1, &mqc_states[79], &mqc_states[73]},
	{0x0085, 0, &mqc_states[80], &mqc_states[74]},
	{0x0085, 1, &mqc_states[81], &mqc_states[75]},
	{0x0049, 0, &mqc_states[82], &mqc_states[76]},
	{0x0049, 1, &mqc_states[83], &mqc_states[77]},
	{0x0025, 0, &mqc_states[84], &mqc_states[78]},
	{0x0025, 1, &mqc_states[85], &mqc_states[79]},
	{0x0015, 0, &mqc_states[86], &mqc_states[80]},
	{0x0015, 1, &mqc_states[87], &mqc_states[81]},
	{0x0009, 0, &mqc_states[88], &mqc_states[82]},
	{0x0009, 1, &mqc_states[89], &mqc_states[83]},
	{0x0005, 0, &mqc_states[90], &mqc_states[84]},
	{0x0005, 1, &mqc_states[91], &mqc_states[85]},
	{0x0001, 0, &mqc_states[90], &mqc_states[86]},
	{0x0001, 1, &mqc_states[91], &mqc_states[87]},
	{0x5601, 0, &mqc_states[92], &mqc_states[92]},
	{0x5601, 1, &mqc_states[93], &mqc_states[93]},
};

/* 
==========================================================
   local functions
==========================================================
*/
static void mqc_byteout(opj_mqc_t *mqc) {
	if (*mqc->bp == 0xff) {
		mqc->bp++;
		*mqc->bp = mqc->c >> 20;
		mqc->c &= 0xfffff;
		mqc->ct = 7;
	} else {
		if ((mqc->c & 0x8000000) == 0) {
			mqc->bp++;
			*mqc->bp = mqc->c >> 19;
			mqc->c &= 0x7ffff;
			mqc->ct = 8;
		} else {
			(*mqc->bp)++;
			if (*mqc->bp == 0xff) {
				mqc->c &= 0x7ffffff;
				mqc->bp++;
				*mqc->bp = mqc->c >> 20;
				mqc->c &= 0xfffff;
				mqc->ct = 7;
			} else {
				mqc->bp++;
				*mqc->bp = mqc->c >> 19;
				mqc->c &= 0x7ffff;
				mqc->ct = 8;
			}
		}
	}
}

static void mqc_renorme(opj_mqc_t *mqc) {
	do {
		mqc->a <<= 1;
		mqc->c <<= 1;
		mqc->ct--;
		if (mqc->ct == 0) {
			mqc_byteout(mqc);
		}
	} while ((mqc->a & 0x8000) == 0);
}

static void mqc_codemps(opj_mqc_t *mqc) {
	mqc->a -= (*mqc->curctx)->qeval;
	if ((mqc->a & 0x8000) == 0) {
		if (mqc->a < (*mqc->curctx)->qeval) {
			mqc->a = (*mqc->curctx)->qeval;
		} else {
			mqc->c += (*mqc->curctx)->qeval;
		}
		*mqc->curctx = (*mqc->curctx)->nmps;
		mqc_renorme(mqc);
	} else {
		mqc->c += (*mqc->curctx)->qeval;
	}
}

static void mqc_codelps(opj_mqc_t *mqc) {
	mqc->a -= (*mqc->curctx)->qeval;
	if (mqc->a < (*mqc->curctx)->qeval) {
		mqc->c += (*mqc->curctx)->qeval;
	} else {
		mqc->a = (*mqc->curctx)->qeval;
	}
	*mqc->curctx = (*mqc->curctx)->nlps;
	mqc_renorme(mqc);
}

static void mqc_setbits(opj_mqc_t *mqc) {
	unsigned int tempc = mqc->c + mqc->a;
	mqc->c |= 0xffff;
	if (mqc->c >= tempc) {
		mqc->c -= 0x8000;
	}
}

static int mqc_mpsexchange(opj_mqc_t *mqc) {
	int d;
	if (mqc->a < (*mqc->curctx)->qeval) {
		d = 1 - (*mqc->curctx)->mps;
		*mqc->curctx = (*mqc->curctx)->nlps;
	} else {
		d = (*mqc->curctx)->mps;
		*mqc->curctx = (*mqc->curctx)->nmps;
	}
	
	return d;
}

static int mqc_lpsexchange(opj_mqc_t *mqc) {
	int d;
	if (mqc->a < (*mqc->curctx)->qeval) {
		mqc->a = (*mqc->curctx)->qeval;
		d = (*mqc->curctx)->mps;
		*mqc->curctx = (*mqc->curctx)->nmps;
	} else {
		mqc->a = (*mqc->curctx)->qeval;
		d = 1 - (*mqc->curctx)->mps;
		*mqc->curctx = (*mqc->curctx)->nlps;
	}
	
	return d;
}

static void mqc_bytein(opj_mqc_t *mqc) {
	if (mqc->bp != mqc->end) {
		unsigned int c;
		if (mqc->bp + 1 != mqc->end) {
			c = *(mqc->bp + 1);
		} else {
			c = 0xff;
		}
		if (*mqc->bp == 0xff) {
			if (c > 0x8f) {
				mqc->c += 0xff00;
				mqc->ct = 8;
			} else {
				mqc->bp++;
				mqc->c += c << 9;
				mqc->ct = 7;
			}
		} else {
			mqc->bp++;
			mqc->c += c << 8;
			mqc->ct = 8;
		}
	} else {
		mqc->c += 0xff00;
		mqc->ct = 8;
	}
}

static void mqc_renormd(opj_mqc_t *mqc) {
	do {
		if (mqc->ct == 0) {
			mqc_bytein(mqc);
		}
		mqc->a <<= 1;
		mqc->c <<= 1;
		mqc->ct--;
	} while (mqc->a < 0x8000);
}

/* 
==========================================================
   MQ-Coder interface
==========================================================
*/

int mqc_numbytes(opj_mqc_t *mqc) {
	return mqc->bp - mqc->start;
}

void mqc_init_enc(opj_mqc_t *mqc, unsigned char *bp) {
	mqc_setcurctx(mqc, 0);
	mqc->a = 0x8000;
	mqc->c = 0;
	mqc->bp = bp-1;
	mqc->ct = 12;
	if (*mqc->bp == 0xff) {	mqc->ct = 13;	}
	mqc->start = bp;
}

void mqc_encode(opj_mqc_t *mqc, int d) {
	if ((*mqc->curctx)->mps == d) {	mqc_codemps(mqc); }
				else  {	mqc_codelps(mqc); }
}

void mqc_flush(opj_mqc_t *mqc) {
	mqc_setbits(mqc);
	mqc->c <<= mqc->ct;
	mqc_byteout(mqc);
	mqc->c <<= mqc->ct;
	mqc_byteout(mqc);
	
	if (*mqc->bp != 0xff) {	mqc->bp++; }
}

void mqc_init_dec(opj_mqc_t *mqc, unsigned char *bp, int len) {
	mqc_setcurctx(mqc, 0);
	mqc->start = bp;
	mqc->end = bp + len;
	mqc->bp = bp;
	if (len==0) mqc->c = 0xff << 16;
	else mqc->c = *mqc->bp << 16;
	mqc_bytein(mqc);
	mqc->c <<= 7;
	mqc->ct -= 7;
	mqc->a = 0x8000;
}

int mqc_decode(opj_mqc_t *mqc) {
	int d;
	mqc->a -= (*mqc->curctx)->qeval;
	if ((mqc->c >> 16) < (*mqc->curctx)->qeval) {
		d = mqc_lpsexchange(mqc);
		mqc_renormd(mqc);
	} else {
		mqc->c -= (*mqc->curctx)->qeval << 16;
		if ((mqc->a & 0x8000) == 0) {
			 d = mqc_mpsexchange(mqc);
			 mqc_renormd(mqc);
		} else { d = (*mqc->curctx)->mps; }
	}
	return d;
}

void mqc_resetstates(opj_mqc_t *mqc) {
	int i;
	for (i = 0; i < MQC_NUMCTXS; i++) {
		mqc->ctxs[i] = mqc_states;
	}
}

void mqc_backup(opj_mqc_t *mqc, opj_mqc_t *mcqb, unsigned char *bufb) {
	mcqb->c = mqc->c;
	mcqb->a = mqc->a;
	mcqb->ct = mqc->ct;
	mcqb->bp = mqc->bp;
	for (int x=0; x<MQC_NUMCTXS; x++) bufb[x]=mqc->start[x];
}
void mqc_restore(opj_mqc_t *mqc, opj_mqc_t *mcqb, unsigned char *bufb) {
	mqc->c = mcqb->c;
	mqc->a = mcqb->a;
	mqc->ct = mcqb->ct;
	mqc->bp = mcqb->bp;
	for (int x=0; x<MQC_NUMCTXS; x++) mqc->start[x]=bufb[x];

}
