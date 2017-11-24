#include <stdio.h>
#include "ilist.h"
#include "parser.h"
#include "scanner.h"


int main ()
	int err;
	err = instr_init();
	if(err == INTERNAL_ERROR)
		goto end;
	err = parse();

	print_all();
	inst_free();
	
	end:
	return err;
}
