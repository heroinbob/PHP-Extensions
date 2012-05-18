#define HACKIFY_VERSION	"0.8"
#define HACKIFY_EXTNAME	"hackify"
#define HACKIFY_DATA_DELIMITOR '|'
#define HACKIFY_UNDEF_MARKER '!'

/*
 * Jeff McKenzie - 5/17/2012
 * Decode an encoded session string to an array returned to the script
 */
PHP_FUNCTION(decode_session_to_array);

