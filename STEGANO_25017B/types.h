#ifndef TYPES_H        // Header guard to prevent multiple inclusion of this file
#define TYPES_H

/* User defined types */
typedef unsigned int uint;   // Define 'uint' as an alias for 'unsigned int'

/* Status will be used in fn. return type */
typedef enum
{
    e_failure,   // Indicates that the function has failed
    e_success    // Indicates that the function has succeeded
} Status;

/* Enumeration to define the type of operation */
typedef enum
{
    e_encode,       // Represents encoding operation
    e_decode,       // Represents decoding operation
    e_unsupported   // Represents unsupported operation type
} OperationType;

#endif  // End of header guard
