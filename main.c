#include <stdio.h>
#include "ilist.h"
#include "parser.h"
#include "scanner.h"


int main ()
{
	int err;
	err = instr_init();
	if(err == INTERNAL_ERROR)
		goto end;
	err = parse();
	if(err != SUCCESS)
		goto end;
	print_all();
	
	end:
	inst_free();
	return err;
}
