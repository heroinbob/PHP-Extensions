// Include the php api
#include <php.h>

// Include types and macros from other files we need.
#include "ext/standard/php_var.h"
#include "ext/session/php_session.h"

// Include our header that defines our method
#include "hackify.h"

// Define the function(s) we want to add.
zend_function_entry hackify_functions[] = {
	PHP_FE(decode_session_to_array, NULL)
	{NULL, NULL, NULL}
};

// hackify_functions is our struct defined above.
// this can be used to specify globals, php.ini info, startup & teardown functions, etc.
zend_module_entry hackify_module_entry = {
	STANDARD_MODULE_HEADER,
	HACKIFY_EXTNAME,
	hackify_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	HACKIFY_VERSION,
	STANDARD_MODULE_PROPERTIES
};

// Install our module
ZEND_GET_MODULE(hackify)

/*
 * Jeff McKenzie - 5/17/2012
 * Decode an encoded session string to an array returned to the script.
 * Returns an array or false otherwise
 */
PHP_FUNCTION(decode_session_to_array) {

	/*
	 * The API for PHP functions passes in the following that are available for our use
	 * int ht,
	 * zval *return_value,      - Defaults to NULL
	 * zval **return_value_ptr, - See above
	 * zval *this_ptr,			- ???
	 * int return_value_used	- ??? probably 0 or 1 based on if I set the other two above
	 */

	// These will contain the data passed in by the calling script
	char *encodedData;
	int  dataLength;

	// Parse the params and fail if the string isn't correct.
	if (
		zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &encodedData, &dataLength) == FAILURE
		|| dataLength <= 0
	   ) {
		// The function parameter did not come in as expected.
		RETURN_FALSE;
	}

	// Set up the return value by creating an array so it at least is an empty array if nothing is added
	array_init(return_value);

	/*
	 * BEGIN modified session.c code ============
	 */

	const char *dataBuffer, *bufferChunk; 		   // We'll use these to process the encoded data
	char *currentVariableName; 					   // name of the variable we're decoding.
	int currentVariableNameLength;				   // length of the name var
	const char *endptr = encodedData + dataLength; // set endptr to the address + length so we know if we've passed the end of the data.
	zval *current;								   // The current zval being worked with in the loop
	int hasValue;
	php_unserialize_data_t varHash;
	zval *http_session_vars;

	//php_printf("decode_session_to_array setup complete. Initializing varHash...\n");

	// Set varHash to php_unserialize_data_t
	PHP_VAR_UNSERIALIZE_INIT(varHash);

	//php_printf("\tDone.\n");

	// Set p to the same address as encoded data
	dataBuffer = encodedData;

	//php_printf("return_value initialized as an array. Entering While...\n");

	while (dataBuffer < endptr) {
		zval **tmp;
		bufferChunk = dataBuffer;

		//php_printf("\tNew iteration started...\n\n");

		// Check to see where the next delimitor is, if there is none then break
		while (*bufferChunk != HACKIFY_DATA_DELIMITOR) {
			if (++bufferChunk >= endptr) {
				//php_printf("\tbufferChunk >= entptr - going to break_outer_loop");
				goto break_outer_loop;
			}
		}

		//php_printf("\tdelimitor found...\n");

		if (dataBuffer[0] == HACKIFY_UNDEF_MARKER) {
			//php_printf("\tdataBuffer contains an undefined marker.\n");

			// The current item is undefined
			dataBuffer++;
			hasValue = 0;
		} else {
			hasValue = 1;
		}

		// Calculate the length of the name of the variable and copy it
		currentVariableNameLength = bufferChunk - dataBuffer;
		currentVariableName = estrndup(dataBuffer, currentVariableNameLength);
		bufferChunk++;

		//php_printf("\tVar Name: '%s' Length: %d\n", currentVariableName, currentVariableNameLength);

		if (zend_hash_find(&EG(symbol_table), currentVariableName, currentVariableNameLength + 1, (void **) &tmp) == SUCCESS) {
			// The var name was defined in the executor global table
			if ((Z_TYPE_PP(tmp) == IS_ARRAY && Z_ARRVAL_PP(tmp) == &EG(symbol_table)) || *tmp == PS(http_session_vars)) {
				// Skip the item because it's an executor global symbol
				//php_printf("\tThe var is going to be skipped.\n");
				goto skip;
			}
		}

		//php_printf("\tExecutor global check complete.\n");

		if (hasValue) {
			//php_printf("\tVar has value, making the current zval pointer a standard zval...");

			// Set up the zval object that'll represent the current item we're processing.
			ALLOC_INIT_ZVAL(current);

			//php_printf(" Done.\n\tUnserializing...");

			// Unserialize the variable and check the result.
			int unserializeResult = php_var_unserialize(
				&current,
				(const unsigned char **) &bufferChunk,
				(const unsigned char *) endptr,
				&varHash
				TSRMLS_CC
			);

			//php_printf(" Done. Result is %d\n", unserializeResult);

			if (unserializeResult) {
				// we decoded the variable so store it.
				// Add the deserialized data to the result array and use the name as the key

				//php_printf("\tAttempting to add item of type (%d) to the result array...", current->type);

				// Gets called from within session.c to set the session var to a given value
				//
				// state_val = current in our case
				// name = currentVariableName in our case
				// namelen = currentVariableNameLength in our case
				//
				//zend_set_hash_symbol(state_val, name, namelen, PZVAL_IS_REF(state_val), 1, Z_ARRVAL_P(PS(http_session_vars)));
				zend_set_hash_symbol(
					current,
					currentVariableName,
					currentVariableNameLength,
					PZVAL_IS_REF(current),
					1,
					Z_ARRVAL_P(return_value)
				);

				//php_printf(" Done. Value has been added.\n");
			}
			else {
				//php_printf("\tphp_var_unserialize was unsuccessful.\n");
			}

			// Free memory associated with the "current" zval - it takes a **zval so pass our *zval as a reference
			zval_ptr_dtor(&current);
			//php_printf("\tCurrent pointer destroyed.\n");
		}
skip:
		//php_printf("\tEnd of iteration\n\n");

		// Since we created a copy of the string we need to release it.
		efree(currentVariableName);
		dataBuffer = bufferChunk;
	}

break_outer_loop:

	//php_printf("\nOut of while loop.\n");
	PHP_VAR_UNSERIALIZE_DESTROY(varHash);

	/*
	 * END modified session.c code ============
	 */

	//php_printf("Function complete.\n");
}
