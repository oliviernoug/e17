/*  Abstract Machine for the Small compiler
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *  Portions Copyright (c) Carsten Haitzler, 2004 <raster@rasterman.com>
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 */
#include "embryo_private.h"

#define JUMPABS(base, ip)     ((Embryo_Cell *)(code + (*ip)))

static void _embryo_byte_swap_16 (unsigned short *v);
static void _embryo_byte_swap_32 (unsigned int *v);
static int  _embryo_native_call  (Embryo_Program *ep, Embryo_Cell index, Embryo_Cell *result, Embryo_Cell *params);
static int  _embryo_func_get     (Embryo_Program *ep, int index, char *funcname);
static int  _embryo_var_get      (Embryo_Program *ep, int index, char *varname, Embryo_Cell *ep_addr);
static int  _embryo_program_init (Embryo_Program *ep, void *code);

static void
_embryo_byte_swap_16(unsigned short *v)
{
   unsigned char *s, t;
   
   s = (unsigned char *)v;
   t = s[0]; s[0] = s[1]; s[1] = t;
}

static void
_embryo_byte_swap_32(unsigned int *v)
{
   unsigned char *s, t;
   
   s = (unsigned char *)v;
   t = s[0]; s[0] = s[3]; s[3] = t;
   t = s[1]; s[1] = s[2]; s[2] = t;
}

static int
_embryo_native_call(Embryo_Program *ep, Embryo_Cell index, Embryo_Cell *result, Embryo_Cell *params)
{
   Embryo_Header    *hdr;
   Embryo_Func_Stub *func_entry;
   Embryo_Native     f;

   hdr = (Embryo_Header *)ep->base;
   func_entry = GETENTRY(hdr, natives, index);
   if ((func_entry->address <= 0) || 
       (func_entry->address > ep->native_calls_size))
     {
	ep->error = EMBRYO_ERROR_CALLBACK;
	return ep->error;
     }
   f = ep->native_calls[func_entry->address - 1];
   if (!f)
     {
	ep->error = EMBRYO_ERROR_CALLBACK;
	return ep->error;
     }
   ep->error = EMBRYO_ERROR_NONE;
   *result = f(ep, params);
   return ep->error;
}

static int
_embryo_func_get(Embryo_Program *ep, int index, char *funcname)
{
   Embryo_Header    *hdr;
   Embryo_Func_Stub *func;
   
   hdr = (Embryo_Header *)ep->code;
   if (index >= (Embryo_Cell)NUMENTRIES(hdr, publics, natives))
     return EMBRYO_ERROR_INDEX;
   
   func = GETENTRY(hdr, publics, index);
   strcpy(funcname, GETENTRYNAME(hdr, func));
   return EMBRYO_ERROR_NONE;
}

static int
_embryo_var_get(Embryo_Program *ep, int index, char *varname, Embryo_Cell *ep_addr)
{

  Embryo_Header    *hdr;
  Embryo_Func_Stub *var;

  hdr=(Embryo_Header *)ep->base;
  if (index >= (Embryo_Cell)NUMENTRIES(hdr, pubvars, tags))
     return EMBRYO_ERROR_INDEX;

  var = GETENTRY(hdr, pubvars, index);
  strcpy(varname, GETENTRYNAME(hdr, var));
  *ep_addr = var->address;
  return EMBRYO_ERROR_NONE;
}

static int
_embryo_program_init(Embryo_Program *ep, void *code)
{
   Embryo_Header    *hdr;
   
   if ((ep->flags & EMBRYO_FLAG_RELOC)) return 1;
   ep->code = (unsigned char *)code;
   hdr = (Embryo_Header *)ep->code;
#ifdef WORDS_BIGENDIAN
   embryo_swap_32((unsigned int *)&hdr->size);
   embryo_swap_16((unsigned short *)&hdr->magic);
   embryo_swap_16((unsigned short *)&hdr->flags);
   embryo_swap_16((unsigned short *)&hdr->defsize);
   embryo_swap_32((unsigned int *)&hdr->cod);
   embryo_swap_32((unsigned int *)&hdr->dat);
   embryo_swap_32((unsigned int *)&hdr->hea);
   embryo_swap_32((unsigned int *)&hdr->stp);
   embryo_swap_32((unsigned int *)&hdr->cip);
   embryo_swap_32((unsigned int *)&hdr->publics);
   embryo_swap_32((unsigned int *)&hdr->natives);
   embryo_swap_32((unsigned int *)&hdr->libraries);
   embryo_swap_32((unsigned int *)&hdr->pubvars);
   embryo_swap_32((unsigned int *)&hdr->tags);
   embryo_swap_32((unsigned int *)&hdr->nametable);
#endif
   
   if (hdr->magic != EMBRYO_MAGIC) return 0;
   if ((hdr->file_version < MIN_FILE_VERSION) ||
      (hdr->ep_version > CUR_FILE_VERSION)) return 0;
   if ((hdr->defsize != sizeof(Embryo_Func_Stub)) && 
      (hdr->defsize != (2 * sizeof(unsigned int)))) return 0;
   if (hdr->defsize == (2 * sizeof(unsigned int)))
     {
	unsigned short *len;
	
	len = (unsigned short*)((unsigned char*)ep->code + hdr->nametable);
#ifdef WORDS_BIGENDIAN
	embryo_swap_16((unsigned short *)len);
#endif	
	if (*len > sNAMEMAX) return 0;
     }
   if (hdr->stp <= 0) return 0;
   if ((hdr->flags & EMBRYO_FLAG_COMPACT)) return 0;
   
#ifdef WORDS_BIGENDIAN
     {
	Embryo_Func_Stub *fs;
	int i, num;
	
	/* also align all addresses in the public function, public variable and */
	/* public tag tables */
	fs = GETENTRY(hdr, publics, 0);
	num = NUMENTRIES(hdr, publics, natives);
	for (i = 0; i < num; i++)
	  {
	     embryo_swap_32(&(fs->address));
	     fs = (Embryo_Func_Stub *)((unsigned char *)fs + hdr->defsize);
	  }
	
	fs = GETENTRY(hdr, pubvars, 0);
	num = NUMENTRIES(hdr, pubvars, tags);
	for (i = 0; i < num; i++)
	  {
	     embryo_swap_32(&(fs->address));
	     fs = (Embryo_Func_Stub *)((unsigned char *)fs + hdr->defsize);
	  }
	
	fs = GETENTRY(hdr, tags, 0);
	num = NUMENTRIES(hdr, tags, nametable);
	for (i = 0; i < num; i++)
	  {
	     embryo_swap_32(&(fs->address));
	     fs = (Embryo_Func_Stub *)((unsigned char *)fs + hdr->defsize);
	  }
     }
#endif   
   ep->flags = EMBRYO_FLAG_RELOC;

#ifdef WORDS_BIGENDIAN
     {
	Embryo_Cell cip, code_size;
	Embryo_Cell *code;
	
	code_size = hdr->dat - hdr->cod;
	code = (Embryo_Cell *)((unsigned char *)ep->code + (int)hdr->cod);
	for (cip = 0; cip < code_size; cip++) embryo_swap_32(&(code[cip]));
     }
#endif  
   /* init native api for handling floating point - default in embryo */
   _embryo_fp_init(ep);
   return 1;
}

