/* machine.c : vm - public domain. */
#include <stdint.h>
#include <stdbool.h>
// #include <stdlib.h>
// #include <math.h>
// #include <string.h>

#define MACHINE_CELL_U64(n) ((machine_cell_t){ .u = (n) })
#define MACHINE_CELL_I64(n) ((machine_cell_t){ .i = (n) })
#define MACHINE_CELL_FLOAT(n) ((machine_cell_t){ .f = (n) })

#define MACHINE_FLAG_Z (1u << 0)
#define MACHINE_FLAG_C (1u << 2)
#define MACHINE_FLAG_N (1u << 4)
#define MACHINE_FLAG_V (1u << 6)

#define COND_IS_EQ /* equal */ (current_machine->s & MACHINE_FLAG_Z)
#define COND_IS_NE /* not-equal */ (!COND_IS_EQ) /* not equal */
#define COND_IS_HI /* unsigned greater-than */ (COND_IS_HS && COND_IS_NE)
#define COND_IS_HS /* unsigned greater-or-equal */ (current_machine->s & MACHINE_FLAG_C)
#define COND_IS_LO /* unsigned less-than */ (!COND_IS_HS)
#define COND_IS_LS /* unsigned less-or-equal */ (COND_IS_EQ || COND_IS_LT)
#define COND_IS_MI /* minus */ (current_machine->s & MACHINE_FLAG_N)
#define COND_IS_PL /* plus */ (!COND_IS_MI)
#define COND_IS_GE /* signed greater-or-equal */ (!COND_IS_MI == !(current_machine->s & MACHINE_FLAG_V))
#define COND_IS_GT /* signed greater-than */ (COND_IS_NE && COND_IS_GE)
#define COND_IS_LT /* signed less-than */ (!COND_IS_GE)
#define COND_IS_LE /* signed less-or-equal */ (COND_IS_EQ || COND_IS_LT)

typedef uint32_t machine_ptr_t;
typedef struct { machine_ptr_t addr, len; } machine_blob_t;
typedef union {
	uint64_t u;
	int64_t i;
	double f;
	machine_blob_t b;
	uint8_t c[sizeof(uint64_t)];
} machine_cell_t;

struct machine_fiber {
		machine_ptr_t
			p, /* program counter */
			s, /* status flags */
			d, /* data stack pointer */
			r, /* return stack pointer */
			a, /* address space identifier */ 	// address space identifier allows task switch without flushing if a==a
			t; /* task state address */
};

struct machine_page {
	machine_ptr_t save; /* save address. 0 = none */
	bool dirty;
	machine_cell_t data[MACHINE_PAGESIZE / sizeof(machine_cell_t)];
};

struct machine {
	struct machine_fiber proc; /* processor state */
	
	struct machine_page
		m, /* memory page (load & store instructions) */
		x, /* executable page */
		d; /* data stack page */
	
	bool fault;
};

enum machine_opcode {
	
	/* Misc */
	OP_FAULT, OP_NOP, OP_LIT, OP_CALL, OP_JUMP,
	OP_TASKSWITCH, OP_BZ, OP_BNZ,
	
	/* Stack */
	OP_DUP, OP_DROP, OP_OVER, OP_SWAP,
	OP_TOR, OP_FROMR,
	
	/* ALU ( UF = update status flags ) */
	OP_ADD, OP_ADD_UF, OP_FADD, OP_FADD_UF,
	OP_SUB, OP_SUB_UF, OP_FSUB, OP_FSUB_UF,
	OP_MUL, OP_MUL_UF, OP_UMUL, OP_UMUL_UF,
	OP_FMUL, OP_FMUL_UF,
	OP_DIVMOD, OP_UDIVMOD, OP_FDIV, OP_FMOD,
	OP_CMP, OP_FCMP,
	// OP_BTST, OP_BSET, OP_BCLR, /* bit test, set, & clear */
	
	/* Memory I/O - word & byte */
	OP_STOREW, OP_LOADW,
	OP_STOREB, OP_LOADB,
	
