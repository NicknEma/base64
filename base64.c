#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define array_count(a) (sizeof(a)/sizeof((a)[0]))

/////////////////
//~ Encoder

static int
base64_encoded_len_from_decoded_len(int decoded_len) {
	// int encoded_len = (decoded_len + (3 - (decoded_len % 3))) / 3 * 4;
	int encoded_len = (decoded_len + 2) / 3 * 4; // We take advantage of integer division rules to obtain the same effect of the above computation.
	
	return encoded_len;
}

static char *base64_encode_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void
base64_encode(char *out, int out_len, char *in, int in_len) {
	int in_index  = 0;
	int out_index = 0;
	
	while (in_index < (in_len - (in_len % 3))) {
		char a =   in[in_index+0]         >> 2;
		char b = ((in[in_index+0] & 0x03) << 4) | (in[in_index+1] >> 4);
		char c = ((in[in_index+1] & 0x0F) << 2) | (in[in_index+2] >> 6);
		char d =   in[in_index+2] & 0x3F;
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = base64_encode_table[c];
		out[out_index+3] = base64_encode_table[d];
		
		in_index  += 3;
		out_index += 4;
	}
	
	if (in_len % 3 == 1) {
		assert(in_index == in_len-1);
		
		char a =  in[in_index]         >> 2;
		char b = (in[in_index] & 0x03) << 4;
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = '=';
		out[out_index+3] = '=';
	} else if (in_len % 3 == 2) {
		assert(in_index == in_len-2);
		
		char a =   in[in_index+0]         >> 2;
		char b = ((in[in_index+0] & 0x03) << 4) | (in[in_index+1] >> 4);
		char c = ((in[in_index+1] & 0x0F) << 2);
		
		out[out_index+0] = base64_encode_table[a];
		out[out_index+1] = base64_encode_table[b];
		out[out_index+2] = base64_encode_table[c];
		out[out_index+3] = '=';
	}
}

/////////////////
//~ Decoder

// Table copypasted from:
//  https://stackoverflow.com/questions/11559203/decode-table-construction-for-base64
static char base64_decode_table[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};

static int
base64_decode(char *out, int out_len, char *in, int in_len) {
	assert(in_len % 4 == 0);
	
	int first_invalid_char = -1;
	
	int in_index  = 0;
	int out_index = 0;
	
	int in_len_not_padded = in_len;
	if (in_len >= 4) {
		if (in[in_len-1] == '=') in_len_not_padded -= 1;
		if (in[in_len-2] == '=') in_len_not_padded -= 1;
	}
	
	while (in_index < in_len_not_padded) {
		char sextets[4];
		for (int i = 0; i < 4; i += 1) {
			assert(in_index+i < in_len);
			
			char table_index = in[in_index+i] - '+';
			if (table_index != -1 && table_index < array_count(base64_decode_table)) {
				sextets[i] = base64_decode_table[table_index];
			} else {
				first_invalid_char = in_index;
				goto invalid_char_err;
			}
		}
		
		out[out_index] = (sextets[0] << 2) | ((sextets[1] & 0x30) >> 4);
		out_index += 1;
		
		if (sextets[2] != -2) {
			out[out_index] = ((sextets[1] & 0x0F) << 4) | (sextets[2] >> 2);
			out_index += 1;
		} else {
			assert(in[in_index+2] == '=');
		}
		
		if (sextets[3] != -2) {
			out[out_index] = ((sextets[2] & 0x03) << 6) | sextets[3];
			out_index += 1;
		} else {
			assert(in[in_index+3] == '=');
		}
		
		in_index += 4;
	}
	
	invalid_char_err:;
	
	return first_invalid_char;
}

/////////////////
//~ Main

#define USAGE_STRING \
"Usage:\n" \
"  base64.exe <text> [-d]\n" \
"\n" \
"Encodes by default. Use optional flag '-d' to decode.\n" \

int main(int argc, char **argv) {
	
	if (argc < 2) {
		puts("Encodes or decodes text to and from base64.\n"
			 USAGE_STRING);
	} else {
		bool should_decode = false;
		
		char **inputs      = malloc(argc * sizeof(char *));
		int    input_count = 0;
		
		for (int argi = 1; argi < argc; argi += 1) {
			if (argv[argi][0] == '-') {
				char *flag = argv[argi];
				if (strcmp(flag, "-d") == 0) {
					if (should_decode) {
						fprintf(stderr, "Ignoring repeated flag '%s'\n", flag);
					} else {
						should_decode = true;
					}
				} else {
					fprintf(stderr, "Ignoring unknown flag '%s'.\n" USAGE_STRING "\n",
							flag);
				}
			} else {
				inputs[input_count] = argv[argi];
				input_count += 1;
			}
		}
		
		if (input_count > 0) {
			if (!should_decode) {
				// Encode inputs:
				
				int max_input_len = strlen(inputs[0]); // Cannot be oob: input_count > 0 when we reach this codepath.
				
				for (int input_index = 1; input_index < input_count; input_index += 1) {
					char *input     = inputs[input_index];
					int   input_len = strlen(input);
					if (max_input_len < input_len) {
						max_input_len = input_len;
					}
				}
				
				int max_output_len = base64_encoded_len_from_decoded_len(max_input_len);
				
				char *output = malloc(max_output_len * sizeof(char));
				
				for (int input_index = 0; input_index < input_count; input_index += 1) {
					char *input     = inputs[input_index];
					int   input_len = strlen(input);
					
					int  output_len = base64_encoded_len_from_decoded_len(input_len);
					
					base64_encode(output, output_len, input, input_len);
					
					printf("%.*s\n", output_len, output);
				}
				
				// Memory intentionally not freed: we know that the program is about to terminate, we let the operating system collect the garbage.
				// free(output);
			} else {
				// Decode inputs:
				
				int max_input_len = strlen(inputs[0]); // Cannot be oob: input_count > 0 when we reach this codepath.
				
				for (int input_index = 1; input_index < input_count; input_index += 1) {
					char *input     = inputs[input_index];
					int   input_len = strlen(input);
					if (max_input_len < input_len) {
						max_input_len = input_len;
					}
				}
				
				int max_output_len = max_input_len / 4 * 3;
				
				char *output = malloc(max_output_len * sizeof(char));
				
				for (int input_index = 0; input_index < input_count; input_index += 1) {
					char *input     = inputs[input_index];
					int   input_len = strlen(input);
					
					if (input_len % 4 == 0) {
						int pad_len = 0;
						if (input_len >= 4) {
							if (input[input_len-1] == '=') pad_len += 1;
							if (input[input_len-2] == '=') pad_len += 1;
						}
						
						int output_len = (input_len / 4 * 3) - pad_len; // I don't know if this is correct for all lengths but it seems good so far.
						
						assert(output_len <= max_output_len);
						
						int first_invalid_char = base64_decode(output, output_len, input, input_len);
						if (first_invalid_char == -1) {
							printf("%.*s\n", output_len, output);
						} else {
							fprintf(stderr, "Invalid input %d, '%c' is not a valid base64 character.\n", input_index + 1, input[first_invalid_char]);
						}
					} else {
						fprintf(stderr, "Invalid input %d, length is %d but it has to be a multiple of 4.\n", input_index + 1, input_len);
					}
				}
				
				// Memory intentionally not freed: we know that the program is about to terminate, we let the operating system collect the garbage.
				// free(output);
			}
		} else {
			fprintf(stderr, "No input text. Try again with some input text.\n");
		}
		
		// Memory intentionally not freed: we know that the program is about to terminate, we let the operating system collect the garbage.
		// free(inputs);
	}
	
	return 0;
}