/*** EXPORTED CALLS ***/

Embryo_Program *
embryo_program_new(void *data, int size)
{
   Embryo_Program *ep;
   void *code_data;
   
   if (size < (int)sizeof(Embryo_Header)) return NULL;
   
   ep = calloc(1, sizeof(Embryo_Program));
   if (!ep) return NULL;
   
   code_data = malloc(size);
   if (!code_data)
     {
	free(ep);
	return NULL;
     }
   memcpy(code_data, data, size);
   if (_embryo_program_init(ep, code_data)) return ep;
   free(code_data);
   free(ep);
   return NULL;
}

Embryo_Program *
embryo_program_const_new(void *data, int size)
{
   Embryo_Program *ep;
   
   if (size < (int)sizeof(Embryo_Header)) return NULL;
   
   ep = calloc(1, sizeof(Embryo_Program));
   if (!ep) return NULL;
   
   if (_embryo_program_init(ep, data))
     {
	ep->dont_free_code = 1;
	return ep;
     }
   free(ep);
   return NULL;
}

Embryo_Program *
embryo_program_load(char *file)
{
   Embryo_Program *ep;
   Embryo_Header   hdr;
   FILE *f;
   void *program = NULL;
   int program_size = 0;
   
   f = fopen(file, "rb");
   if (!f) return NULL;
   fseek(f, 0, SEEK_END);
   program_size = ftell(f);
   rewind(f);
   if (program_size < (int)sizeof(Embryo_Header))
     {
	fclose(f);
	return NULL;
     }
   if (fread(&hdr, sizeof(Embryo_Header), 1, f) != 1)
     {
	fclose(f);
	return NULL;
     }
   rewind(f);
   embryo_swap_32((unsigned int *)(&hdr.size));
   if ((int)hdr.size < program_size) program_size = hdr.size;
   program = malloc(program_size);
   if (!program)
     {
	fclose(f);
	return NULL;
     }
   if (fread(program, program_size, 1, f) != 1)
     {
	free(program);
	fclose(f);
	return NULL;
     }
   ep = embryo_program_new(program, program_size);
   free(program);
   fclose(f);
   return ep;
}

void
embryo_program_free(Embryo_Program *ep)
{
   int i;
   
   if (ep->base) free(ep->base);
   if ((!ep->dont_free_code) && (ep->code)) free(ep->code);
   if (ep->native_calls) free(ep->native_calls);
   for (i = 0; i < ep->params_size; i++)
     {
	if (ep->params[i].string) free(ep->params[i].string);
	if (ep->params[i].cell_array) free(ep->params[i].cell_array);
     }
   if (ep->params) free(ep->params);
   free(ep);
}

void
embryo_program_native_call_add(Embryo_Program *ep, char *name, Embryo_Cell (*func) (Embryo_Program *ep, Embryo_Cell *params))
{
   Embryo_Func_Stub *func_entry;
   Embryo_Header    *hdr;
   int               i, num;

   if ((ep == NULL ) || (name == NULL) || (func == NULL)) return;
   if (strlen(name) > sEXPMAX) return;
   
   hdr = (Embryo_Header *)ep->code;
   if (hdr->defsize < 1) return;
   num = NUMENTRIES(hdr, natives, libraries);
   if (num <= 0) return;

   ep->native_calls_size++;
   if (ep->native_calls_size > ep->native_calls_alloc)
     {
	Embryo_Native *calls;
	
	ep->native_calls_alloc += 16;
	calls = realloc(ep->native_calls, 
			ep->native_calls_alloc * sizeof(Embryo_Native));
	if (!calls)
	  {
	     ep->native_calls_size--;
	     ep->native_calls_alloc -= 16;
	     return;
	  }
	ep->native_calls = calls;
     }
   ep->native_calls[ep->native_calls_size - 1] = func;
   
   func_entry = GETENTRY(hdr, natives, 0);
   for (i = 0; i < num; i++)
     {
	if (func_entry->address == 0)
	  {
	     char *entry_name;
	     
	     entry_name = GETENTRYNAME(hdr, func_entry);
	     if ((entry_name) && (!strcmp(entry_name, name)))
	       {
		  func_entry->address = ep->native_calls_size;
		  return;
	       }
	  }
	func_entry = 
	  (Embryo_Func_Stub *)((unsigned char *)func_entry + hdr->defsize);
     }
}

void
embryo_program_vm_reset(Embryo_Program *ep)
{
   Embryo_Header *hdr;
   
   if ((!ep) || (!ep->base)) return;
   hdr = (Embryo_Header *)ep->code;
   memcpy(ep->base, hdr, hdr->size);
   *(Embryo_Cell *)(ep->base + (int)hdr->stp - sizeof(Embryo_Cell)) = 0;
   
   ep->hlw = hdr->hea - hdr->dat; /* stack and heap relative to data segment */
   ep->stp = hdr->stp - hdr->dat - sizeof(Embryo_Cell);
   ep->hea = ep->hlw;
   ep->stk = ep->stp;
}

void
embryo_program_vm_push(Embryo_Program *ep)
{
   Embryo_Header *hdr;

   if ((!ep) || (ep->base)) return;
   hdr = (Embryo_Header *)ep->code;
   ep->base = malloc(hdr->stp);
   if (!ep->base) return;
   embryo_program_vm_reset(ep);
}

void
embryo_program_vm_pop(Embryo_Program *ep)
{
   if ((!ep) || (!ep->base)) return;
   free(ep->base);
   ep->base = NULL;
}