	/* conditionals */
	OP_EQ_IT,  OP_EQ_ITT, OP_EQ_ITE, OP_EQ_ITTT, OP_EQ_ITTE, OP_EQ_ITEE, // OP_EQ_ITTTT, OP_EQ_ITTTE, OP_EQ_ITTEE, OP_EQ_ITEEE,
	OP_NE_IT,  OP_NE_ITT, OP_NE_ITE, OP_NE_ITTT, OP_NE_ITTE, OP_NE_ITEE, // OP_NE_ITTTT, OP_NE_ITTTE, OP_NE_ITTEE, OP_NE_ITEEE,
	OP_GE_IT,  OP_GE_ITT, OP_GE_ITE, OP_GE_ITTT, OP_GE_ITTE, OP_GE_ITEE, // OP_GE_ITTTT, OP_GE_ITTTE, OP_GE_ITTEE, OP_GE_ITEEE,
	OP_LE_IT,  OP_LE_ITT, OP_LE_ITE, OP_LE_ITTT, OP_LE_ITTE, OP_LE_ITEE, // OP_LE_ITTTT, OP_LE_ITTTE, OP_LE_ITTEE, OP_LE_ITEEE,
	OP_GT_IT,  OP_GT_ITT, OP_GT_ITE, OP_GT_ITTT, OP_GT_ITTE, OP_GT_ITEE, // OP_GT_ITTTT, OP_GT_ITTTE, OP_GT_ITTEE, OP_GT_ITEEE,
	OP_LT_IT,  OP_LT_ITT, OP_LT_ITE, OP_LT_ITTT, OP_LT_ITTE, OP_LT_ITEE, // OP_LT_ITTTT, OP_LT_ITTTE, OP_LT_ITTEE, OP_LT_ITEEE,
};

/******************************************************************************/
static const machine_cell_t bootstrap[MACHINE_PAGESIZE / sizeof(machine_cell_t)] = {
	OP_LIT, 12345, OP_DROP,
	// TODO: implement this
};

/******************************************************************************/

static struct machine *current_mach;
static machine_cell_t top; /* mirror of top of stack */

/******************************************************************************/

static void
validate_stack(void)
{
	// TODO: insure that top is always valid
}

static void
fault(void)
{
	current_mach->fault = true;
}

/* reduce a pointer to point to the start of a page */
static inline machine_ptr_t
on_page(machine_ptr_t addr)
{
	return addr & ~(sizeof(((struct machine_page*)0)->data) - 1);
}

static inline unsigned
page_offset(machine_ptr_t addr)
{
	return addr - on_page(addr);	
}

static inline machine_cell_t
fetch_generic_word(machine_ptr_t addr, struct machine_page *pg)
{
	if (pg->save != on_page(addr)) {
		if (pg->dirty) {
			// TODO: save old page
			pg->dirty = 0;
		}
		// TODO: load page
	}		
	return pg->data[page_offset(addr) / sizeof(pg->data[0])];
}

static machine_cell_t
fetch_data_word(machine_ptr_t addr)
{
	return fetch_generic_word(addr, &current_mach->d);
}

static machine_cell_t
fetch_exec_word(machine_ptr_t addr)
{
	return fetch_generic_word(addr, &current_mach->x);
}

static machine_cell_t
fetch_mem_word(machine_ptr_t addr)
{
	if (current_mach->d.save == current_mach->m.save) {
		// TODO: sync before flushing
	}
	return fetch_generic_word(addr, &current_mach->m);
}

/* return next instruction */
static int
next(void)
{
	machine_cell_t v = fetch_exec_word(current_mach->proc.p);
	// TODO: implement this
	// TODO: support some kind of packing of opcodes. but need to handle opcodes that have arguments like LIT and BZ
	return -1; // ERROR
}

static void
push(machine_cell_t v)
{
	abort(); // TODO
}

/* ofs: 1 = top of stack, 0 = future stack positions */
static machine_cell_t
pick(int ofs)
{
	return fetch_data_word(current_mach->proc.d - ofs);
}

static machine_cell_t
pop(void)
{
	machine_cell_t ret = top;

	// TODO: check stack depth
	// TODO: load page
	top = current_mach->d.data[page_offset(current_mach->proc.d)];
	current_mach->proc.d++;
	return ret;
}

static machine_cell_t
peek(void)
{
	return top;
}

static void
execute(void)
{
	int op = next();
	
	if (op == -1) {
		fault();
		return;
	}
	switch ((enum machine_opcode )op) {
	case OP_DUP:
		push(top);
		break;
	case OP_DROP:
		pop();
		break;
	case OP_ADD:
		push(MACHINE_CELL_U64(pick(2).u + top.u));
		break;
	case OP_SUB:
		push(MACHINE_CELL_U64(pick(2).u - top.u));
		break;
	case OP_MUL:
		push(MACHINE_CELL_I64(pick(2).i * top.i));
		break;
	case OP_UMUL:
		push(MACHINE_CELL_U64(pick(2).u * top.u));
		break;
	}
}

/******************************************************************************/

int
machine_new(struct machine *mach)
{
	memset(mach, 0, sizeof(*mach));
	memcpy(mach->x.data, bootstrap, sizeof(bootstrap));
	mach->x.save = 0; /* zero is special */
}

void
machine_use(struct machine *mach)
{
	current_mach = mach;
	top = pick(1);
}

void
machine_run(struct machine *mach)
{
}


/* callback signature:
 *   void callback(machine_cell_t *top);
 *
 * globals:
 *   struct machine current_state;
 *
 * API:
 *   push(machine_cell_t v);
 *   machine_cell_t pop(void);
 *   check(int depth);
 *   adjust(int depth);
 */
