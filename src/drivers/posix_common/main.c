#ifdef __cplusplus
extern "C" {
#endif

#include "drivers/posix_common/drv_cmd_args.h"
#include "drivers/posix_common/runtime.h"
#include "run.h"

#include <stdbool.h>
#include <string.h>

static bool _soft_reset;

void posix_soft_reset( void ) {
	_soft_reset = true;
}

int main( int argc, char** argv ) {
	parse_arguments( argc, argv );
	_soft_reset = false;

	while( true ) {
		setup();

		while ( true ) {
			loop();

			if(_soft_reset) {
				_soft_reset = false;
				break;
			}
		}
	}
}

#ifdef __cplusplus
}
#endif