void
embryo_swap_16(unsigned short *v)
{
#ifdef WORDS_BIGENDIAN
   _embryo_byte_swap_16(v);
#endif
}

void
embryo_swap_32(unsigned int *v)
{
#ifdef WORDS_BIGENDIAN
   _embryo_byte_swap_32(v);
#endif   
}

Embryo_Function
embryo_program_function_find(Embryo_Program *ep, char *name)
{
   int            first, last, mid, result;
   char           pname[sNAMEMAX + 1];
   Embryo_Header *hdr;

   if (!ep) return EMBRYO_FUNCTION_NONE;
   hdr = (Embryo_Header *)ep->code;
   last = NUMENTRIES(hdr, publics, natives) - 1;
   first = 0;
   /* binary search */
   while (first <= last)
     {
	mid = (first + last) / 2;
	if (_embryo_func_get(ep, mid, pname) == EMBRYO_ERROR_NONE)
	  result = strcmp(pname, name);
	else
	  result = -1;
	if (result > 0) last = mid - 1;
	else if (result < 0) first = mid + 1;
	else return mid;
     }
   return EMBRYO_FUNCTION_NONE;
}

Embryo_Cell
embryo_program_variable_find(Embryo_Program *ep, char *name)
{
   int            first, last, mid, result;
   char           pname[sNAMEMAX + 1];
   Embryo_Cell    paddr;
   Embryo_Header *hdr;

   if (!ep) return EMBRYO_CELL_NONE;
   if (!ep->base) return EMBRYO_CELL_NONE;
   hdr = (Embryo_Header *)ep->base;
   last = NUMENTRIES(hdr, pubvars, tags) - 1;
   first = 0;
   /* binary search */
   while (first <= last)
     {
	mid = (first + last) / 2;
	if (_embryo_var_get(ep, mid, pname, &paddr) == EMBRYO_ERROR_NONE)
	  result = strcmp(pname, name);
	else
	  result = -1;
	if (result > 0) last = mid - 1;
	else if (result < 0) first = mid + 1;
	return paddr;
     }
   return EMBRYO_CELL_NONE;
}

void
embryo_program_error_set(Embryo_Program *ep, int error)
{
   if (!ep) return;
   ep->error = error;
}

int
embryo_program_error_get(Embryo_Program *ep)
{
   if (!ep) return EMBRYO_ERROR_NONE;
   return ep->error;
}

const char *
embryo_error_string_get(int error)
{
   const char *messages[] =
     {
	/* EMBRYO_ERROR_NONE      */ "(none)",
	  /* EMBRYO_ERROR_EXIT      */ "Forced exit",
	  /* EMBRYO_ERROR_ASSERT    */ "Assertion failed",
	  /* EMBRYO_ERROR_STACKERR  */ "Stack/heap collision (insufficient stack size)",
	  /* EMBRYO_ERROR_BOUNDS    */ "Array index out of bounds",
	  /* EMBRYO_ERROR_MEMACCESS */ "Invalid memory access",
	  /* EMBRYO_ERROR_INVINSTR  */ "Invalid instruction",
	  /* EMBRYO_ERROR_STACKLOW  */ "Stack underflow",
	  /* EMBRYO_ERROR_HEAPLOW   */ "Heap underflow",
	  /* EMBRYO_ERROR_CALLBACK  */ "No (valid) native function callback",
	  /* EMBRYO_ERROR_NATIVE    */ "Native function failed",
	  /* EMBRYO_ERROR_DIVIDE    */ "Divide by zero",
	  /* EMBRYO_ERROR_SLEEP     */ "(sleep mode)",
	  /* 13 */                     "(reserved)",
	  /* 14 */                     "(reserved)",
	  /* 15 */                     "(reserved)",
	  /* EMBRYO_ERROR_MEMORY    */ "Out of memory",
	  /* EMBRYO_ERROR_FORMAT    */ "Invalid/unsupported P-code file format",
	  /* EMBRYO_ERROR_VERSION   */ "File is for a newer version of the Embryo_Program",
	  /* EMBRYO_ERROR_NOTFOUND  */ "Native/Public function is not found",
	  /* EMBRYO_ERROR_INDEX     */ "Invalid index parameter (bad entry point)",
	  /* EMBRYO_ERROR_DEBUG     */ "Debugger cannot run",
	  /* EMBRYO_ERROR_INIT      */ "Embryo_Program not initialized (or doubly initialized)",
	  /* EMBRYO_ERROR_USERDATA  */ "Unable to set user data field (table full)",
	  /* EMBRYO_ERROR_INIT_JIT  */ "Cannot initialize the JIT",
	  /* EMBRYO_ERROR_PARAMS    */ "Parameter error",
     };
   if ((error < 0) || (error >= (int)(sizeof(messages) / sizeof(messages[0]))))
     return (const char *)"(unknown)";
   return messages[error];
}

int
embryo_data_string_length_get(Embryo_Program *ep, Embryo_Cell *str_cell)
{
   int            len;
   Embryo_Header *hdr;
   
   if ((!ep) || (!ep->base)) return 0;
   hdr = (Embryo_Header *)ep->base;
   if ((!str_cell) ||
       ((void *)str_cell >= (void *)(ep->base + hdr->stp)) ||
       ((void *)str_cell < (void *)ep->base))
     return 0;
   for (len = 0; str_cell[len] != 0; len++);
   return len;
}

void
embryo_data_string_get(Embryo_Program *ep, Embryo_Cell *str_cell, char *dst)
{
   int            i;
   Embryo_Header *hdr;
   
   if (!dst) return;
   if ((!ep) || (!ep->base))
     {
	dst[0] = 0;
	return;
     }
   hdr = (Embryo_Header *)ep->base;
   if ((!str_cell) ||
       ((void *)str_cell >= (void *)(ep->base + hdr->stp)) ||
       ((void *)str_cell < (void *)ep->base))
     {
	dst[0] = 0;
	return;
     }
   for (i = 0; str_cell[i] != 0; i++) dst[i] = str_cell[i];
   dst[i] = 0;
}

void
embryo_data_string_set(Embryo_Program *ep, char *src, Embryo_Cell *str_cell)
{
   int            i;
   Embryo_Header *hdr;
   
   if (!ep) return;
   if (!ep->base) return;
   hdr = (Embryo_Header *)ep->base;
   if ((!str_cell) ||
       ((void *)str_cell >= (void *)(ep->base + hdr->stp)) ||
       ((void *)str_cell < (void *)ep->base))
     return;
   if (!src)
     {
	str_cell[0] = 0;
	return;
     }
   for (i = 0; src[i] != 0; i++)
     {
	if ((void *)(&(str_cell[i])) >= (void *)(ep->base + hdr->stp)) return;
	else if ((void *)(&(str_cell[i]) == (void *)(ep->base + hdr->stp - 1)))
	  {
	     str_cell[i] = 0;
	     return;
	  }
	str_cell[i] = src[i];
     }
   str_cell[i] = 0;
}

Embryo_Cell *
embryo_data_address_get(Embryo_Program *ep, Embryo_Cell addr)
{
   Embryo_Header *hdr;
   unsigned char *data;
   
   if ((!ep) || (!ep->base)) return NULL;
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   if ((addr < 0) || (addr >= hdr->stp)) return NULL;
   return (Embryo_Cell *)(data + (int)addr);
}

Embryo_Cell
embryo_data_heap_push(Embryo_Program *ep, int cells)
{
   Embryo_Header *hdr;
   unsigned char *data;
   Embryo_Cell    addr;

   if ((!ep) || (!ep->base)) return EMBRYO_CELL_NONE;
   hdr = (Embryo_Header *)ep->base;
   data = ep->base + (int)hdr->dat;
   if (ep->stk - ep->hea - (cells * sizeof(Embryo_Cell)) < STKMARGIN)
     return EMBRYO_CELL_NONE;
   addr = ep->hea;
   ep->hea += (cells * sizeof(Embryo_Cell));
   return addr;
}

void
embryo_data_heap_pop(Embryo_Program *ep, Embryo_Cell down_to)
{
   if (!ep) return;
   if (down_to < 0) down_to = 0;
   if (ep->hea > down_to) ep->hea = down_to;
}

int
embryo_program_run(Embryo_Program *ep, Embryo_Function fn)
{
   Embryo_Header    *hdr;
   Embryo_Func_Stub *func;
   unsigned char    *code, *data;
   Embryo_Cell      pri, alt, stk, frm, hea, hea_start;
   Embryo_Cell      reset_stk, reset_hea, *cip;
   Embryo_UCell     codesize;
   int              i;
   unsigned int     op;
   Embryo_Cell      offs;
   int              num;

   if (!ep) return EMBRYO_PROGRAM_FAIL;
   if (!(ep->flags & EMBRYO_FLAG_RELOC))
     {
	ep->error = EMBRYO_ERROR_INIT;
	return EMBRYO_PROGRAM_FAIL;
     }
   if (!ep->base)
     {
	ep->error = EMBRYO_ERROR_INIT;
	return EMBRYO_PROGRAM_FAIL;
     }
   if (ep->run_count > 0)
     {
	/* return EMBRYO_PROGRAM_BUSY; */
	/* FIXME: test C->vm->C->vm recursion more fully */
	/* it seems to work... just fine!!! - strange! */
     }
   
   /* set up the registers */
   hdr = (Embryo_Header *)ep->base;
   codesize = (Embryo_UCell)(hdr->dat - hdr->cod);
   code = ep->base + (int)hdr->cod;
   data = ep->base + (int)hdr->dat;
   hea_start = hea = ep->hea;
   stk = ep->stk;
   reset_stk = stk;
   reset_hea = hea;
   frm = alt = pri = 0;

   /* get the start address */
   if (fn == EMBRYO_FUNCTION_MAIN)
     {
	if (hdr->cip < 0)
	  {
	     ep->error = EMBRYO_ERROR_INDEX;
	     return EMBRYO_PROGRAM_FAIL;
	  }
	cip = (Embryo_Cell *)(code + (int)hdr->cip);
     }
   else if (fn == EMBRYO_FUNCTION_CONT)
     {
	/* all registers: pri, alt, frm, cip, hea, stk, reset_stk, reset_hea */
	frm = ep->frm;
	stk = ep->stk;
	hea = ep->hea;
	pri = ep->pri;
	alt = ep->alt;
	reset_stk = ep->reset_stk;
	reset_hea = ep->reset_hea;
	cip = (Embryo_Cell *)(code + (int)ep->cip);
     }
   else if (fn < 0)
     {
	ep->error = EMBRYO_ERROR_INDEX;
	return EMBRYO_PROGRAM_FAIL;
     }
   else
     {
	if (fn >= (Embryo_Cell)NUMENTRIES(hdr, publics, natives))
	  {
	     ep->error = EMBRYO_ERROR_INDEX;
	     return EMBRYO_PROGRAM_FAIL;
	  }
	func = GETENTRY(hdr, publics, fn);
	cip = (Embryo_Cell *)(code + (int)func->address);
     }
   /* check values just copied */
   CHKSTACK();
   CHKHEAP();

   if (fn != EMBRYO_FUNCTION_CONT)
     {
	int i;
	
	for (i = ep->params_size - 1; i >= 0; i--)
	  {
	     Embryo_Param *pr;
	     
	     pr = &(ep->params[i]);
	     if (pr->string)
	       {
		  int len;
		  Embryo_Cell ep_addr, *addr;
		  
		  len = strlen(pr->string);
		  ep_addr = embryo_data_heap_push(ep, len + 1);
		  if (ep_addr == EMBRYO_CELL_NONE)
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  addr = embryo_data_address_get(ep, ep_addr);
		  if (addr)
		    embryo_data_string_set(ep, pr->string, addr);
		  else
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  PUSH(ep_addr);
		  free(pr->string);
	       }
	     else if (pr->cell_array)
	       {
		  int len;
		  Embryo_Cell ep_addr, *addr;
		  
		  len = strlen(pr->string);
		  ep_addr = embryo_data_heap_push(ep, len + 1);
		  if (ep_addr == EMBRYO_CELL_NONE)
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  addr = embryo_data_address_get(ep, ep_addr);
		  if (addr)
		    memcpy(addr, pr->cell_array, 
			   pr->cell_array_size * sizeof(Embryo_Cell));
		  else
		    {
		       ep->error = EMBRYO_ERROR_HEAPLOW;
		       return EMBRYO_PROGRAM_FAIL;
		    }
		  PUSH(ep_addr);
		  free(pr->cell_array);
	       }
	     else
	       {
		  PUSH(pr->cell);
	       }
	  }
	PUSH(ep->params_size * sizeof(Embryo_Cell));
	PUSH(0);
	if (ep->params)
	  {
	     free(ep->params);
	     ep->params = NULL;
	  }
	ep->params_size = ep->params_alloc = 0;
     }
   /* check stack/heap before starting to run */
   CHKMARGIN();

   /* track recursion depth */
   ep->run_count++;
   
   /* start running */
   for (;;)
     {
	op = (Embryo_Opcode)*cip++;
	switch (op)
	  {
	   case EMBRYO_OP_LOAD_PRI:
	     GETPARAM(offs);
	     pri = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LOAD_ALT:
	     GETPARAM(offs);
	     alt = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LOAD_S_PRI:
	     GETPARAM(offs);
	     pri = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	     break;
	   case EMBRYO_OP_LOAD_S_ALT:
	     GETPARAM(offs);
	     alt = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	     break;
	   case EMBRYO_OP_LREF_PRI:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)offs);
	     pri = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LREF_ALT:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)offs);
	     alt = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LREF_S_PRI:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	     pri = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LREF_S_ALT:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	     alt = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LOAD_I:
	     CHKMEM(pri);
	     pri = *(Embryo_Cell *)(data + (int)pri);
	     break;
	   case EMBRYO_OP_LODB_I:
	     GETPARAM(offs);
	     CHKMEM(pri);
	     switch (offs)
	       {
		case 1:
		  pri = *(data + (int)pri);
		  break;
		case 2:
		  pri = *(unsigned short *)(data + (int)pri);
		  break;
		case 4:
		  pri = *(unsigned int *)(data + (int)pri);
		  break;
		default:
		  ABORT(ep, EMBRYO_ERROR_INVINSTR);
		  break;
	       }
	     break;
	   case EMBRYO_OP_CONST_PRI:
	     GETPARAM(pri);
	     break;
	   case EMBRYO_OP_CONST_ALT:
	     GETPARAM(alt);
	     break;
	   case EMBRYO_OP_ADDR_PRI:
	     GETPARAM(pri);
	     pri += frm;
	     break;
	   case EMBRYO_OP_ADDR_ALT:
	     GETPARAM(alt);
	     alt += frm;
	     break;
	   case EMBRYO_OP_STOR_PRI:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)offs) = pri;
	     break;
	   case EMBRYO_OP_STOR_ALT:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)offs) = alt;
	     break;
	   case EMBRYO_OP_STOR_S_PRI:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)frm + (int)offs) = pri;
	     break;
	   case EMBRYO_OP_STOR_S_ALT:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)frm + (int)offs) = alt;
	     break;
	   case EMBRYO_OP_SREF_PRI:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)offs);
	     *(Embryo_Cell *)(data + (int)offs) = pri;
	     break;
	   case EMBRYO_OP_SREF_ALT:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)offs);
	     *(Embryo_Cell *)(data + (int)offs) = alt;
	     break;
	   case EMBRYO_OP_SREF_S_PRI:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	     *(Embryo_Cell *)(data + (int)offs) = pri;
	     break;
	   case EMBRYO_OP_SREF_S_ALT:
	     GETPARAM(offs);
	     offs = *(Embryo_Cell *)(data + (int)frm + (int)offs);
	     *(Embryo_Cell *)(data + (int)offs) = alt;
	     break;
	   case EMBRYO_OP_STOR_I:
	     CHKMEM(alt);
	     *(Embryo_Cell *)(data + (int)alt) = pri;
	     break;
	   case EMBRYO_OP_STRB_I:
	     GETPARAM(offs);
	     CHKMEM(alt);
	     switch (offs)
	       {
		case 1:
		  *(data + (int)alt) = (unsigned char)pri;
		  break;
		case 2:
		  *(unsigned short *)(data + (int)alt) = (unsigned short)pri;
		  break;
		case 4:
		  *(unsigned int *)(data + (int)alt) = (unsigned int)pri;
		  break;
		default:
		  ABORT(ep, EMBRYO_ERROR_INVINSTR);
		  break;
	       }
	     break;
	   case EMBRYO_OP_LIDX:
	     offs = (pri * sizeof(Embryo_Cell)) + alt;
	     CHKMEM(offs);
	     pri = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_LIDX_B:
	     GETPARAM(offs);
	     offs = (pri << (int)offs) + alt;
	     CHKMEM(offs);
	     pri = *(Embryo_Cell *)(data + (int)offs);
	     break;
	   case EMBRYO_OP_IDXADDR:
	     pri = (pri * sizeof(Embryo_Cell)) + alt;
	     break;
	   case EMBRYO_OP_IDXADDR_B:
	     GETPARAM(offs);
	     pri = (pri << (int)offs) + alt;
	     break;
	   case EMBRYO_OP_ALIGN_PRI:
	     GETPARAM(offs);
#ifdef WORDS_BIGENDIAN
	     if ((size_t)offs < sizeof(Embryo_Cell))
	       pri ^= sizeof(Embryo_Cell) - offs;
#endif       
	     break;
	   case EMBRYO_OP_ALIGN_ALT:
	     GETPARAM(offs);
#ifdef WORDS_BIGENDIAN
	     if ((size_t)offs < sizeof(Embryo_Cell))
	       alt ^= sizeof(Embryo_Cell) - offs;
#endif       
	     break;
	   case EMBRYO_OP_LCTRL:
	     GETPARAM(offs);
	     switch (offs)
	       {
		case 0:
		  pri = hdr->cod;
		  break;
		case 1:
		  pri = hdr->dat;
		  break;
		case 2:
		  pri = hea;
		  break;
		case 3:
		  pri = ep->stp;
		  break;
		case 4:
		  pri = stk;
		  break;
		case 5:
		  pri = frm;
		  break;
		case 6:
		  pri = (Embryo_Cell)((unsigned char *)cip - code);
		  break;
		default:
		  ABORT(ep, EMBRYO_ERROR_INVINSTR);
		  break;
	       }
	     break;
	   case EMBRYO_OP_SCTRL:
	     GETPARAM(offs);
	     switch (offs)
	       {
		case 0:
		case 1:
		case 2:
		  hea = pri;
		  break;
		case 3:
		  /* cannot change these parameters */
		  break;
		case 4:
		  stk = pri;
		  break;
		case 5:
		  frm = pri;
		  break;
		case 6:
		  cip = (Embryo_Cell *)(code + (int)pri);
		  break;
		default:
		  ABORT(ep, EMBRYO_ERROR_INVINSTR);
		  break;
	       }
	     break;
	   case EMBRYO_OP_MOVE_PRI:
	     pri = alt;
	     break;
	   case EMBRYO_OP_MOVE_ALT:
	     alt = pri;
	     break;
	   case EMBRYO_OP_XCHG:
	     offs = pri;         /* offs is a temporary variable */
	     pri = alt;
	     alt = offs;
	     break;
	   case EMBRYO_OP_PUSH_PRI:
	     PUSH(pri);
	     break;
	   case EMBRYO_OP_PUSH_ALT:
	     PUSH(alt);
	     break;
	   case EMBRYO_OP_PUSH_C:
	     GETPARAM(offs);
	     PUSH(offs);
	     break;
	   case EMBRYO_OP_PUSH_R:
	     GETPARAM(offs);
	     while (offs--) PUSH(pri);
	     break;
	   case EMBRYO_OP_PUSH:
	     GETPARAM(offs);
	     PUSH(*(Embryo_Cell *)(data + (int)offs));
	     break;
	   case EMBRYO_OP_PUSH_S:
	     GETPARAM(offs);
	     PUSH(*(Embryo_Cell *)(data + (int)frm + (int)offs));
	     break;
	   case EMBRYO_OP_POP_PRI:
	     POP(pri);
	     break;
	   case EMBRYO_OP_POP_ALT:
	     POP(alt);
	     break;
	   case EMBRYO_OP_STACK:
	     GETPARAM(offs);
	     alt = stk;
	     stk += offs;
	     CHKMARGIN();
	     CHKSTACK();
	     break;
	   case EMBRYO_OP_HEAP:
	     GETPARAM(offs);
	     alt = hea;
	     hea += offs;
	     CHKMARGIN();
	     CHKHEAP();
	     break;
	   case EMBRYO_OP_PROC:
	     PUSH(frm);
	     frm = stk;
	     CHKMARGIN();
	     break;
	   case EMBRYO_OP_RET:
	     POP(frm);
	     POP(offs);
	     if ((Embryo_UCell)offs >= codesize)
	       ABORT(ep, EMBRYO_ERROR_MEMACCESS);
	     cip = (Embryo_Cell *)(code + (int)offs);
	     break;
	   case EMBRYO_OP_RETN:
	     POP(frm);
	     POP(offs);
	     if ((Embryo_UCell)offs >= codesize)
	       ABORT(ep, EMBRYO_ERROR_MEMACCESS);
	     cip = (Embryo_Cell *)(code + (int)offs);
	     stk += *(Embryo_Cell *)(data + (int)stk) + sizeof(Embryo_Cell); /* remove parameters from the stack */
	     ep->stk = stk;
	     break;
	   case EMBRYO_OP_CALL:
	     PUSH(((unsigned char *)cip - code) + sizeof(Embryo_Cell));/* skip address */
	     cip = JUMPABS(code, cip); /* jump to the address */
	     break;
	   case EMBRYO_OP_CALL_PRI:
	     PUSH((unsigned char *)cip - code);
	     cip = (Embryo_Cell *)(code + (int)pri);
	   break;
	   case EMBRYO_OP_JUMP:
	     /* since the GETPARAM() macro modifies cip, you cannot
	      * do GETPARAM(cip) directly */
	     cip = JUMPABS(code, cip);
	     break;
	   case EMBRYO_OP_JREL:
	     offs = *cip;
	     cip = (Embryo_Cell *)((unsigned char *)cip + (int)offs + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JZER:
	     if (pri == 0)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JNZ:
	     if (pri != 0)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JEQ:
	     if (pri==alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JNEQ:
	     if (pri != alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JLESS:
	     if ((Embryo_UCell)pri < (Embryo_UCell)alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JLEQ:
	     if ((Embryo_UCell)pri <= (Embryo_UCell)alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JGRTR:
	     if ((Embryo_UCell)pri > (Embryo_UCell)alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JGEQ:
	     if ((Embryo_UCell)pri >= (Embryo_UCell)alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JSLESS:
	     if (pri < alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JSLEQ:
	     if (pri <= alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JSGRTR:
	     if (pri > alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_JSGEQ:
	     if (pri >= alt)
	       cip = JUMPABS(code, cip);
	     else
	       cip = (Embryo_Cell *)((unsigned char *)cip + sizeof(Embryo_Cell));
	     break;
	   case EMBRYO_OP_SHL:
	     pri <<= alt;
	     break;
	   case EMBRYO_OP_SHR:
	     pri = (Embryo_UCell)pri >> (int)alt;
	     break;
	   case EMBRYO_OP_SSHR:
	     pri >>= alt;
	     break;
	   case EMBRYO_OP_SHL_C_PRI:
	     GETPARAM(offs);
	     pri <<= offs;
	     break;
	   case EMBRYO_OP_SHL_C_ALT:
	     GETPARAM(offs);
	     alt <<= offs;
	     break;
	   case EMBRYO_OP_SHR_C_PRI:
	     GETPARAM(offs);
	     pri = (Embryo_UCell)pri >> (int)offs;
	     break;
	   case EMBRYO_OP_SHR_C_ALT:
	     GETPARAM(offs);
	     alt = (Embryo_UCell)alt >> (int)offs;
	     break;
	   case EMBRYO_OP_SMUL:
	     pri *= alt;
	     break;
	   case EMBRYO_OP_SDIV:
	     if (alt == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	     /* divide must always round down; this is a bit
	      * involved to do in a machine-independent way.
	      */
	     offs = ((pri % alt) + alt) % alt; /* true modulus */
	     pri = (pri - offs) / alt;         /* division result */
	     alt = offs;
	     break;
	   case EMBRYO_OP_SDIV_ALT:
	     if (pri == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	     /* divide must always round down; this is a bit
	      * involved to do in a machine-independent way.
	      */
	     offs = ((alt % pri) + pri) % pri; /* true modulus */
	     pri = (alt - offs) / pri;         /* division result */
	     alt = offs;
	     break;
	   case EMBRYO_OP_UMUL:
	     pri = (Embryo_UCell)pri * (Embryo_UCell)alt;
	     break;
	   case EMBRYO_OP_UDIV:
	     if (alt == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	     offs = (Embryo_UCell)pri % (Embryo_UCell)alt; /* temporary storage */
	     pri = (Embryo_UCell)pri / (Embryo_UCell)alt;
	     alt = offs;
	     break;
	   case EMBRYO_OP_UDIV_ALT:
	     if (pri == 0) ABORT(ep, EMBRYO_ERROR_DIVIDE);
	     offs = (Embryo_UCell)alt % (Embryo_UCell)pri;     /* temporary storage */
	     pri = (Embryo_UCell)alt / (Embryo_UCell)pri;
	     alt = offs;
	     break;
	   case EMBRYO_OP_ADD:
	     pri += alt;
	     break;
	   case EMBRYO_OP_SUB:
	     pri -= alt;
	     break;
	   case EMBRYO_OP_SUB_ALT:
	     pri = alt - pri;
	     break;
	   case EMBRYO_OP_AND:
	     pri &= alt;
	     break;
	   case EMBRYO_OP_OR:
	     pri |= alt;
	     break;
	   case EMBRYO_OP_XOR:
	     pri ^= alt;
	     break;
	   case EMBRYO_OP_NOT:
	     pri = !pri;
	     break;
	   case EMBRYO_OP_NEG:
	     pri = -pri;
	     break;
	   case EMBRYO_OP_INVERT:
	     pri = ~pri;
	     break;
	   case EMBRYO_OP_ADD_C:
	     GETPARAM(offs);
	     pri += offs;
	     break;
	   case EMBRYO_OP_SMUL_C:
	     GETPARAM(offs);
	     pri *= offs;
	     break;
	   case EMBRYO_OP_ZERO_PRI:
	     pri = 0;
	     break;
	   case EMBRYO_OP_ZERO_ALT:
	     alt = 0;
	     break;
	   case EMBRYO_OP_ZERO:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)offs) = 0;
	     break;
	   case EMBRYO_OP_ZERO_S:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)frm + (int)offs) = 0;
	     break;
	   case EMBRYO_OP_SIGN_PRI:
	     if ((pri & 0xff) >= 0x80) pri |= ~(Embryo_UCell)0xff;
	     break;
	   case EMBRYO_OP_SIGN_ALT:
	     if ((alt & 0xff) >= 0x80) alt |= ~(Embryo_UCell)0xff;
	     break;
	   case EMBRYO_OP_EQ:
	     pri = (pri == alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_NEQ:
	     pri = (pri != alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_LESS:
	     pri = ((Embryo_UCell)pri < (Embryo_UCell)alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_LEQ:
	   pri = ((Embryo_UCell)pri <= (Embryo_UCell)alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_GRTR:
	     pri = ((Embryo_UCell)pri > (Embryo_UCell)alt) ? 1 : 0;
	   break;
	   case EMBRYO_OP_GEQ:
	     pri = ((Embryo_UCell)pri >= (Embryo_UCell)alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_SLESS:
	     pri = (pri < alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_SLEQ:
	     pri = (pri <= alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_SGRTR:
	     pri = (pri > alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_SGEQ:
	     pri = (pri >= alt) ? 1 : 0;
	     break;
	   case EMBRYO_OP_EQ_C_PRI:
	     GETPARAM(offs);
	     pri = (pri == offs) ? 1 : 0;
	     break;
	   case EMBRYO_OP_EQ_C_ALT:
	     GETPARAM(offs);
	     pri = (alt == offs) ? 1 : 0;
	     break;
	   case EMBRYO_OP_INC_PRI:
	     pri++;
	     break;
	   case EMBRYO_OP_INC_ALT:
	     alt++;
	     break;
	   case EMBRYO_OP_INC:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)offs) += 1;
	     break;
	   case EMBRYO_OP_INC_S:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)frm + (int)offs) += 1;
	     break;
	   case EMBRYO_OP_INC_I:
	     *(Embryo_Cell *)(data + (int)pri) += 1;
	     break;
	   case EMBRYO_OP_DEC_PRI:
	     pri--;
	     break;
	   case EMBRYO_OP_DEC_ALT:
	     alt--;
	     break;
	   case EMBRYO_OP_DEC:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)offs) -= 1;
	     break;
	   case EMBRYO_OP_DEC_S:
	     GETPARAM(offs);
	     *(Embryo_Cell *)(data + (int)frm + (int)offs) -= 1;
	     break;
	   case EMBRYO_OP_DEC_I:
	     *(Embryo_Cell *)(data + (int)pri) -= 1;
	     break;
	   case EMBRYO_OP_MOVS:
	     GETPARAM(offs);
	     CHKMEM(pri);
	     CHKMEM(pri + offs);
	     CHKMEM(alt);
	     CHKMEM(alt + offs);
	     memcpy(data+(int)alt, data+(int)pri, (int)offs);
	     break;
	   case EMBRYO_OP_CMPS:
	     GETPARAM(offs);
	     CHKMEM(pri);
	     CHKMEM(pri + offs);
	     CHKMEM(alt);
	     CHKMEM(alt + offs);
	     pri = memcmp(data + (int)alt, data + (int)pri, (int)offs);
	     break;
	   case EMBRYO_OP_FILL:
	     GETPARAM(offs);
	     CHKMEM(alt);
	     CHKMEM(alt + offs);
	     for (i = (int)alt; 
		  (size_t)offs >= sizeof(Embryo_Cell); 
		  i += sizeof(Embryo_Cell), offs -= sizeof(Embryo_Cell))
	       *(Embryo_Cell *)(data + i) = pri;
	     break;
	   case EMBRYO_OP_HALT:
	     GETPARAM(offs);
	     ep->retval = pri;
	     /* store complete status */
	     ep->frm = frm;
	     ep->stk = stk;
	     ep->hea = hea;
	     ep->pri = pri;
	     ep->alt = alt;
	     ep->cip = (Embryo_Cell)((unsigned char*)cip - code);
	     if (offs == EMBRYO_ERROR_SLEEP)
	       {
		  ep->reset_stk = reset_stk;
		  ep->reset_hea = reset_hea;
		  ep->run_count--;
		  return EMBRYO_PROGRAM_SLEEP;
	       }
	     OK(ep, (int)offs);
	   case EMBRYO_OP_BOUNDS:
	     GETPARAM(offs);
	     if ((Embryo_UCell)pri > (Embryo_UCell)offs)
	       ABORT(ep, EMBRYO_ERROR_BOUNDS);
	     break;
	   case EMBRYO_OP_SYSREQ_PRI:
	     /* save a few registers */
	     ep->cip = (Embryo_Cell)((unsigned char *)cip - code);
	     ep->hea = hea;
	     ep->frm = frm;
	     ep->stk = stk;
	     num = _embryo_native_call(ep, pri, &pri, (Embryo_Cell *)(data + (int)stk));
	     if (num != EMBRYO_ERROR_NONE)
	       {
		  if (num == EMBRYO_ERROR_SLEEP)
		    {
		       ep->pri = pri;
		       ep->alt = alt;
		       ep->reset_stk = reset_stk;
		       ep->reset_hea = reset_hea;
		       ep->run_count--;
		       return EMBRYO_PROGRAM_SLEEP;
		    }
		  ABORT(ep, num);
	       }
	     break;
	   case EMBRYO_OP_SYSREQ_C:
	     GETPARAM(offs);
	     /* save a few registers */
	     ep->cip = (Embryo_Cell)((unsigned char *)cip - code);
	     ep->hea = hea;
	     ep->frm = frm;
	     ep->stk = stk;
	     num = _embryo_native_call(ep, offs, &pri, (Embryo_Cell *)(data + (int)stk));
	     if (num != EMBRYO_ERROR_NONE)
	       {
		  if (num == EMBRYO_ERROR_SLEEP)
		    {
		       ep->pri = pri;
		       ep->alt = alt;
		       ep->reset_stk = reset_stk;
		       ep->reset_hea = reset_hea;
		       ep->run_count--;
		       return EMBRYO_PROGRAM_SLEEP;
		    }
		  ABORT(ep, num);
	       }
	     break;
	   case EMBRYO_OP_SYSREQ_D:
	     GETPARAM(offs);
	     /* save a few registers */
	     ep->cip = (Embryo_Cell)((unsigned char *)cip - code);
	     ep->hea = hea;
	     ep->frm = frm;
	     ep->stk = stk;
	     pri = ((Embryo_Native)offs)(ep, (Embryo_Cell *)(data + (int)stk));
	     if (ep->error != EMBRYO_ERROR_NONE)
	       {
		  if (ep->error == EMBRYO_ERROR_SLEEP)
		    {
		       ep->pri = pri;
		       ep->alt = alt;
		       ep->reset_stk = reset_stk;
		       ep->reset_hea = reset_hea;
		       ep->run_count--;
		       return EMBRYO_PROGRAM_SLEEP;
		    }
		  ABORT(ep, ep->error);
	       }
	     break;
	   case EMBRYO_OP_JUMP_PRI:
	     cip = (Embryo_Cell *)(code + (int)pri);
	     break;
	   case EMBRYO_OP_SWITCH:
	       {
		  Embryo_Cell *cptr;
		  
		  cptr = (Embryo_Cell *)*cip + 1; /* +1, to skip the "casetbl" opcode */
		  cip = (Embryo_Cell *)*(cptr + 1); /* preset to "none-matched" case */
		  num = (int)*cptr; /* number of records in the case table */
		  for (cptr += 2; (num > 0) && (*cptr != pri); num--, cptr += 2);
		  if (num > 0) cip = (Embryo_Cell *)*(cptr + 1); /* case found */
	       }
	     break;
	   case EMBRYO_OP_SWAP_PRI:
	     offs = *(Embryo_Cell *)(data + (int)stk);
	     *(Embryo_Cell *)(data + (int)stk) = pri;
	     pri = offs;
	     break;
	   case EMBRYO_OP_SWAP_ALT:
	     offs = *(Embryo_Cell *)(data + (int)stk);
	     *(Embryo_Cell *)(data + (int)stk) = alt;
	     alt = offs;
	     break;
	   case EMBRYO_OP_PUSHADDR:
	     GETPARAM(offs);
	     PUSH(frm + offs);
	     break;
	   case EMBRYO_OP_NOP:
	     break;
	   default:
	     ABORT(ep, EMBRYO_ERROR_INVINSTR);
	  }
     }
   ep->run_count--;
   
   //
   ep->hea = hea_start;
   
   return EMBRYO_PROGRAM_OK;
}

Embryo_Cell
embryo_program_return_value_get(Embryo_Program *ep)
{
   if (!ep) return 0;
   return ep->retval;
}

int
embryo_parameter_cell_push(Embryo_Program *ep, Embryo_Cell cell)
{
   Embryo_Param *pr;
   
   ep->params_size++;
   if (ep->params_size > ep->params_alloc)
     {
	ep->params_alloc += 8;
	pr = realloc(ep->params, ep->params_alloc * sizeof(Embryo_Param));
	if (!pr) return 0;
	ep->params = pr;
     }
   pr = &(ep->params[ep->params_size - 1]);
   pr->string = NULL;
   pr->cell_array = NULL;
   pr->cell_array_size = 0;
   pr->cell = 0;
   pr->cell = cell;
   return 1;
}

int
embryo_parameter_string_push(Embryo_Program *ep, char *str)
{
   Embryo_Param *pr;
   char *str_dup;
   
   if (!str)
     return embryo_parameter_string_push(ep, "");
   str_dup = strdup(str);
   if (!str_dup) return 0;
   ep->params_size++;
   if (ep->params_size > ep->params_alloc)
     {
	ep->params_alloc += 8;
	pr = realloc(ep->params, ep->params_alloc * sizeof(Embryo_Param));
	if (!pr)
	  {
	     free(str_dup);
	     return 0;
	  }
	ep->params = pr;
     }
   pr = &(ep->params[ep->params_size - 1]);
   pr->string = NULL;
   pr->cell_array = NULL;
   pr->cell_array_size = 0;
   pr->cell = 0;
   pr->string = str_dup;
   return 1;
}

int
embryo_parameter_cell_array_push(Embryo_Program *ep, Embryo_Cell *cells, int num)
{
   Embryo_Param *pr;
   Embryo_Cell *cell_array;
   
   pr->cell_array = malloc(num * sizeof(Embryo_Cell));
   if ((!cells) || (num <= 0))
     return embryo_parameter_cell_push(ep, 0);
   ep->params_size++;
   if (ep->params_size > ep->params_alloc)
     {
	ep->params_alloc += 8;
	pr = realloc(ep->params, ep->params_alloc * sizeof(Embryo_Param));
	if (!pr)
	  {
	     free(cell_array);
	     return 0;
	  }
	ep->params = pr;
     }
   pr = &(ep->params[ep->params_size - 1]);
   pr->string = NULL;
   pr->cell_array = NULL;
   pr->cell_array_size = 0;
   pr->cell = 0;
   pr->cell_array = cell_array;
   pr->cell_array_size = num;
   memcpy(pr->cell_array, cells, num * sizeof(Embryo_Cell));
   return 1;
}
